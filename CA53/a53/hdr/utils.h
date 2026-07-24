#include <stdio.h>

#define   A53_R5_SHELL_CMD     0x0001
#define   A53_R5_ATCMD         0x0002
#define   A53_R5_INTER_MSG     0x0003
#define   A53_R5_LOG_OUTPUT    0x0004


enum shell_cmd_mem_state {IDLE, CMD_WRITE_DONE, CMD_EXECUTING, CMD_EXE_DONE, CMD_RESP_WRITE_DONE};

