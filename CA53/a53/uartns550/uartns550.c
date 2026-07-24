/** 
 * @file   uartns550.c
 * @note   Nanjing Digitgate Communication Technology Co., LTD. All Rights Reserved.
 * @brief   
 *
 * @author guodecai	
 * @date   2026/05/09
 *
 * @version
 *  date        |version |author              |message
 *  :----       |:----   |:----               |:------
 *  2026/05/09  |V1.0    |guodecai            |create base code
 * @warning 
 */

#include <stdio.h>
#include <string.h>
#include <time.h>
#include "xuartns550.h"
#include "fmsh_gic.h"
#include "fmsh_print.h"
#include "uartns550.h"
#include "eeprom_api.h"
#include "semphr.h"

static XUartNs550 UartNs550Instance0;
static volatile int TotalReceivedCount0 = 0;
static volatile int TotalSentCount0 = 0;
static volatile int TotalErrorCount0 = 0;
static volatile u8 UartNs550RecFlag0 = 0;

static volatile u8 first_tod_check = 0;            /* 上一次tod是否有效 */     
static struct GPS_INFO last_valid_tod = {0};       /* 上一次有效TOD */    
static uint8_t modify_sec_en = 0;

struct GPS_INFO g_gps_info = {0};
static struct GPS_INFO g_gps_info_pps_aligned = {0};

static uint8_t g_cali_cnt = 1;
static volatile uint32_t tod_recv_cnt = 0;
static volatile uint32_t last_recv_cnt = 0;
static uint8_t tod_recv_buf[TOD_RECV_SIZE] = {0};
static SemaphoreHandle_t tod_sem = NULL;
static BaseType_t taskwoken_tod_sem = pdFALSE;

#ifdef PPS_TIME_TEST
static uint8_t pps_test_en = 0;
#endif
static uint32_t pps_cycle_buf[PPS_CYCLE_BUF_SIZE];
static uint16_t pps_cycle_idx = 0;
static uint8_t  pps_cycle_buf_full = 0;

UINT32 serial_band[] = 
{
    9600,
    19200,
    38400,
    57600,
    115200,
    230400,
    460800,
    921600
};

/* 解析结果结构体 */
struct NmeaFrame_t 
{
    uint8_t gga_buf[NMEA_MAX_LEN];   /* 单条 $GNGGA，可直接传给 parse_gps_info */
    uint8_t rmc_buf[NMEA_MAX_LEN];   /* 单条 $GNRMC，可直接传给 parse_gps_info */
    bool gga_valid;                  /* gga_buf 是否有有效数据 */
    bool rmc_valid;                  /* rmc_buf 是否有有效数据 */
};

static struct NmeaFrame_t frames = {0};

/* 星历TOD接收状态机 */
typedef enum
{
    WAIT_SYNC,              /* 等待第一个 0x7E */ 
    WAIT_VALID_FRAME,       /* 已收到0x7E，等待第4字节=0x84确认 */
    RECVING,                /* 接收数据中 */
} RecvState;

enum PRINT_LEVEL g_print_type = DFT_NONE;
enum ANT_TYPE g_ant_type = XL_TYPE;
static RecvState s_recv_state = WAIT_SYNC;
static u8 g_recv_frame_buf[MAX_FRAME_SIZE] = {0};
static volatile u16 g_recv_frame_len = 0;
extern int parse_gps_info(char *raw_data, struct GPS_INFO *gps_info);
extern int parse_xl_gps_info(char *raw_data, uint16_t len, struct GPS_INFO *gps_info);
static u32 uartns550_recv(u32 deviceID, u8 *DataBufferPtr, u32 NumBytes);

static const uint8_t *find_str(const uint8_t *buf, uint32_t buf_len, const char *str)
{
    uint32_t str_len = strlen(str);
    uint32_t i;

    if (buf_len < str_len) return NULL;

    for (i = 0; i <= buf_len - str_len; i++)
    {
        if (memcmp(buf + i, str, str_len) == 0)
        {
            return buf + i;
        }
    }

    return NULL;
}

static const uint8_t *find_end(const uint8_t *buf, uint32_t buf_len, uint32_t offset)
{
    uint32_t i;

    for (i = offset; i < buf_len - 1; i++)
    {
        if (buf[i] == '\r' && buf[i+1] == '\n')
        {
            return buf + i + 2;
        }
        if (buf[i] == '\n')
        {
            return buf + i + 1;
        }
    }

    return NULL;
}

static void extract_time(const char *nmea, char *time_buf, uint32_t buf_size)
{
    const char *p = nmea;
    uint32_t k = 0;
    
    /* 跳过开头的 $XXYYY，找到第一个逗号 */
    while (*p && *p != ',') p++;
    
    /* 跳过逗号 */
    if (*p == ',') p++;
    
    /* 复制时间字段（到下一个逗号为止） */
    while (*p && *p != ',' && k < buf_size - 1)
    {
        time_buf[k++] = *p++;
    }
    
    time_buf[k] = '\0';
}

static int extract_nmea_frames_from_buf(const uint8_t *buf, uint32_t buf_len, struct NmeaFrame_t *result)
{
    const uint8_t *start, *end;
    uint32_t len;
    const uint8_t *last_gga = NULL;
    const uint8_t *last_rmc = NULL;
    const uint8_t *p;
    uint32_t remain;

    if (result == NULL || buf == NULL)
    {
        return -1;
    }

    memset(result, 0, sizeof(struct NmeaFrame_t));

    if (buf_len == 0 || buf_len > TOD_RECV_SIZE)
    {
        return -1;
    }

    /* 找最后一个 $GNGGA */
    p = buf;
    remain = buf_len;
    while ((p = find_str(p, remain, "$GNGGA")) != NULL)
    {
        last_gga = p;
        p += 6;
        remain = buf_len - (p - buf);
    }

    /* 找最后一个 $GNRMC */
    p = buf;
    remain = buf_len;
    while ((p = find_str(p, remain, "$GNRMC")) != NULL)
    {
        last_rmc = p;
        p += 6;
        remain = buf_len - (p - buf);
    }

    /* 提取最后一个 GGA */
    if (last_gga != NULL)
    {
        end = find_end(buf, buf_len, last_gga - buf);
        if (end != NULL)
        {
            len = end - last_gga;
            if (len >= NMEA_MAX_LEN) len = NMEA_MAX_LEN - 1;
            memcpy(result->gga_buf, last_gga, len);
            result->gga_buf[len] = '\0';
            result->gga_valid = true;
        }
    }

    /* 提取最后一个 RMC */
    if (last_rmc != NULL)
    {
        end = find_end(buf, buf_len, last_rmc - buf);
        if (end != NULL)
        {
            len = end - last_rmc;
            if (len >= NMEA_MAX_LEN) len = NMEA_MAX_LEN - 1;
            memcpy(result->rmc_buf, last_rmc, len);
            result->rmc_buf[len] = '\0';
            result->rmc_valid = true;
        }
    }

    /* 检查时间戳是否匹配 */
    if (result->gga_valid && result->rmc_valid)
    {
        char gga_time[16] = {0};
        char rmc_time[16] = {0};
        
        extract_time(result->gga_buf, gga_time, sizeof(gga_time));
        extract_time(result->rmc_buf, rmc_time, sizeof(rmc_time));
        
        if (strcmp(gga_time, rmc_time) != 0)
        {
            /* 时间不匹配，GGA 可能是上一帧的 */
            fmsh_print("Time mismatch: GGA=%s, RMC=%s\r\n", gga_time, rmc_time);
            /* 只保留 RMC */
            result->gga_valid = false;
            memset(result->gga_buf, 0, sizeof(result->gga_buf));
        }
    }

    return (result->gga_valid || result->rmc_valid) ? 0 : -1;
}

static void gps_time_add_1_second(struct GPS_INFO *tm)
{
    struct tm gps_tm = {0};

    if (NULL == tm)
    {
        if ((ERRORS == g_print_type) ||  (ALL == g_print_type))
        {
            fmsh_print("%s:nullptr\r\n", __func__);
        }
        
        return;
    } 

    /* 秒 +1 */
    tm->sec += 1;
    tm->total_sec += 1;

    /* 秒进位到分 */
    if (tm->sec >= 60) 
    {
        tm->sec = 0;
        tm->min += 1;

        /* 分进位到时 */
        if (tm->min >= 60) 
        {
            tm->min = 0;
            tm->hour += 1;

            /* 时进位到天 */
            if (tm->hour >= 24) 
            {
                tm->hour = 0;

                gps_tm.tm_year = tm->year - 1900;
                gps_tm.tm_mon  = tm->mon - 1;
                gps_tm.tm_mday = tm->day + 1;
                gps_tm.tm_hour = 0;
                gps_tm.tm_min  = 0;
                gps_tm.tm_sec  = 0;

                mktime(&gps_tm);

                tm->year = gps_tm.tm_year + 1900;
                tm->mon  = gps_tm.tm_mon + 1;
                tm->day  = gps_tm.tm_mday;
            }
        }
    }
}

static u8 check_leap_second(struct GPS_INFO *current)
{
    if (NULL == current)
    {
        first_tod_check = 0;
        if ((ERRORS == g_print_type) || (ALL == g_print_type))
        {
            fmsh_print("%s-nullptr\r\n", __func__);
        }

        return FMSH_FAILURE;
    }

    /* First valid TOD , no check */
    if (0 == first_tod_check) 
    {
        first_tod_check = 1;
        memcpy(&last_valid_tod, current, sizeof(struct GPS_INFO));
        return FMSH_SUCCESS;
    }

    /* start check */
    if (first_tod_check) 
    {
        if (current->total_sec != last_valid_tod.total_sec + 1) 
        {
            /* Reset baseline to avoid continuous errors */
            current->status = INVALID;
            if ((ERRORS == g_print_type) || (ALL == g_print_type))
            {
                fmsh_print("check_leap_second failed!\r\n");
                return FMSH_FAILURE;
            }
        }
    }

    //fmsh_print("t-:%lu\r\n", current->total_sec);
    memcpy(&g_gps_info_pps_aligned, current, sizeof(struct GPS_INFO));
    gps_time_add_1_second(&g_gps_info_pps_aligned);
    //fmsh_print("t+:%lu\r\n", g_gps_info_pps_aligned.total_sec);

    /* Fallback: update baseline */
    memcpy(&last_valid_tod, current, sizeof(struct GPS_INFO));

    return FMSH_SUCCESS;
}

static void pps_intr(void)
{
#ifdef PPS_TIME_TEST
    if (1 == pps_test_en)
    {
        uint32_t cur_cycle = FMSH_ReadReg(0x0, 0x80005594);
        /* 存入 Buffer（循环覆盖） */
        if (pps_cycle_idx < PPS_CYCLE_BUF_SIZE) 
        {
            pps_cycle_buf[pps_cycle_idx++] = cur_cycle;
        } 
        else
        {
            pps_cycle_buf_full = 1;
            pps_cycle_idx = 0;
            pps_cycle_buf[pps_cycle_idx++] = cur_cycle;
        }
    }
#endif
    /* 优先更新FPGA寄存器 */
    if (VALID == g_gps_info.status)
    {
        /* 第一次校准 */
        if (0 == modify_sec_en)
        {
            modify_sec_en = 1;
            write_fpga_sec_reg();
        }
        else
        {
            /* 达到校准次数：60S */
            if (CALI_FPGA_SECOND <= g_cali_cnt)
            {
                g_cali_cnt = 0;
                write_fpga_sec_reg();
#ifdef PPS_TIME_TEST
                pps_test_en = 1;
#endif
            }
        }

        g_cali_cnt++;
    }

    xSemaphoreGiveFromISR(tod_sem, &taskwoken_tod_sem);
}

#ifdef PPS_TIME_TEST
void analyze_pps_cycles(void)
{
    uint32_t i = 0;
    uint32_t count = 0;
    uint32_t max_cycle = 0;
    uint32_t min_cycle = 0xFFFFFFFF;

    count = pps_cycle_buf_full ? PPS_CYCLE_BUF_SIZE : pps_cycle_idx;

    if (count == 0) 
    {
        fmsh_print("No PPS cycle data collected yet.\r\n");
        return;
    }

    fmsh_print("=== PPS Cycle Analysis (Total: %lu) ===\r\n", count);

    for (i = 0; i < count; i++) 
    {
        uint32_t cycle = pps_cycle_buf[i];

        if (cycle > max_cycle) max_cycle = cycle;
        if (cycle < min_cycle) min_cycle = cycle;
    }

    /* 打印统计结果 */
    fmsh_print("  Max cycle:%lu, us:%lu\r\n", max_cycle, (unsigned long long)((uint64_t)max_cycle * 10ULL / 1000U));
    fmsh_print("  Min cycle:%lu, us:%lu\r\n", min_cycle,(unsigned long long)((uint64_t)min_cycle * 10ULL / 1000U));
    fmsh_print("Delta cycle:%lu, us:%lu\r\n", max_cycle - min_cycle, 
                    (unsigned long long)((uint64_t)(max_cycle - min_cycle) * 10ULL / 1000U));
}
#endif

u8 pps_intr_init(u32 intId)
{
    u8 Status = GIC_FAILURE;

    Status = FGicPs_Connect(&IntcInstance, intId, (FMSH_InterruptHandler)pps_intr, 0);
    if(Status != GIC_SUCCESS) 
    {
        return GIC_FAILURE;
    }

    /* 设置触发方式：上升沿触发 */
    FGicPs_SetPriorityTriggerType(&IntcInstance, intId, PPS_INT_PRIOIRTY, 0x3);
    FGicPs_InterruptMaptoCpu(&IntcInstance, GICMAP_CPUID0, intId);	
    FGicPs_Enable(&IntcInstance, intId);
    return GIC_SUCCESS;
}

static void print_gps_info(struct GPS_INFO gps_info)
{
    fmsh_print("GPS_INFO:\r\n");
    fmsh_print("  total_sec: %llu\r\n", (u64)gps_info.total_sec);
    fmsh_print("  utc-time: %04u-%02u-%02u %02u:%02u:%02u.%03u\r\n",
              gps_info.year, gps_info.mon, gps_info.day,
              gps_info.hour, gps_info.min, gps_info.sec, gps_info.ms);
    fmsh_print("  status:    %s\r\n", (VALID == gps_info.status) ? "Valid (A)" : "Invalid (V)");
    fmsh_print("  latitude:  %.7f \r\n", gps_info.latitude);
    fmsh_print("  longitude: %.7f \r\n", gps_info.longitude);
    fmsh_print("  altitude:  %.3f m\r\n", gps_info.altitute);
    
    if (XL_TYPE == g_ant_type)
    {
        fmsh_print("  E_speed:  %.2f m/s\r\n", gps_info.E_speed);
        fmsh_print("  N_speed:  %.2f m/s\r\n", gps_info.N_speed);
        fmsh_print("  D_speed:  %.2f m/s\r\n", gps_info.D_speed);
    }
}

static void UartNs550IntrHandler(void *CallBackRef, u32 Event, unsigned int EventData)
{
	u8 Errors;
	XUartNs550 *UartNs550Ptr = (XUartNs550 *)CallBackRef;

    if (XPAR_AXI_UART16550_0_BASEADDR == UartNs550Ptr->BaseAddress)
    {
        /*
        * All of the data has been sent.
        */
        if (Event == XUN_EVENT_SENT_DATA) 
        {
            TotalSentCount0 = EventData;
        }

        /*
        * All of the data has been received.
        */
        if (Event == XUN_EVENT_RECV_DATA) 
        {
            TotalReceivedCount0 += EventData;
            UartNs550RecFlag0 = 1;
            
            if (TOD_RECV_SIZE <= tod_recv_cnt)
            {
                tod_recv_cnt = 0;
                memset(tod_recv_buf, 0x0, TOD_RECV_SIZE);
            }
            
            uartns550_recv(UARTNS550_DEVICE_0_ID, tod_recv_buf + tod_recv_cnt, 1);
            tod_recv_cnt++;
        }
#if 0
        /*
        * Data was received, but not the expected number of bytes, a
        * timeout just indicates the data stopped for 4 character times.
        */
        if (Event == XUN_EVENT_RECV_TIMEOUT) 
        {
            TotalReceivedCount0 += EventData;
            UartNs550RecFlag0 = 1;
        }
#endif
        /*
        * Data was received with an error, keep the data but determine
        * what kind of errors occurred.
        */
        if (Event == XUN_EVENT_RECV_ERROR) 
        {
            TotalReceivedCount0 = EventData;
            TotalErrorCount0++;
            Errors = XUartNs550_GetLastErrors(UartNs550Ptr);
        }
    }
    else
    {
        /* noting to do */
    }
}

static u32 uartns550_send(u32 deviceID, u8 *DataBufferPtr, u32 NumBytes)
{
    XUartNs550 *InstancePtr;
    
    if (deviceID > MAX_UARTNS550_DEVICE_ID)
    {
        fmsh_print("invalid device id, support max device id is:%d\r\n", MAX_UARTNS550_DEVICE_ID);
        return FMSH_FAILURE;
    }

    if (UARTNS550_DEVICE_0_ID == deviceID)
    {
        InstancePtr = &UartNs550Instance0;
    }    
    
    return XUartNs550_Send(InstancePtr, DataBufferPtr, NumBytes);
}

static u32 uartns550_recv(u32 deviceID, u8 *DataBufferPtr, u32 NumBytes)
{
    XUartNs550 *InstancePtr;

    if (deviceID > MAX_UARTNS550_DEVICE_ID)
    {
        fmsh_print("invalid device id, support max device id is:%d\r\n", MAX_UARTNS550_DEVICE_ID);
        return FMSH_FAILURE;
    }

    if(UARTNS550_DEVICE_0_ID == deviceID)
    {
        InstancePtr = &UartNs550Instance0;
    }
    
    return XUartNs550_Recv(InstancePtr, DataBufferPtr, NumBytes);
}

static u8 uartns550_init(u32 deviceID, u32 BaudRate) 
{
    u32 intId = 0;
    u16 Options = 0;
    u32 intPriority = 0;
    XUartNs550 *UartInstancePtr;
    u8 Status = FMSH_FAILURE;
    
    if (deviceID > MAX_UARTNS550_DEVICE_ID)
    {
        fmsh_print("invalid device id, support max device id is:%d\r\n", MAX_UARTNS550_DEVICE_ID);
        return FMSH_FAILURE;
    }
    
    if (UARTNS550_DEVICE_0_ID == deviceID)
    {
        UartInstancePtr = &UartNs550Instance0;
        intId = UARTNS550_DEVICE_0_INT;
        intPriority = UARTNS550_DEVICE_0_INT_PRIOIRTY;
    }
        
	/*
	 * Initialize the UART driver so that it's ready to use.
	 */
	Status = XUartNs550_Initialize(UartInstancePtr, deviceID, BaudRate);
	if (Status != FMSH_SUCCESS) 
    {
        fmsh_print("XUartNs550_Initialize failed\r\n");
		return FMSH_FAILURE;
	}

    /*
	 * Perform a self-test to ensure that the hardware was built correctly.
	*/
	Status = XUartNs550_SelfTest(UartInstancePtr);
	if (Status != FMSH_SUCCESS) 
    {
		return FMSH_FAILURE;
	}

    /*
	 * Connect a device driver handler that will be called when an interrupt
	 * for the device occurs, the device driver handler performs the
	 * specific interrupt processing for the device.
	 */
 
    Status = FGicPs_Connect(&IntcInstance, intId, (FMSH_InterruptHandler)XUartNs550_InterruptHandler, UartInstancePtr);
    if(Status != GIC_SUCCESS) 
    {
        return GIC_FAILURE;
    }

    /* 设置触发方式：上升沿触发 */
    FGicPs_SetPriorityTriggerType(&IntcInstance, intId, intPriority, 0x3);   
    FGicPs_InterruptMaptoCpu(&IntcInstance, GICMAP_CPUID0, intId);
    FGicPs_Enable(&IntcInstance, intId);
    
    /*
	 * Setup the handlers for the UART that will be called from the
	 * interrupt context when data has been sent and received, specify a
	 * pointer to the UART driver instance as the callback reference so
	 * the handlers are able to access the instance data.
	 */
	XUartNs550_SetHandler(UartInstancePtr, UartNs550IntrHandler,
			  UartInstancePtr);
    
    /*
	 * Enable the interrupt of the UART so interrupts will occur, and keep the
	 * FIFOs enabled.
	 */
	Options = XUN_OPTION_DATA_INTR | XUN_OPTION_FIFOS_ENABLE;
	XUartNs550_SetOptions(UartInstancePtr, Options);

    /*
     * clear the fpga rx fifo.
	*/
    XUartNs550_WriteReg(0x0, 0xa2011008, 0xc7);
    delay_ms(10);

    /*
     * init tod sem.
	*/
    tod_sem = xSemaphoreCreateBinary();

    return FMSH_SUCCESS;
}

u8 tod_uart_init(u32 deviceID)
{
    u32 tod_baud = 9600;
    u8 tmp_board = 0;
    u8 tmp_type = XL_TYPE;
    u8 Status = FMSH_FAILURE;

    Status = eeprom_get_ant_type(&tmp_type);
    if (FMSH_SUCCESS != Status)
    {
        fmsh_print("eeprom_get_ant_type filed\r\n");
        return FMSH_FAILURE;
    }
    else
    {
        if (tmp_board != 0xff)
        {
            g_ant_type = tmp_type; 
        }
        else
        {
            g_ant_type = XL_TYPE;
        }

        fmsh_print("ant_type: %s\r\n", (XL_TYPE == g_ant_type) ?  "XL_TYPE" : "STD_TYPE");
    }

    Status = eeprom_get_board_type(&tmp_board);
    if (FMSH_SUCCESS != Status)
    {
        fmsh_print("eeprom_get_board_type filed\r\n");
        return FMSH_FAILURE;
    }
    else
    {
        fmsh_print("eeprom_get_board_type:%x\r\n", tmp_board);
        if (tmp_board != 0xff)
        {
            tod_baud = serial_band[tmp_board % 8];
            fmsh_print("board_id:0x%x, tod_baud:%d\r\n", tmp_board, tod_baud);
        }
    }

    Status = uartns550_init(deviceID, tod_baud);
    if (FMSH_SUCCESS != Status)
    {
        fmsh_print("tod_uart_init failed\r\n");
        return FMSH_FAILURE;
    }
    else
    {
        fmsh_print("tod_uart_init success\r\n");
        return FMSH_SUCCESS;
    }
}

static int write_fpga_sec_reg(void)
{
    /* set ctl bit low level */
    FMSH_WriteReg(0x0, W_SECOND_IN_REG, 0x0);

    /* clear second-in reg */
    FMSH_WriteReg(0x0, W_SECOND_IN_REG, 0x80000000);

    /* set second count to fpga */
    FMSH_WriteReg(0x0, W_SECOND_REG, g_gps_info_pps_aligned.total_sec);
    //fmsh_print("p:%lu\r\n", g_gps_info_pps_aligned.total_sec);

    /* set ctl bit high level */
    FMSH_WriteReg(0x0, W_SECOND_IN_REG, 0x40000000);
}

static void xl_type_handler(const u8 *local_buf, uint32_t local_cnt)
{
    uint32_t j;
    u8 i;
    u8 Status = FMSH_FAILURE;
    u8 recv_byte = 0;

    for (j = 0; j < local_cnt; j++)
    {
        recv_byte = local_buf[j];

        switch (s_recv_state)
        {
            case WAIT_SYNC:
                if (recv_byte == FRAME_SYNC) 
                {
                    memset(g_recv_frame_buf, 0, MAX_FRAME_SIZE);
                    g_recv_frame_len = 0;
                    g_recv_frame_buf[g_recv_frame_len++] = recv_byte;
                    s_recv_state = WAIT_VALID_FRAME;
                }
                break;

            case WAIT_VALID_FRAME:
                if (g_recv_frame_len >= MAX_FRAME_SIZE) 
                {
                    s_recv_state = WAIT_SYNC;
                    break;
                }

                g_recv_frame_buf[g_recv_frame_len++] = recv_byte;

                /* 收到第4字节,判断是否 0x84 */
                if (g_recv_frame_len == 4) 
                {
                    if (g_recv_frame_buf[3] == VALID_CMD) 
                    {
                        /* 有效帧，继续接收 */
                        s_recv_state = RECVING;
                    } 
                    else
                    {
                        /* 非有效帧，丢弃 */
                        s_recv_state = WAIT_SYNC;
                    }
                }
                break;

            case RECVING:
                if (g_recv_frame_len >= MAX_FRAME_SIZE) 
                {
                    s_recv_state = WAIT_SYNC;
                    break;
                }

                g_recv_frame_buf[g_recv_frame_len++] = recv_byte;
                if (recv_byte == FRAME_SYNC)
                {
                    Status = FMSH_SUCCESS;
                    
                    if ((DEBUG == g_print_type) || (ALL == g_print_type))
                    {
                        fmsh_print("recv_raw_data:\r\n");
                        for (i = 0; i < XL_TMP_SIZE; i++)
                        {
                            fmsh_print("%02x ", g_recv_frame_buf[i]);
                            if ((i + 1) % 8 == 0)
                            {
                                fmsh_print("\r\n");
                            }
                        }
                        fmsh_print("\r\n");
                    }
                                  
                    parse_xl_gps_info(g_recv_frame_buf, g_recv_frame_len, &g_gps_info);
                    check_leap_second(&g_gps_info);
                    
                    if (ALL == g_print_type)
                    {
                        print_gps_info(g_gps_info);
                    }
                    
                    s_recv_state = WAIT_SYNC;
                }
                break;

            default:
                s_recv_state = WAIT_SYNC;
                break;
        }
    }
}

#ifdef UARTNS550_EN
void uartns550_intr_handle(void *pvParameters)
{
    int ret = -1;
    uint32_t local_cnt = 0;
    uint8_t local_buf[TOD_RECV_SIZE] = {0};

    while (1)
    {
        xSemaphoreTake(tod_sem, portMAX_DELAY);

        /* 轮询等待数据稳定：50ms * 12 = 600ms */
        int i;
        int stable_cnt = 0;
        for (i = 1; i <= SLEEP_TIMES; i++)
        {
            if ((DEBUG == g_print_type) ||  (ALL == g_print_type))
            {
                fmsh_print("cur poll times:%d, cur recv cnt:%d\r\n", i, tod_recv_cnt);
            }
            
            if ((last_recv_cnt == tod_recv_cnt) && (0 != tod_recv_cnt))
            {
                stable_cnt++;
                /* 连续2次（100ms）数据不变且已收到数据，认为收完 */
                if (stable_cnt >= STABLE_CNT && tod_recv_cnt > 0)
                {
                    if ((DEBUG == g_print_type) ||  (ALL == g_print_type))
                    {
                        fmsh_print("final recv cnt:%d\r\n", tod_recv_cnt);
                    }
                    
                    break;
                }
            }
            else
            {
                stable_cnt = 0;
            }
            
            last_recv_cnt = tod_recv_cnt;
            vTaskDelay(pdMS_TO_TICKS(SLEEP_TIME_MS));
        }

        /* 600ms 内没收到任何数据，超时 */
        if (tod_recv_cnt == 0)
        {
            if ((ERRORS == g_print_type) ||  (ALL == g_print_type))
            {
                fmsh_print("[ERROR]tod timeout!\r\n");
            }
            g_gps_info.status = INVALID;

            /* 防御性清空 */
            memset(tod_recv_buf, 0x0, TOD_RECV_SIZE);
            tod_recv_cnt = 0;
            last_recv_cnt = 0;
            continue;
        }

        /* 拷贝到本地缓冲区并清空全局缓冲区 */
        local_cnt = tod_recv_cnt;
        if (local_cnt > TOD_RECV_SIZE) local_cnt = TOD_RECV_SIZE;
        memcpy(local_buf, tod_recv_buf, local_cnt);
        memset(tod_recv_buf, 0x0, TOD_RECV_SIZE);
        tod_recv_cnt = 0;
        last_recv_cnt = 0;

        if (XL_TYPE == g_ant_type)
        {
            /* XL_TYPE */
            s_recv_state = WAIT_SYNC;
            g_recv_frame_len = 0;
            memset(tod_recv_buf, 0x0, TOD_RECV_SIZE);
            xl_type_handler(local_buf, local_cnt);
        }
        else /* TOD盒子：STD_TYPE */
        {
            /* 用本地缓冲区提取 NMEA 帧 */
            extract_nmea_frames_from_buf(local_buf, local_cnt, &frames);

            /* 解析 $GNRMC */
            if (frames.rmc_valid)
            {
                ret = parse_gps_info((char*)frames.rmc_buf, &g_gps_info);
                if (ret == 0)
                {
                    /* RMC 解析成功，再解析 GGA 补充高度 */
                    if (frames.gga_valid)
                    {
                        ret = parse_gps_info((char*)frames.gga_buf, &g_gps_info);
                        if (ret != 0)
                        {
                            if ((ERRORS == g_print_type) ||  (ALL == g_print_type))
                            {
                                fmsh_print("Parse GNGGA failed!\r\n");
                            }
                            
                            g_gps_info.status = INVALID;
                        }
                    }
                    else
                    {
                        if ((ERRORS == g_print_type) ||  (ALL == g_print_type))
                        {
                            fmsh_print("not found $GNGGA\r\n");
                        }
                    }

                    /* 跳秒检测 */
                    if (g_gps_info.status == VALID)
                    {
                        check_leap_second(&g_gps_info);
                    }
                }
                else
                {
                    if ((ERRORS == g_print_type) ||  (ALL == g_print_type))
                    {
                        fmsh_print("Parse GNRMC failed!\r\n");
                    }
                    
                    g_gps_info.status = INVALID;
                }
            }
            else
            {
                if ((ERRORS == g_print_type) ||  (ALL == g_print_type))
                {
                    fmsh_print("not found $GNRMC\r\n");
                }
                
                g_gps_info.status = INVALID;
            }
    }
    }
}
#endif

#ifdef PRINTF_TOD_TEST
void printf_tod_handler(void *pvParameters)
{
    uint32_t cur_fpga_sec_in_cycle = 0;
    uint16_t cur_fpga_sec_in_ms = 0;
    
    while (1)
    {
        cur_fpga_sec_in_cycle = FMSH_ReadReg(0x0, 0x80005594);
        cur_fpga_sec_in_ms = (cur_fpga_sec_in_cycle * 10ULL) / 1000000;
        g_gps_info_pps_aligned.ms = (uint16_t)cur_fpga_sec_in_ms;
        
        print_gps_info(g_gps_info_pps_aligned);
        vTaskDelay(pdMS_TO_TICKS(900));
    }
}
#endif

#ifdef API_TEST
void api_test_handler(void *pvParameters)
{
    struct GPS_INFO api_test = {0};

    while (1)
    {
        a53_get_gps_info(&api_test);
        print_gps_info(api_test);
        vTaskDelay(pdMS_TO_TICKS(900));
    }
}
#endif
