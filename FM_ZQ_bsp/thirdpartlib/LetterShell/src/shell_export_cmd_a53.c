/* =============================================================
 * 文件作用：注册 A53 本地 shell 命令
 *
 * 新增一条命令的步骤：
 *   1) 在 A53 工程的头文件中声明真实函数（原型）
 *   2) 在下方"命令注册区"加一行 SHELL_EXPORT_CMD(...) 注册
 *   3) 描述字符串格式："para:p1 p2 --- 说明"
 * ============================================================= */

#if (SHELL_CMD_MASTER == 0)
/* ============ A53 .h add here == */
#include "shell.h"
#include "shell_cmd_a53_impl.h"

extern IpiPsu Ipi_a53_cr5_0;
extern IpiPsu Ipi_a53_cr5_1;

/* ============ Forwarder area ============ */
/* 共享给 SHELL_R*_FWD 在 A53 端展开时引用,A53 把命令名+参数打包后
 * 通过共享内存队列 + IPI 转到对应 R5 执行 */
#define DEFINE_SHELL_FWD(_name, _queue, _ipi_inst, _ipi_mask)                \
    void _shell_fwd_##_name(int argc, char *argv[])                          \
    {                                                                       \
        char buf[SHELL_WORK_BUFFER_SIZE];                                   \
        int len = snprintf(buf, sizeof(buf) - 1, "%s", argv[0]);            \
        for (int i = 1; i < argc; i++)                                      \
        {                                                                   \
            len += snprintf(buf + len, sizeof(buf) - 1 - len, " %s",        \
                            argv[i]);                                       \
        }                                                                   \
        if (len < (int)sizeof(buf) - 1)                                      \
        {                                                                   \
            buf[len++] = '\n';                                              \
        }                                                                   \
        L1cSendToOam(buf, len, _queue, 0);                                  \
        IpiPsu_TriggerIpi(&(_ipi_inst), _ipi_mask);                         \
    }

DEFINE_SHELL_FWD(r50, MSGQ_A53_R50_SHMA_SHELL_CMD_REQ, Ipi_a53_cr5_0, REMOTE_MASK_CH1)
DEFINE_SHELL_FWD(r51, MSGQ_A53_R51_SHMA_SHELL_CMD_REQ, Ipi_a53_cr5_1, REMOTE_MASK_CH2)


/* ============ A53 Command registrations (add new cmd here) ============ */
SHELL_EXPORT_CMD(SHELL_CMD_PERMISSION(0)|SHELL_CMD_TYPE(SHELL_TYPE_CMD_FUNC)|SHELL_CMD_DISABLE_RETURN,
                    readreg, readreg, (addr));
SHELL_EXPORT_CMD(SHELL_CMD_PERMISSION(0)|SHELL_CMD_TYPE(SHELL_TYPE_CMD_FUNC)|SHELL_CMD_DISABLE_RETURN,
                    writereg, writereg, (addr, data));
SHELL_EXPORT_CMD(SHELL_CMD_PERMISSION(0)|SHELL_CMD_TYPE(SHELL_TYPE_CMD_FUNC)|SHELL_CMD_DISABLE_RETURN,
                    fpga_readRegs, fpga_readRegs, (offSet));
SHELL_EXPORT_CMD(SHELL_CMD_PERMISSION(0)|SHELL_CMD_TYPE(SHELL_TYPE_CMD_FUNC)|SHELL_CMD_DISABLE_RETURN,
                    fpga_writeRegs, fpga_writeRegs, (offSet, data));
SHELL_EXPORT_CMD(SHELL_CMD_PERMISSION(0)|SHELL_CMD_TYPE(SHELL_TYPE_CMD_FUNC)|SHELL_CMD_DISABLE_RETURN,
                    ddr_capture, ddr_capture, (void));
SHELL_EXPORT_CMD(SHELL_CMD_PERMISSION(0)|SHELL_CMD_TYPE(SHELL_TYPE_CMD_FUNC)|SHELL_CMD_DISABLE_RETURN,
                    powermeter, calc_powermeter, (void));
SHELL_EXPORT_CMD(SHELL_CMD_PERMISSION(0)|SHELL_CMD_TYPE(SHELL_TYPE_CMD_FUNC)|SHELL_CMD_DISABLE_RETURN,
                    showversion, cmd_showversion, show version);
SHELL_EXPORT_CMD(SHELL_CMD_PERMISSION(0)|SHELL_CMD_TYPE(SHELL_TYPE_CMD_FUNC)|SHELL_CMD_DISABLE_RETURN,
                    update_img, cmd_update_img, update img);
SHELL_EXPORT_CMD(SHELL_CMD_PERMISSION(0)|SHELL_CMD_TYPE(SHELL_TYPE_CMD_FUNC)|SHELL_CMD_DISABLE_RETURN,
                    update_module, cmd_update_module, para:filename id(0(boot) 1(a53) 2(r50) 3(r51) 4(bit)));
SHELL_EXPORT_CMD(SHELL_CMD_PERMISSION(0)|SHELL_CMD_TYPE(SHELL_TYPE_CMD_FUNC)|SHELL_CMD_DISABLE_RETURN,
                    ls, cmd_ls, para: path);
SHELL_EXPORT_CMD(SHELL_CMD_PERMISSION(0)|SHELL_CMD_TYPE(SHELL_TYPE_CMD_FUNC)|SHELL_CMD_DISABLE_RETURN,
                    mem2bin, cmd_mem2bin, para:granularity(0,1) direction(0,1) number(0, 2^32-1) frame_count(1, 40));
SHELL_EXPORT_CMD(SHELL_CMD_PERMISSION(0)|SHELL_CMD_TYPE(SHELL_TYPE_CMD_FUNC)|SHELL_CMD_DISABLE_RETURN,
                    set_rtc_clock, cmd_set_rtc_clock, para:year mon day hour min sec (2026 01 19 14 30 20)--- set rtc clock);
SHELL_EXPORT_CMD(SHELL_CMD_PERMISSION(0)|SHELL_CMD_TYPE(SHELL_TYPE_CMD_FUNC)|SHELL_CMD_DISABLE_RETURN,
                    get_rtcclock, cmd_get_rtc_clock, get rtc clock);
SHELL_EXPORT_CMD(SHELL_CMD_PERMISSION(0)|SHELL_CMD_TYPE(SHELL_TYPE_CMD_FUNC)|SHELL_CMD_DISABLE_RETURN,
                    getbandinfo, cmd_getbandinfo, (void));
SHELL_EXPORT_CMD(SHELL_CMD_PERMISSION(0)|SHELL_CMD_TYPE(SHELL_TYPE_CMD_FUNC)|SHELL_CMD_DISABLE_RETURN,
                    setbandinfo, cmd_setbandinfo, bandid(0x0, 0x3, 0x4));
SHELL_EXPORT_CMD(SHELL_CMD_PERMISSION(0)|SHELL_CMD_TYPE(SHELL_TYPE_CMD_MAIN),
                    stack, cmd_stack_main, para: id cmd);
SHELL_EXPORT_CMD(SHELL_CMD_PERMISSION(0)|SHELL_CMD_TYPE(SHELL_TYPE_CMD_FUNC)|SHELL_CMD_DISABLE_RETURN,
                    set_tod_baudrate, cmd_set_tod_baudrate, baudrate_id(0x0-0x7));
SHELL_EXPORT_CMD(SHELL_CMD_PERMISSION(0)|SHELL_CMD_TYPE(SHELL_TYPE_CMD_FUNC)|SHELL_CMD_DISABLE_RETURN,
                    get_tod_baudrate, cmd_get_tod_baudrate, get tod baudrate);
SHELL_EXPORT_CMD(SHELL_CMD_PERMISSION(0)|SHELL_CMD_TYPE(SHELL_TYPE_CMD_FUNC)|SHELL_CMD_DISABLE_RETURN,
                    set_ant_type, cmd_set_ant_type, ant_type(XL:0, STD:1));
SHELL_EXPORT_CMD(SHELL_CMD_PERMISSION(0)|SHELL_CMD_TYPE(SHELL_TYPE_CMD_FUNC)|SHELL_CMD_DISABLE_RETURN,
                    get_ant_type, cmd_get_ant_type, get ant type);
SHELL_EXPORT_CMD(SHELL_CMD_PERMISSION(0)|SHELL_CMD_TYPE(SHELL_TYPE_CMD_FUNC)|SHELL_CMD_DISABLE_RETURN,
                    set_tod_print_level, cmd_set_tod_print_level, print_type(0x0-0x4));
SHELL_EXPORT_CMD(SHELL_CMD_PERMISSION(0)|SHELL_CMD_TYPE(SHELL_TYPE_CMD_FUNC)|SHELL_CMD_DISABLE_RETURN,
                    get_tod_print_level, cmd_get_tod_print_level, get tod print level);
SHELL_EXPORT_CMD(SHELL_CMD_PERMISSION(0)|SHELL_CMD_TYPE(SHELL_TYPE_CMD_MAIN)|SHELL_CMD_DISABLE_RETURN,
                    uart_at_print, cmd_uart_at_print, para:on/off/1/0);
#ifdef UDP_TOD_TEST
SHELL_EXPORT_CMD(SHELL_CMD_PERMISSION(0)|SHELL_CMD_TYPE(SHELL_TYPE_CMD_MAIN)|SHELL_CMD_DISABLE_RETURN,
                    udp_send_time, cmd_udp_send_time, para:n (default 1, send n packets 5s apart));
#endif

#ifdef PPS_TIME_TEST
SHELL_EXPORT_CMD(SHELL_CMD_PERMISSION(0)|SHELL_CMD_TYPE(SHELL_TYPE_CMD_MAIN)|SHELL_CMD_DISABLE_RETURN,
                    pps_delta_test, cmd_pps_delta_test, (void));
#endif

#endif /* SHELL_CMD_MASTER == 0 */