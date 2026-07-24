#ifndef _NR_SHM_OAM_H_
#define _NR_SHM_OAM_H_

#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>

#define MAX_CELL_NUM 1

#define LIKELY(x)		__builtin_expect(!!(x), 1)
#define UNLIKELY(x)		__builtin_expect(!!(x), 0)

typedef enum
{
    //* A53 <=> R51 shell_agent
    MSGQ_A53_R51_SHMA_SHELL_CMD_REQ = 0,
    MSGQ_R51_A53_SHMA_SHELL_CMD_CNF,

    //* A53 <=> R50 shell_agent
    MSGQ_A53_R50_SHMA_SHELL_CMD_REQ,
    MSGQ_R50_A53_SHMA_SHELL_CMD_CNF,

    MSGQ_A53R5_SHMA_OAM_CH_END,
} EnumShmQueueOAM;

typedef enum
{
    //* R51 <=> A53 log collection
    MSGQ_R50_A53_SHMA_LOG_RPT_IND = 0,
    MSGQ_R51_A53_SHMA_LOG_RPT_IND,
    MSGQ_A53R5_SHMA_LOG_CH_END,
} EnumShmQueueLog;

#define MAX_NUM_CELL_PER_DU                 1

#define IPC_MAC_QUEUE_NUMBER MSGQ_MACPHY_SHMA_CH_END
#define IPC_RRC_QUEUE_NUMBER MSGQ_RRCPHY_SHMA_CH_END
#define IPC_L1C_QUEUE_NUMBER MSGQ_R50R51_SHMA_CH_END
#define IPC_OAM_QUEUE_NUMBER MSGQ_A53R5_SHMA_OAM_CH_END
#define IPC_LOG_QUEUE_NUMBER MSGQ_A53R5_SHMA_LOG_CH_END
#define IPC_USED_BY_PHY 0
#define IPC_USED_BY_MAC 1
#define IPC_USED_BY_RRC 2

#define MAX_NUM_SEG_PER_SHM 10
#define SIZE_LONG_MESSAGE (1024*(24*6+5))
#define SIZE_SHORT_MESSAGE (256)
#define SIZE_NORMAL_MESSAGE 2500
#define SIZE_TINY_LOG_MESSAGE   (1024*4)
#define SIZE_BUFFER_PKG 50000
#define BYTES_LEN_BUFFER sizeof(uint64_t)
//#define MAX_NUM_SEG_LOG_PER_SHM  64526
#define MAX_NUM_SEG_LOG_PER_SHM  32128
#define MAX_NUM_SEG_OAM_PER_SHM  128

#define IPC_SUCCESS (1)
#define IPC_NOMSG (0)
#define IPC_NOT_INIT_ERR (-1)
#define IPC_INPUT_PARA_ERR (-2)
#define IPC_SEND_QUEUE_FULL_ERR (-3)
#define IPC_SEND_EDQUOT_ERR (-4)
#define IPC_SEND_ENOBUFS_ERR (-5)
#define IPC_RECV_BUF_ERR (-6)
#define IPC_UNKNOWN_ERR (-7)

typedef struct
{
    unsigned int flag_create;
    unsigned int id;
    char* name;
    unsigned int num_seg;
    unsigned int size_buffer_per_seg;
    unsigned char write_type; // 0: discasd,not write. 1: cover write
}ipc_queue_s;

typedef struct
{
    /*write index, can only be updated by sending side after sending a message*/
    unsigned int nWriteIdx;
    
    /*read index, can only be updated by receiving side after received a message,
        no messages when nWriteIdx = nReadIdx*/
    unsigned int nReadIdx;

    /* unused now */
    unsigned int accidentNum;

    /*in each message, the 1st 4 byte is length, remaining bytes are payload*/
    /* Variable length and payload field:
        Each PDUData block includes:
            1)UWORD32 nLen: message length
            2)UWORD8 pMsg[nLen]: message payload
            |-|---|-|-|---|-------|---|-------------|---------------|
Byte         |0|...|3|4|...|3+nLen|....| ... ...     |ring counter-1|
            |--------|------------|---|-----------------------------|
pMsgBuf     |nLen- |ringSize |.....    | ... ... |
            |--------|------------|---|-----------------------------|
            |---No.1 message ------|.... |---No.(ring counter-1) message--|*/
    unsigned char pMsgBuf[1];
}memory_header_s;

int L1cSendToLogAgent(void *pBuf, uint32_t dlSize, uint16_t dlMsgId, uint16_t cell_id);
void LogAgentRecvFromL1c(void *pBuf, uint32_t *ulSize, uint16_t ulMsgId, int16_t *rtc, uint16_t cell_id);
void LogAgentRecvTinyLogFromL1c(void *p_buf, uint32_t *msg_size, uint16_t log_channel_id, int16_t *rtc, uint16_t cell_id);

int L1cSendToOam(void *pBuf, uint32_t dlSize, uint16_t dlMsgId, uint16_t cell_id);
int OamSendToL1c(void *pBuf, uint32_t dlSize, uint16_t dlMsgId, uint16_t cell_id);
void L1cRecvFromOam(void *pBuf, uint32_t *ulSize, uint16_t ulMsgId, int16_t *rtc, uint16_t cell_id);
void OamRecvFromL1c(void *pBuf, uint32_t *ulSize, uint16_t ulMsgId, int16_t *rtc, uint16_t cell_id);

int shm_ipc_init_oam(int32_t mac_cell_id, int32_t mem_reset);
void shm_ipc_exit_oam();

char* get_msg_id_name(uint16_t id);

#endif
