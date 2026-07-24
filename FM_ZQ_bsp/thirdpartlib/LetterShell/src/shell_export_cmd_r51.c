/* =============================================================
 * 文件作用：R51 端 shell 命令注册表（A53 转发 / R51 本地共用）
 *
 * 新增一条 R51 命令的步骤：
 *   1) 在 R51 工程的头文件中声明真实函数（原型）
 *   2) 在下方"命令注册区"加一行 SHELL_R51_FWD(attr, name, real_func, desc)
 *   3) 描述字符串格式："para:p1 p2 --- 说明"
 * ============================================================= */
#include "shell.h"

/* ============ Forwarder macros ============ */
#if (SHELL_CMD_MASTER == 0)
    #define SHELL_R51_FWD(_attr, _name, _func, _desc) \
        SHELL_EXPORT_CMD(((_attr) & ~SHELL_CMD_TYPE(0xF)) | SHELL_CMD_TYPE(SHELL_TYPE_CMD_MAIN), _name, _shell_fwd_r51, _desc)
    extern void _shell_fwd_r51(int argc, char *argv[]);
#endif
#if (SHELL_CMD_MASTER == 2)
    #define SHELL_R51_FWD(_attr, _name, _func, _desc) SHELL_EXPORT_CMD(_attr, _name, _func, _desc)
#endif


/* ============ R51-specific includes (only compiled on R51) ============ */
#if (SHELL_CMD_MASTER == 2)

#endif


/* ============ R51 command registrations (add new SHELL_R51_FWD here) ============ */
#if (SHELL_CMD_MASTER == 0) || (SHELL_CMD_MASTER == 2)
extern int test_cmd (int a, int b);


SHELL_R51_FWD(SHELL_CMD_PERMISSION(0) | SHELL_CMD_TYPE(SHELL_TYPE_CMD_FUNC), test_cmd, test_cmd, "para:a b --- this is a test cmd");


#endif
