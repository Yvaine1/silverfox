#include "at_cmd.h"
// #include "FreeRTOS.h"
// #include "task.h"
#include "fmsh_common_delay.h"

/**
 * @brief EXEC
 */
static int do_cmd_exec(int argc, char *argv[], char *ret)
{
    sprintf(ret, "OK\r\n");
    return 0;
}

/**
 * @brief QUERY
 */
static int do_cmd_query(int argc, char *argv[], char *ret)
{
    static int number = 0;
    sprintf(ret, "+QUERY:%d\r\nOK\r\n", number++);
    return 0;
}

/**
 * @brief SET
 */
static int do_cmd_set(int argc, char *argv[], char *ret)
{
    sprintf(ret, "+SET\r\nOK\r\n");
    return 0;
}

/**
 * @brief AT-CMD table
 */
static const cmd_item_t cmd_tbl[] = {
	{"EXEC",  0, do_cmd_exec,  "exec-type command"},
	{"QUERY", 1, do_cmd_query, "query-type command"},
	{"SET",   1, do_cmd_set,   "set-type command"},
};

/**
 * @brief search cmd_tbl to find cmd
 */
static const cmd_item_t *find_cmd(const char *keyword, int n)
{                   
	const cmd_item_t *it;
    const int tb_size = sizeof(cmd_tbl) / sizeof(cmd_tbl[0]);
    for (it = cmd_tbl; it < cmd_tbl + tb_size; it++) {
        if (it->name != NULL && !strncasecmp(keyword, it->name, n))
            return it;
    }
	return NULL;
}

/**
 * split AT-CMD string
 */
static size_t strsplit(char *s, const char *separator, char *list[], size_t len)
{
    size_t count = 0;      
    if (s == NULL || list == NULL || len == 0) 
        return 0;     
        
    list[count++] = s;    
    while(*s && count < len) {       
        if (strchr(separator, *s) != NULL) {
            *s = '\0';                                       
            list[count++] = s + 1;
        }
        s++;        
    }    
    return count;
}

/**
 * @brief hanld one at-cmd
 * @param
 * - target_id: means target cpu index (0: A53; 1: R51; 2: R50)
 * - ret: response of at-cmd
 */
void pro_at_cmd(int target_id, 
                at_cmd_read_t handler_a53_get_cmd, 
                at_cmd_write_t handler_a53_ret_conf)
{
    char recvbuf[CLI_MAX_CMD_LEN];
    char *argv[CLI_MAX_ARGS];
    char ret[CLI_MAX_CMD_LEN];
    int argc, type;
    const cmd_item_t *it;
    char *start;
    char *end;
    
    volatile rgbufMsg_t *p_msg = NULL;

    // get AT-CMD
    if (target_id == 0) // on A51, AT-CMD is from APP
    {
        handler_a53_get_cmd(recvbuf, CLI_MAX_CMD_LEN);
    }
    else // on R51/R50, AT-CMD is from A51
    {
        while ((p_msg = rgbufRecv(A53_TO_R51_RINGBUF0_ID)) == NULL) {
//            vTaskDelay(pdMS_TO_TICKS(10));
              delay_ms(10);
        }
        sprintf(recvbuf, "%s", p_msg->data);
        rgbufFree(A53_TO_R51_RINGBUF0_ID, p_msg->len);
    }

    // if not prefixed with AT+, it is not a valid AT-CMD
    if (strncasecmp(recvbuf, "AT+", 3))
        return;

    start= recvbuf + 3;

    if ((end = strchr(start, '=')) != NULL) 
    {
        type = AT_CMD_TYPE_SET;
    } 
    else if ((end = strchr(start, '?')) != NULL) 
    {
        type = AT_CMD_TYPE_QUERY;
    } 
    else 
    {
        type = AT_CMD_TYPE_EXEC;
        end = start + strlen(recvbuf) - 3;
    }
    if (start == end) // if string is only "AT+"
        return;

    if ((it = find_cmd(start, end - start)) == NULL) // if AT-CMD is not valid cmd
        return;

    if (target_id == 0) // on A53
    {
        if (it->target == 0) // process it locally
        {
            argc = strsplit(recvbuf, ",", argv, CLI_MAX_ARGS);
            it->handler(argc, argv, ret);
        }
        else // transfer it to R51/R50 and wait for response if necessary
        {
            if (it->target == 1)
            {
                // transfer to R51
                int message_size = (strlen(recvbuf) + 3) & 0xfffc;
                while ((p_msg = rgbufAllocate(A53_TO_R51_RINGBUF0_ID, message_size)) == NULL) 
                {
                    //vTaskDelay(pdMS_TO_TICKS(10));
                   delay_ms(10);
                }
                p_msg->len = message_size;
                p_msg->id  = 0;
                volatile char *p_data = (volatile char *)p_msg->data;
                sprintf(p_data, "%s", recvbuf);
                rgbufSend(A53_TO_R51_RINGBUF0_ID, p_msg);

                // get response from R51
                while ((p_msg = rgbufRecv(R51_TO_A53_RINGBUF1_ID)) == NULL) 
                {
                    //vTaskDelay(pdMS_TO_TICKS(10));
                    delay_ms(10);
                }
                sprintf(ret, "%s", p_msg->data);
                rgbufFree(R51_TO_A53_RINGBUF1_ID, p_msg->len);
            }
        }

        // send ret to APP
        handler_a53_ret_conf(ret, strlen(ret));

        return;
    }
    else // on R51 or R50
    {
        argc = strsplit(recvbuf, ",", argv, CLI_MAX_ARGS);
        it->handler(argc, argv, ret);
        
        // send ret to A53
        int message_size = (strlen(ret) + 3) & 0xfffc;
        while ((p_msg = rgbufAllocate(R51_TO_A53_RINGBUF1_ID, message_size)) == NULL) 
        {
            //vTaskDelay(pdMS_TO_TICKS(100));
            delay_ms(10);
        }
        p_msg->len = message_size;
        p_msg->id  = 0;
        sprintf(p_msg->data, "%s", ret);
        rgbufSend(R51_TO_A53_RINGBUF1_ID, p_msg);
    }
    
}