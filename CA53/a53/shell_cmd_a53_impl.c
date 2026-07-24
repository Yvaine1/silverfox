/* =============================================================
 * 文件作用：A53 shell 命令函数实现
 *
 * 对应声明：CA53/a53/shell_cmd_a53_impl.h
 * 注册方：FM_ZQ_bsp/thirdpartlib/LetterShell/src/shell_export_cmd_a53.c
 * ============================================================= */

#if (SHELL_CMD_MASTER == 0)

#include "shell_cmd_a53_impl.h"

/* ============ A53 .h add here == */
#include "shell.h"
#include "fmsh_common.h"
#include "string.h"
#include "fmsh_sdmmc_example.h"
#include "eeprom_api.h"
#include "FreeRTOS.h"
#include "task.h"
#include "stdio.h"
#include "mem_common.h"
#include "ddr_capture.h"
#include "fmsh_uart_at.h"
#include "lwip/udp.h"
#include "lwip/ip_addr.h"
#include "lwip/err.h"
#include "uartns550.h"
#include "fmsh_rtc_mix.h"
#include "load_img.h"
#include "powermeter.h"
#include "mem2bin.h"
#include "nr_shm_oam.h"
#include "fmsh_mailbox_ipips.h"

extern IpiPsu Ipi_a53_cr5_0;
extern IpiPsu Ipi_a53_cr5_1;

/* ============ Command definitions ============ */
#define FPGA_BASEADDR 0x80000000
#define FPGA_ADDR_MASK 0x003FFFFF

extern void stack_shell_command_description(void);
extern void stack_handle_shell_command(const char* c_name, const char* c_param);
extern enum ANT_TYPE ant_type;

void readreg(u64 addr)
{
    fmsh_print("reg cnt: %#x \r\n", FMSH_ReadReg(0x0, addr));
}

void writereg(u64 addr, u64 Data)
{
    FMSH_WriteReg(0x0, addr, Data);
    fmsh_print("reg cnt: %#x \r\n", FMSH_ReadReg(0x0, addr));
}

void fpga_writeRegs(u32 offSet, u32 Data)
{
    if (offSet > FPGA_ADDR_MASK)
    {
        fmsh_print("Error: offset 0x%x out of range (0x00000000~0x003FFFFF)\r\n", offSet);
        return;
    }
    if (offSet & 0x3)
    {
        fmsh_print("Error: offset 0x%x not 4-byte aligned\r\n", offSet);
        return;
    }
    FMSH_WriteReg((u32)FPGA_BASEADDR, offSet, Data);
    fmsh_print("reg cnt: %#x \r\n", FMSH_ReadReg((u32)FPGA_BASEADDR, offSet));
}

void fpga_readRegs(u32 offSet)
{
    if (offSet > FPGA_ADDR_MASK)
    {
        fmsh_print("Error: offset 0x%x out of range (0x00000000~0x003FFFFF)\r\n", offSet);
        return;
    }
    if (offSet & 0x3)
    {
        fmsh_print("Error: offset 0x%x not 4-byte aligned\r\n", offSet);
        return;
    }
    fmsh_print("reg cnt: %#x \r\n", FMSH_ReadReg((u32)FPGA_BASEADDR, offSet));
}

void cmd_showversion()
{
    u8 device_id = 0;
    image_header_t header;
    if(emmc_read_image_header_version(device_id, &header) == FMSH_SUCCESS)
    {
        print_image_header_info(&header);
        for(LOAD_IMAGE_LIST img_list = 0; img_list < LOAD_IMAGE_ALL - 1; img_list++)
        {
            emmc_read_module_version(device_id, img_list);
        }
    }
    else
    {
        fmsh_print("Image header read fail!\r\n");
    }
    return;
}

void cmd_update_img()
{
    const char *image_file = "0:image";
    u8 device_id = 0;
    if(emmc_update_image(image_file, device_id) == FMSH_SUCCESS)
        fmsh_print("Update image Success!\r\n");
    else
        fmsh_print("Update image fail!\r\n");
}

void cmd_update_module(char *filename, u8 load_id)
{
    u8 device_id = 0;
    emmc_update_module(filename, device_id, load_id);
}

void cmd_ls(char *path)
{
    fmsh_print("Enter %s \r\n", __FUNCTION__);
    show_all_dir_of_partition(path);
    show_all_file_info_of_dir(path);
}

void cmd_mem2bin(u8 granularity, u8 direction, u32 number, u8 frame_count)
{
    fmsh_print("Enter %s \r\n", __FUNCTION__);
    mem2bin(granularity, direction, number, frame_count);
}

extern FRtcPs_T g_RTC;
void cmd_set_rtc_clock(u16 year, u8 mon, u8 day, u8 hour, u8 min, u8 sec)
{
    int ret = FMSH_SUCCESS;
    rtc_time ctm;
    FRtcPs_portmap_T *portmap;
    portmap = (FRtcPs_portmap_T *)g_RTC.base_address;

    if(year < 1980 || year > 2107)
    {
        fmsh_print("Year param error! (1980~2107)\r\n");
        return;
    }
    if(mon < 1 || mon > 12)
    {
        fmsh_print("Month param error! (1~12)\r\n");
        return;
    }
    if(day < 1 || day > 31)
    {
        fmsh_print("Day param error! (1~31)\r\n");
        return;
    }
    if(hour > 23 || min > 59 || sec > 59)
    {
        fmsh_print("Time param error! (hour:0~23,min/sec:0~59)\r\n");
        return;
    }

    ctm.tm_year = year - 1900;
    ctm.tm_mon  = mon - 1;
    ctm.tm_mday = day;
    ctm.tm_hour = hour;
    ctm.tm_min  = min;
    ctm.tm_sec  = sec;

    ret = FRtcPs_set_time(&g_RTC, &ctm);
    if (ret != FMSH_SUCCESS)
    {
        fmsh_print("FRtcPs_set_time failed\r\n");
        return;
    }

    FRtcPs_seconds_irq_enable(&g_RTC, 1);

    u32 s_val = 32768;
    u32 reg;
    FMSH_BIT_SET_NOREAD(reg, CALIB_RW_Max_Tick, s_val);
    RTC_OUT32P(reg, portmap->CALIB_WRITE);

    return;
}

void cmd_get_rtc_clock(void)
{
    rtc_time ctm;
    FRtcPs_read_time(&g_RTC, &ctm);
    fmsh_print("Time is %ld/%ld/%ld %2d:%2d:%2d \r\n", ctm.tm_year+1900, ctm.tm_mon+1, ctm.tm_mday, ctm.tm_hour, ctm.tm_min, ctm.tm_sec);
    return;
}

#ifdef UDP_TOD_TEST
#define UDP_TIME_DST_IP1    10
#define UDP_TIME_DST_IP2    255
#define UDP_TIME_DST_IP3    0
#define UDP_TIME_DST_IP4    7
#define UDP_TIME_DST_PORT   8080

/* 2006-01-01 00:00:00 UTC = 1136073600 (Unix epoch seconds).
 * Hardcoded to avoid newlib mktime() timezone dependence. */
#define UNIX_EPOCH_2006_UTC (1136073600)

/* Send one FPGA-TOD UDP packet to UDP_TIME_DST.
 * On-wire format matches udpServer.cpp::getTVFromDateTime (atol@0, atol@11).
 */
static void udp_send_time_one(void)
{
    uint64_t    cur_fpga_sec;
    uint32_t    cur_fpga_cycle;
    uint32_t    cur_us;
    uint64_t    unix_sec;
    char        msg[32];
    int         len;
    ip_addr_t   dst;
    struct udp_pcb *upcb;
    struct pbuf    *p;
    err_t       err;


    cur_fpga_sec   = FMSH_ReadReg(0x0, 0x80005598);  /* sec since 2006-01-01 UTC */
    cur_fpga_cycle = FMSH_ReadReg(0x0, 0x80005594);  /* sub-sec cycle */

    /* Match gps.c::a53_get_gps_info (cycle * 10 / 1000000 -> ms), then ms -> us */
    cur_us = ((uint64_t)cur_fpga_cycle * 10ULL) / 1000U;

    unix_sec = cur_fpga_sec + (uint64_t)UNIX_EPOCH_2006_UTC;

    len = snprintf(msg, sizeof(msg), "%010u.%06u",
                   (unsigned int)unix_sec, cur_us);

    IP4_ADDR(&dst, UDP_TIME_DST_IP1, UDP_TIME_DST_IP2,
                    UDP_TIME_DST_IP3, UDP_TIME_DST_IP4);

    upcb = udp_new();
    if (upcb == NULL) {
        fmsh_print("udp_send_time: udp_new failed\r\n");
        return;
    }

    p = pbuf_alloc(PBUF_TRANSPORT, len, PBUF_POOL);
    if (p == NULL) {
        fmsh_print("udp_send_time: pbuf_alloc failed\r\n");
        udp_remove(upcb);
        return;
    }

    memcpy(p->payload, msg, len);

    err = udp_sendto(upcb, p, &dst, UDP_TIME_DST_PORT);
    if (err != ERR_OK) {
        fmsh_print("udp_send_time: udp_sendto err=%d\r\n", err);
    } else {
        fmsh_print("udp_send_time: sent %u.%06u to %d.%d.%d.%d:%d\r\n",
                   (unsigned int)unix_sec, cur_us,
                   UDP_TIME_DST_IP1, UDP_TIME_DST_IP2,
                   UDP_TIME_DST_IP3, UDP_TIME_DST_IP4, UDP_TIME_DST_PORT);
    }

    pbuf_free(p);
    udp_remove(upcb);
}

/* `udp_send_time [n]` — send n packets, 5 s apart. vTaskDelay yields CPU. */
void cmd_udp_send_time(int argc, char *argv[])
{
    uint32_t n = 1;
    uint32_t i;

    if (argc >= 2) {
        n = (uint32_t)atoi(argv[1]);
        if (n == 0) n = 1;
    }

    fmsh_print("udp_send_time: sending %u packets, 5s apart\r\n", n);

    for (i = 0; i < n; i++) {
        udp_send_time_one();
        if (i + 1 < n) {
            vTaskDelay(pdMS_TO_TICKS(5000));
        }
    }

    fmsh_print("udp_send_time: done\r\n");
}
#endif

void cmd_getbandinfo(void)
{
    u8 bandid=0xff;
    eeprom_get_bandinfo(&bandid);
    fmsh_print("bandid = 0x%x\r\n", bandid);
}

void cmd_setbandinfo(u8 bandid)
{
    if(bandid > BAND_LINUX_SAMPLE_MAX)
    {
        fmsh_print("Error! bandid = 0x%x, value out of range\r\n", bandid);
        return;
    }
    fmsh_print("bandid = 0x%x\r\n", bandid);
    eeprom_set_bandinfo(bandid);
}

void cmd_stack_main(int argc, char *argv[])
{
    fmsh_print("Enter %s \r\n", __FUNCTION__);
    if (argc < 3)
    {
        stack_shell_command_description();
        return;
    }
    char *cmd_name = argv[1];
    char *cmd_param = argv[2];
    stack_handle_shell_command(cmd_name, cmd_param);
}

static void cmd_get_tod_baudrate_usage(void)
{
    fmsh_print("baudrate  -------   value  \r\n");
    fmsh_print("9600      -------   0x0  \r\n");
    fmsh_print("19200     -------   0x1  \r\n");
    fmsh_print("38400     -------   0x2  \r\n");
    fmsh_print("57600     -------   0x3  \r\n");
    fmsh_print("115200    -------   0x4  \r\n");
    fmsh_print("230400    -------   0x5  \r\n");
    fmsh_print("460800    -------   0x6  \r\n");
    fmsh_print("921600    -------   0x7  \r\n");
}

void cmd_get_tod_baudrate(void)
{
    u8 baudrate_id = 0;
    u8 Status = FMSH_FAILURE;

    cmd_get_tod_baudrate_usage();

    Status = eeprom_get_board_type(&baudrate_id);
    if (FMSH_SUCCESS != Status)
    {
        fmsh_print("eeprom_get_board_type filed\r\n");
        return;
    }

    if (baudrate_id == 0xff)
    {
        fmsh_print("baudrate_id:0xff, baudrate:9600\r\n");
        return;
    }

    switch (baudrate_id % 8)
    {
        case 0x0: fmsh_print("baudrate_id:0x0, baudrate:9600\r\n");   break;
        case 0x1: fmsh_print("baudrate_id:0x1, baudrate:19200\r\n");  break;
        case 0x2: fmsh_print("baudrate_id:0x2, baudrate:38400\r\n");  break;
        case 0x3: fmsh_print("baudrate_id:0x3, baudrate:57600\r\n");  break;
        case 0x4: fmsh_print("baudrate_id:0x4, baudrate:115200\r\n"); break;
        case 0x5: fmsh_print("baudrate_id:0x5, baudrate:230400\r\n"); break;
        case 0x6: fmsh_print("baudrate_id:0x6, baudrate:460800\r\n"); break;
        case 0x7: fmsh_print("baudrate_id:0x7, baudrate:921600\r\n"); break;
        default:  break;
    }
}

void cmd_set_tod_baudrate(u8 baudrate_id)
{
    u8 Status = FMSH_FAILURE;

    if (baudrate_id > 0x7)
    {
        fmsh_print("Error! baudrate_id is not support! \r\n");
        return;
    }

    switch (baudrate_id % 8)
    {
        case 0x0: fmsh_print("baudrate_id:0x0, baudrate:9600\r\n");   break;
        case 0x1: fmsh_print("baudrate_id:0x1, baudrate:19200\r\n");  break;
        case 0x2: fmsh_print("baudrate_id:0x2, baudrate:38400\r\n");  break;
        case 0x3: fmsh_print("baudrate_id:0x3, baudrate:57600\r\n");  break;
        case 0x4: fmsh_print("baudrate_id:0x4, baudrate:115200\r\n"); break;
        case 0x5: fmsh_print("baudrate_id:0x5, baudrate:230400\r\n"); break;
        case 0x6: fmsh_print("baudrate_id:0x6, baudrate:460800\r\n"); break;
        case 0x7: fmsh_print("baudrate_id:0x7, baudrate:921600\r\n"); break;
        default:  break;
    }

    Status = eeprom_set_board_type(baudrate_id);
    if (FMSH_SUCCESS != Status)
        fmsh_print("eeprom_set_board_type filed\r\n");
}

void cmd_set_ant_type(u8 ant_type)
{
    u8 Status = FMSH_FAILURE;

    if (ant_type > MAX_TYPE_NUM)
    {
        fmsh_print("Error! anttype is not support! \r\n");
        return;
    }

    if (XL_TYPE == ant_type)
        fmsh_print("anttype:0x0, prase xl tod\r\n");

    if (STD_TYPE == ant_type)
        fmsh_print("anttype:0x1, prase std tod\r\n");

    Status = eeprom_set_ant_type(ant_type);
    if (FMSH_SUCCESS != Status)
        fmsh_print("eeprom_set_ant_type filed\r\n");
}

void cmd_get_ant_type(void)
{
    u8 ant_type = MAX_TYPE_NUM;
    u8 Status = FMSH_FAILURE;

    Status = eeprom_get_ant_type(&ant_type);
    if (FMSH_SUCCESS != Status)
    {
        fmsh_print("eeprom_get_ant_type filed\r\n");
        return;
    }

    if (XL_TYPE == ant_type)
        fmsh_print("anttype:0x0, is xl tod\r\n");
    else if (STD_TYPE == ant_type)
        fmsh_print("anttype:0x1, is std tod\r\n");
    else
        fmsh_print("[error]anttype invalid!\r\n");
}

void cmd_set_tod_print_level(u8 print_type)
{
    if (print_type >= MAX_LEVEL)
    {
        fmsh_print("Error! print_type is not support! \r\n");
        return;
    }

    switch (print_type)
    {
        case DFT_NONE: g_print_type = DFT_NONE; fmsh_print("print_type: DFT_NONE!\r\n"); break;
        case ERRORS:  g_print_type = ERRORS;  fmsh_print("print_type: ERRORS!\r\n");  break;
        case DEBUG:   g_print_type = DEBUG;   fmsh_print("print_type: DEBUG!\r\n");   break;
        case INFO:    g_print_type = INFO;    fmsh_print("print_type: INFO!\r\n");    break;
        case ALL:     g_print_type = ALL;     fmsh_print("print_type: ALL!\r\n");     break;
        default:      break;
    }
}

static void cmd_get_tod_print_level_usage(void)
{
    fmsh_print("level     -------   value  \r\n");
    fmsh_print("DFT_NONE  -------   0x0  \r\n");
    fmsh_print("ERRORS    -------   0x1  \r\n");
    fmsh_print("DEBUG     -------   0x2  \r\n");
    fmsh_print("INFO      -------   0x3  \r\n");
    fmsh_print("ALL       -------   0x4  \r\n");
}

void cmd_get_tod_print_level(void)
{
    cmd_get_tod_print_level_usage();

    switch (g_print_type)
    {
        case DFT_NONE: fmsh_print("print_type: DFT_NONE!\r\n"); break;
        case ERRORS:  fmsh_print("print_type: ERRORS!\r\n");  break;
        case DEBUG:   fmsh_print("print_type: DEBUG!\r\n");   break;
        case INFO:    fmsh_print("print_type: INFO!\r\n");    break;
        case ALL:     fmsh_print("print_type: ALL!\r\n");     break;
        default:      break;
    }
}

void cmd_uart_at_print(int argc, char *argv[])
{
    BOOL enable = fmsh_uart_at_get_rx_print_enable();

    if (argc >= 2)
    {
        if ((strcmp(argv[1], "on") == 0)
            || (strcmp(argv[1], "0x1") == 0)
            || (strcmp(argv[1], "1") == 0))
            enable = TRUE;
        else if ((strcmp(argv[1], "off") == 0)
                 || (strcmp(argv[1], "0x0") == 0)
                 || (strcmp(argv[1], "0") == 0))
            enable = FALSE;
    }
    else
        enable = enable ? FALSE : TRUE;

    fmsh_uart_at_set_rx_print_enable(enable);
    fmsh_print("uart_at_print: %s \r\n", enable ? "on" : "off");
}

#ifdef PPS_TIME_TEST
void cmd_pps_delta_test(void)
{
    analyze_pps_cycles();
}
#endif

#endif /* SHELL_CMD_MASTER == 0 */
