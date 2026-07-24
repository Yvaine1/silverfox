#ifndef AT_CMD_H
#define AT_CMD_H
#include "stdio.h"
#include "ring_buffer.h"
#include <string.h>

#define CLI_MAX_CMD_LEN           256            /*命令行长度*/                 
#define CLI_MAX_ARGS              16             /*最大参数个数*/

/*命令类型 */
#define AT_CMD_TYPE_EXEC         0              /* 普通执行命令*/
#define AT_CMD_TYPE_QUERY        1              /* 查询命令 (XXX?)*/
#define AT_CMD_TYPE_SET          2              /* 设备命令 (XXX=YY)*/

/*命令项定义*/
typedef struct {
	char	   *name;		                         /* 命令名*/
    int        target;                               /* 0:A53; 1:R51; 2:R50*/ 
    /**
     * @brief     命令处理程序,类型
     * @params    o      - cli 对象
     * @params    argc   - 命令参数个数
     * @params    argv   - 命令参数表
     * @return    命令执行结果, 对于AT指令, 返回true时会自动响应OK,返回false时则
     *            响应ERROR
     */       
	int        (*handler)(int argc, char *argv[], char *ret);   
    const char *brief;                               /*命令简介*/
    void       *reserved;
} cmd_item_t;

typedef unsigned int (*at_cmd_write_t)(const void *buf, unsigned int len);
typedef unsigned int (*at_cmd_read_t)(void *buf, unsigned int len);

void pro_at_cmd(int target_id, 
    at_cmd_read_t handler_a53_get_cmd, 
    at_cmd_write_t handler_a53_ret_conf);

#endif