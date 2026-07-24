#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <stdbool.h>
#include "fmsh_cache.h"
#include "fmsh_pseudo_asm.h"
#include "nr_shm_oam.h"
#include "ring_buffer.h"
//#include "undefined_para.h"

#define CP_SHM_ADDR_START             0x07000000
#define CP_SHM_ADDR_END               0x08F00000 //31M

#define OAM_SHM_ADDR_START            0x08F00000
#define OAM_SHM_ADDR_END              0x08FFFFFF //1M

#define CP_R50_LOG_SHM_ADDR_START     0x03000000
#define CP_R50_LOG_SHM_ADDR_END       0x03FFFFFF //16M
#define CP_R51_LOG_SHM_ADDR_START     0x04000000
#define CP_R51_LOG_SHM_ADDR_END       0x04FFFFFF //16M

#define CP_SHM_ALIGN_SIZE 8


static char* shm_start_addr_oam = (char*)OAM_SHM_ADDR_START;

static char queue_init_done_oam[MAX_NUM_CELL_PER_DU] = {0};
static ipc_queue_s queue_oam_config[IPC_OAM_QUEUE_NUMBER];
static ipc_queue_s queue_log_config[IPC_LOG_QUEUE_NUMBER];


static void *g_pOamSharedMemAddr[MAX_CELL_NUM][IPC_OAM_QUEUE_NUMBER];
static void *g_pLogSharedMemAddr[MAX_CELL_NUM][IPC_LOG_QUEUE_NUMBER];

static bool shm_init_success_oam = false;

void shm_init_buf_oam(void)
{
    shm_start_addr_oam = (char*)OAM_SHM_ADDR_START;
}

void* shm_get_buf_oam(uint32_t size, int align)
{
    align = (align > sizeof(void*) && align % sizeof(void*) == 0) ? align : sizeof(void*);
    size = (size + align - 1) & ~(align - 1);

    if (shm_start_addr_oam + size > (char*)OAM_SHM_ADDR_END)
    {
        fmsh_print("ERROR: exceed limit size:%u shm_start_addr_oam:%p\r\n", size, (void*)shm_start_addr_oam);
        return NULL;
    }

    void* ret = (void*)shm_start_addr_oam;
    shm_start_addr_oam += size;

    memset(ret, 0, size);
    //Fmsh_DCacheFlushRange((uintptr_t)ret, size);

    return ret;
}

int shm_ipc_init_oam(int32_t cell_id, int32_t mem_reset)
{
    shm_init_buf_oam();

    int ret=IPC_SUCCESS;
    int queue_id = 0, share_memory_size = 0;

    // A53<->R5 shmem
    ipc_queue_s t_oam_queue_para[MSGQ_A53R5_SHMA_OAM_CH_END] = {
        {1, (unsigned int)MSGQ_A53_R51_SHMA_SHELL_CMD_REQ, "a53_r51_shell_command_req",  MAX_NUM_SEG_OAM_PER_SHM, SIZE_SHORT_MESSAGE, 1},
        {1, (unsigned int)MSGQ_R51_A53_SHMA_SHELL_CMD_CNF, "r51_a53_shell_command_resp", MAX_NUM_SEG_OAM_PER_SHM, SIZE_SHORT_MESSAGE, 1},
        {1, (unsigned int)MSGQ_A53_R50_SHMA_SHELL_CMD_REQ, "a53_r50_shell_command_req",  MAX_NUM_SEG_OAM_PER_SHM, SIZE_SHORT_MESSAGE, 1},
        {1, (unsigned int)MSGQ_R50_A53_SHMA_SHELL_CMD_CNF, "r50_a53_shell_command_resp", MAX_NUM_SEG_OAM_PER_SHM, SIZE_SHORT_MESSAGE, 1},
    };

    // A53<->R5 shmem
    ipc_queue_s t_log_queue_para[MSGQ_A53R5_SHMA_OAM_CH_END] = {
        {1, (unsigned int)MSGQ_R50_A53_SHMA_LOG_RPT_IND, "r50_a53_log_rpt_ind", MAX_NUM_SEG_LOG_PER_SHM, SIZE_SHORT_MESSAGE, 1},
        {1, (unsigned int)MSGQ_R51_A53_SHMA_LOG_RPT_IND, "r51_a53_log_rpt_ind", MAX_NUM_SEG_LOG_PER_SHM, SIZE_SHORT_MESSAGE, 1}
    };
 
    memcpy(queue_oam_config, t_oam_queue_para, sizeof(ipc_queue_s) * IPC_OAM_QUEUE_NUMBER);
    memcpy(queue_log_config, t_log_queue_para, sizeof(ipc_queue_s) * IPC_LOG_QUEUE_NUMBER);

    for(queue_id = 0; queue_id < IPC_OAM_QUEUE_NUMBER; queue_id++)
    {
        uint32_t total_size = sizeof(memory_header_s) \
                            + t_oam_queue_para[queue_id].num_seg * (t_oam_queue_para[queue_id].size_buffer_per_seg \
                            + sizeof(uint32_t));
        g_pOamSharedMemAddr[cell_id][queue_id] = shm_get_buf_oam(total_size, CP_SHM_ALIGN_SIZE);
        if(g_pOamSharedMemAddr[cell_id][queue_id] == (void *)NULL)
        {
            fmsh_print("shm_get_buf error for oam queue:%d\r\n",queue_id);
            return false;
        }
    }

    /*
    #define CP_R50_LOG_SHM_ADDR_START     0x6A000000
    #define CP_R50_LOG_SHM_ADDR_END       0x6AFFFFFF //16M
    #define CP_R51_LOG_SHM_ADDR_START     0x6B000000
    #define CP_R51_LOG_SHM_ADDR_END       0x6BFFFFFF //16M
    */
    for(queue_id = 0; queue_id < IPC_LOG_QUEUE_NUMBER; queue_id++)
    {
#if 1
        uint32_t total_size = sizeof(memory_header_s) \
                            + t_log_queue_para[queue_id].num_seg * (t_log_queue_para[queue_id].size_buffer_per_seg \
                            + sizeof(uint32_t));
#endif
        void* log_ptr = NULL;
        if (MSGQ_R50_A53_SHMA_LOG_RPT_IND == queue_id)
        {
            log_ptr = (void*) CP_R50_LOG_SHM_ADDR_START;
        }
        else if (MSGQ_R51_A53_SHMA_LOG_RPT_IND == queue_id)
        {
            log_ptr = (void*) CP_R51_LOG_SHM_ADDR_START;
        }
        else
        {
            fmsh_print("shm_get_buf error for log queue:%d\r\n",queue_id);
            continue;
        }

        if (mem_reset > 0)
        {
            memset(log_ptr, 0, total_size);
        }
        g_pLogSharedMemAddr[cell_id][queue_id] = log_ptr;
        if(g_pLogSharedMemAddr[cell_id][queue_id] == (void *)NULL)
        {
            fmsh_print("shm_get_buf error for log queue:%d\r\n",queue_id);
            return false;
        }
    }

    queue_init_done_oam[cell_id] = 1;
    if (ret == IPC_SUCCESS)
    {
        shm_init_success_oam = true;
    }

    fmsh_print("shm_ipc_init_oam success\r\n");
    return ret;
}

void shm_ipc_exit_oam()
{
    shm_init_success_oam = false;
    
    unsigned int queue_id = 0;
    for(queue_id = 0; queue_id < IPC_OAM_QUEUE_NUMBER; queue_id++)
    {
        for (unsigned int cell_id = 0; cell_id < MAX_CELL_NUM; cell_id++)
        {
            g_pOamSharedMemAddr[cell_id][queue_id] = NULL;
        }
    }

    for(queue_id = 0; queue_id < IPC_LOG_QUEUE_NUMBER; queue_id++)
    {
        for (unsigned int cell_id = 0; cell_id < MAX_CELL_NUM; cell_id++)
        {
            g_pLogSharedMemAddr[cell_id][queue_id] = NULL;
        }
    }

    fmsh_print("Detach all shared memory.\r\n");

    return;
}

/**
 * @brief Send a message to MAC shared memory
 *
 * @param[in]  pBuf          pointer to buffer containing the message
 * @param[in]  inSize        size of the message
 * @param[in]  msgId         message ID
 * @param[in]  cell_id       cell ID
 * @param[in]  offset        offset for the message payload
 * @param[in]  tb            pointer to an rte_mbuf structure for additional message segments
 *
 * @return  0 on success, -1 on failure
 *
 * @details
 *  This function sends a message to MAC shared memory. It checks if the shared memory is 
 *  initialized and if the buffer is valid. It verifies the message size and ensures the 
 *  write index is within bounds. It calculates the message offset, copies the message 
 *  length and payload into shared memory, and updates the write index. If the message 
 *  buffer is full, it logs an error and returns -1. This function handles segmented 
 *  payloads using the rte_mbuf structure.
 */

/*inline*/ int LogShmSend(void *pBuf, uint32_t inSize, uint16_t msgId, uint16_t cell_id, uint16_t offset)
{
    // fmsh_print("\n\rmac send msgId:0x%x\r\n", msgId);

    int msgOffset = 0;
    int rc = 0;
    //unsigned char *ppBuf = pBuf;
    memory_header_s *psShMem = (memory_header_s *)g_pLogSharedMemAddr[cell_id][msgId];

    if  ( UNLIKELY( !shm_init_success_oam || NULL == psShMem ) )
    {
        fmsh_print("ERROR shm_init_success_oam =%d, psShMem[%u][%u] is NULL\r\n", shm_init_success_oam, cell_id, msgId);
        rc=-1;
        return rc;
    }

    if(queue_init_done_oam[cell_id] == 0)
    {
        fmsh_print("Func %s Line %u\r\n",__func__, __LINE__);
        rc=-1;
        return rc;
    }

    if( (rc != -1) && ((NULL == pBuf) || (inSize > queue_log_config[msgId].size_buffer_per_seg) ))
    {
        if(pBuf == NULL)
        {
            fmsh_print("ERROR in flexran_ipc_send_msg pBuf == NULL, queue:%s!\r\n", queue_log_config[msgId].name);
        }
        else
        {
            fmsh_print("ERROR in flexran_ipc_send_msg len:%u, queue Len:%d,queue:%s!\r\n", 
                    inSize, queue_log_config[msgId].size_buffer_per_seg, queue_log_config[msgId].name);
        }

        fmsh_print("Func %s Line %u\r\n",__func__, __LINE__);
        rc=-1;
        return rc;
    }

    // Fmsh_DCacheInvalidateRange((uintptr_t)&psShMem->nReadIdx, sizeof(psShMem->nReadIdx));
    // Fmsh_DCacheInvalidateRange((uintptr_t)&psShMem->nWriteIdx, sizeof(psShMem->nWriteIdx));

    if((rc != -1) && psShMem->nWriteIdx >= queue_log_config[msgId].num_seg)
    {
        fmsh_print("ERROR nWriteIdx:%u of queue:%s.\r\n", psShMem->nWriteIdx, queue_log_config[msgId].name);
        rc=-1;
        return rc;
    }

    if (rc != -1 && queue_log_config[msgId].write_type == 0) {
        if (queue_log_config[msgId].num_seg == 1 && psShMem->nWriteIdx == 1){
            fmsh_print("ERROR LogShmSend Queue %d Full, write index:%u, read index:%u, num_set:%d\r\n",msgId, psShMem->nWriteIdx,
                    psShMem->nReadIdx, queue_log_config[msgId].num_seg);
        } else if (((psShMem->nWriteIdx + 1) % queue_log_config[msgId].num_seg) == psShMem->nReadIdx) {
            fmsh_print("ERROR LogShmSend Queue %d Full, write index:%u, read index:%u, num_set:%d\r\n",msgId, psShMem->nWriteIdx,
                    psShMem->nReadIdx, queue_log_config[msgId].num_seg);
            rc=-1;
            return rc;
        }
    }

    if(rc != -1)
    {
        /*get message offset*/
        msgOffset = psShMem->nWriteIdx * (sizeof(unsigned int) + queue_log_config[msgId].size_buffer_per_seg);

        /*fill in the length*/
        memcpy((psShMem->pMsgBuf + msgOffset), (unsigned char *)&inSize, sizeof(unsigned int));//length of a message

        /*fill in the message payload*/
        if ( offset == 0 )
        {
            memcpy((void *)(psShMem->pMsgBuf + msgOffset + sizeof(unsigned int)), pBuf, inSize);
        }
        else
        {
            rc=-1;
            fmsh_print("LogShmSend, tb is NULL with offset %d for queue:%s\r\n", offset, queue_log_config[msgId].name);
            return rc;
        }

        //Fmsh_DCacheFlushRange((uintptr_t)(psShMem->pMsgBuf + msgOffset), sizeof(unsigned int) + inSize);

        dsb();
        isb();

        /*update nWriteIdx*/
        if (queue_log_config[msgId].num_seg == 1)
        {
            psShMem->nWriteIdx = 1;
        } else
        {
            psShMem->nWriteIdx = (psShMem->nWriteIdx + 1) % queue_log_config[msgId].num_seg;
        }

        //Fmsh_DCacheFlushRange((uintptr_t)&psShMem->nWriteIdx, sizeof(psShMem->nWriteIdx));

        if (psShMem->nWriteIdx == psShMem->nReadIdx)
        {
            if (psShMem->accidentNum % 100 == 0)
            {
                rc = -1;
                fmsh_print("MacShmSend, accident happenning, num=%u, queue:%s\r\n", 
                    psShMem->accidentNum, queue_log_config[msgId].name);
            }
            psShMem->accidentNum++;
            //Fmsh_DCacheFlushRange((uintptr_t)&psShMem->accidentNum, sizeof(psShMem->accidentNum));
        }

    }

    return rc;
}

/*inline*/ void LogShmRecv(void *pBuf, uint32_t *outSize, uint16_t msgId, int16_t *rtc, uint16_t cell_id)
{
    int data_len = 0, msgOffset = 0;

    memory_header_s *psShMem = (memory_header_s *)g_pLogSharedMemAddr[cell_id][msgId];

    if  ( UNLIKELY( !shm_init_success_oam || NULL == psShMem ) )
    {
        fmsh_print("shm_init_success_oam =%d, psShMem[%u][%u] is NULL\r\n", shm_init_success_oam, cell_id, msgId);
        return;
    }


    if(queue_init_done_oam[cell_id] == 0)
    {
        *rtc = IPC_NOT_INIT_ERR;
        fmsh_print("ERROR in queue(%d) not initilized.\r\n",msgId);
        return;
    }

    if(NULL == pBuf)
    {
        fmsh_print("ERROR in MacShmRecv pBuf == NULL.\r\n");
        *rtc = IPC_INPUT_PARA_ERR;
        return;
    }

    //Fmsh_DCacheInvalidateRange((uintptr_t)&psShMem->nReadIdx, sizeof(psShMem->nReadIdx));
    //Fmsh_DCacheInvalidateRange((uintptr_t)&psShMem->nWriteIdx, sizeof(psShMem->nWriteIdx) + sizeof(psShMem->nReadIdx));

    if(psShMem->nReadIdx == psShMem->nWriteIdx)
    {
        // fmsh_print("No msg in queue(%d).\r\n",msgId);
        *rtc = IPC_NOMSG;
        return;
    }

    if(psShMem->nReadIdx >= queue_log_config[msgId].num_seg)
    {
        fmsh_print("ERROR nReadIdx:%d of queue:%s.\r\n", psShMem->nReadIdx, queue_log_config[msgId].name);
        *rtc = IPC_INPUT_PARA_ERR;
        return;
    }

    /*get message offset*/
    msgOffset = psShMem->nReadIdx * (sizeof(unsigned int) + queue_log_config[msgId].size_buffer_per_seg);

    //Fmsh_DCacheInvalidateRange((uintptr_t)(psShMem->pMsgBuf + msgOffset), sizeof(unsigned int) + queue_log_config[msgId].size_buffer_per_seg);
    /*extract the message length*/
    memcpy((unsigned char *)&data_len , (psShMem->pMsgBuf + msgOffset), sizeof(int)) ;

    if((SIZE_BUFFER_PKG < data_len) || (data_len <= 0))
    {
        fmsh_print("ERROR in MacShmRecv, len:%d, data_len:%d.\r\n", SIZE_BUFFER_PKG, data_len);
        *rtc = IPC_RECV_BUF_ERR;
        return ;
    }
    /*extract the message payload*/
    //Fmsh_DCacheInvalidateRange((uintptr_t)(psShMem->pMsgBuf + msgOffset + sizeof(unsigned int)), data_len);

    memcpy(pBuf, (void *)(psShMem->pMsgBuf + msgOffset + sizeof(int)), data_len);

    dsb();
    isb();

    /*update nReadIdx*/
    psShMem->nReadIdx = (psShMem->nReadIdx + 1) % queue_log_config[msgId].num_seg;
    if (queue_log_config[msgId].num_seg == 1)
    {
        psShMem->nWriteIdx = 0;
        //Fmsh_DCacheFlushRange((uintptr_t)&psShMem->nWriteIdx, sizeof(psShMem->nWriteIdx));
    }

    //Fmsh_DCacheFlushRange((uintptr_t)&psShMem->nReadIdx, sizeof(psShMem->nReadIdx));

    *outSize = data_len;
    *rtc=IPC_SUCCESS;
    return;
}


/*inline*/ int OamShmSend(void *pBuf, uint32_t inSize, uint16_t msgId, uint16_t cell_id, uint16_t offset)
{
    int msgOffset = 0;
    int rc = 0;
    unsigned char *ppBuf=pBuf;

    memory_header_s *psShMem = (memory_header_s *)g_pOamSharedMemAddr[cell_id][msgId];

    if  ( UNLIKELY( !shm_init_success_oam || NULL == psShMem ) )
    {
        fmsh_print("ERROR shm_init_success_oam =%d, psShMem[%u][%u] is NULL\r\n", shm_init_success_oam, cell_id, msgId);
        rc=-1;
        return rc;
    }

    if(queue_init_done_oam[cell_id] == 0)
    {
        fmsh_print("Func %s Line %u\r\n",__func__, __LINE__);
        rc=-1;
        return rc;
    }

    if((rc !=-1 ) && ((NULL == pBuf) || (inSize > queue_oam_config[msgId].size_buffer_per_seg)) )
    {
        if(pBuf == NULL)
        {
            fmsh_print("ERROR in flexran_ipc_send_msg pBuf == NULL, queue:%s!\r\n", queue_oam_config[msgId].name);
        }
        else
        {
            fmsh_print("ERROR in flexran_ipc_send_msg len:%u, queue Len:%d,queue:%s!\r\n", 
                    inSize, queue_oam_config[msgId].size_buffer_per_seg, queue_oam_config[msgId].name);
        }

        fmsh_print("Func %s Line %u\r\n",__func__, __LINE__);
        rc=-1;
        return rc;
    }

    //Fmsh_DCacheInvalidateRange((uintptr_t)&psShMem->nWriteIdx, sizeof(psShMem->nWriteIdx));

    if((rc != -1) && (psShMem->nWriteIdx >= queue_oam_config[msgId].num_seg) )
    {
        fmsh_print("ERROR nWriteIdx:%u of queue:%s.\r\n", psShMem->nWriteIdx, queue_oam_config[msgId].name);
        rc=-1;
        return rc;
    }

    if (rc != -1 && queue_oam_config[msgId].write_type == 0)
    {
        if (queue_oam_config[msgId].num_seg == 1 && psShMem->nWriteIdx == 1)
        {
            fmsh_print("ERROR OamShmSend Queue %d Full, write index:%u, read index:%u, num_set:%d\r\n",msgId, psShMem->nWriteIdx,
                    psShMem->nReadIdx, queue_oam_config[msgId].num_seg);
        }
        else if (((psShMem->nWriteIdx + 1) % queue_oam_config[msgId].num_seg) == psShMem->nReadIdx)
        {
            fmsh_print("ERROR OamShmSend Queue %d Full, write index:%u, read index:%u, num_set:%d\r\n",msgId, psShMem->nWriteIdx,
                    psShMem->nReadIdx, queue_oam_config[msgId].num_seg);
            rc=-1;
            return rc;
        }
    }

    if (rc != -1)
    {
        /*get message offset*/
        msgOffset = psShMem->nWriteIdx * (sizeof(unsigned int) + queue_oam_config[msgId].size_buffer_per_seg);

        /*fill in the length*/
        memcpy((psShMem->pMsgBuf + msgOffset), (unsigned char *)&inSize, sizeof(unsigned int));//length of a message

        /*fill in the message payload*/
        memcpy((void *)(psShMem->pMsgBuf + msgOffset + sizeof(unsigned int)), pBuf, inSize);

        //Fmsh_DCacheFlushRange((uintptr_t)(psShMem->pMsgBuf + msgOffset), sizeof(unsigned int) + inSize);

        dsb();
        isb();

        /*update nWriteIdx*/
        if (queue_oam_config[msgId].num_seg == 1)
        {
            psShMem->nWriteIdx = 1;
        }
        else
        {
            psShMem->nWriteIdx = (psShMem->nWriteIdx + 1) % queue_oam_config[msgId].num_seg;
        }

        //Fmsh_DCacheFlushRange((uintptr_t)&psShMem->nWriteIdx, sizeof(psShMem->nWriteIdx));

        if (psShMem->nWriteIdx == psShMem->nReadIdx)
        {
            if (psShMem->accidentNum % 100 == 0)
            {
                fmsh_print("OamShmSend, accident happenning, num=%u\r\n", psShMem->accidentNum);
            }
            psShMem->accidentNum++;
            //Fmsh_DCacheFlushRange((uintptr_t)&psShMem->accidentNum, sizeof(psShMem->accidentNum));
        }
    }
    return rc;
}

/*inline*/ void OamShmRecv(void *pBuf, uint32_t *outSize, uint16_t msgId, int16_t *rtc, uint16_t cell_id)
{
    int data_len = 0, msgOffset = 0;

    memory_header_s *psShMem = (memory_header_s *)g_pOamSharedMemAddr[cell_id][msgId];

    if  ( UNLIKELY( !shm_init_success_oam || NULL == psShMem ) )
    {
        fmsh_print("shm_init_success_oam =%d, psShMem[%u][%u] is NULL\r\n", shm_init_success_oam, cell_id, msgId);
        return;
    }

    if(queue_init_done_oam[cell_id] == 0)
    {
        *rtc = IPC_NOT_INIT_ERR;
        fmsh_print("ERROR in queue(%d) not initilized.\r\n",msgId);
        return;
    }

    if(NULL == pBuf)
    {
        fmsh_print("ERROR in OamShmRecv pBuf == NULL.\r\n");
        *rtc = IPC_INPUT_PARA_ERR;
        return;
    }

    //Fmsh_DCacheInvalidateRange((uintptr_t)&psShMem->nReadIdx, sizeof(psShMem->nReadIdx));
    //Fmsh_DCacheInvalidateRange((uintptr_t)&psShMem->nWriteIdx, sizeof(psShMem->nWriteIdx));

    if(psShMem->nReadIdx == psShMem->nWriteIdx)
    {
        // fmsh_print("No msg in queue(%d).\r\n",msgId);
        *rtc = IPC_NOMSG;
        return;
    }

    if(psShMem->nReadIdx >= queue_oam_config[msgId].num_seg)
    {
        fmsh_print("ERROR nReadIdx:%d of queue:%s.\r\n", psShMem->nReadIdx, queue_oam_config[msgId].name);
        *rtc = IPC_INPUT_PARA_ERR;
        return;
    }

    /*get message offset*/
    msgOffset = psShMem->nReadIdx * (sizeof(unsigned int) + queue_oam_config[msgId].size_buffer_per_seg);

    //Fmsh_DCacheInvalidateRange((uintptr_t)(psShMem->pMsgBuf + msgOffset), sizeof(unsigned int) + queue_oam_config[msgId].size_buffer_per_seg);

    /*extract the message length*/
    memcpy((unsigned char *)&data_len , (psShMem->pMsgBuf + msgOffset), sizeof(int)) ;
    if((SIZE_BUFFER_PKG < data_len) || (data_len <= 0))
    {
        fmsh_print("ERROR in OamShmRecv, len:%d, data_len:%d.\r\n", SIZE_BUFFER_PKG, data_len);
        *rtc = IPC_RECV_BUF_ERR;
        return ;
    }

    /*extract the message payload*/
    memcpy(pBuf, (void *)(psShMem->pMsgBuf + msgOffset + sizeof(int)), data_len);

    dsb();
    isb();

    /*update nReadIdx*/
    psShMem->nReadIdx = (psShMem->nReadIdx + 1) % queue_oam_config[msgId].num_seg;
    if (queue_oam_config[msgId].num_seg == 1)
    {
        psShMem->nWriteIdx = 0;
        //Fmsh_DCacheFlushRange((uintptr_t)&psShMem->nWriteIdx, sizeof(psShMem->nWriteIdx));
    }

    //Fmsh_DCacheFlushRange((uintptr_t)&psShMem->nReadIdx, sizeof(psShMem->nReadIdx));

    *outSize = data_len;
    *rtc=IPC_SUCCESS;
    return;
}

/**
 * @brief Function to send log data from L1c to Log Agent
 *
 * @details This function is used to send data from L1c to MAC
 *
 * @param[in]   pBuf          Pointer to the buffer containing the data to be sent
 * @param[in]   msg_size      Size of the data to be sent
 * @param[in]   msg_id        Message Id of the data to be sent
 * @param[in]   cell_id       Cell ID
 *
 * @return      int           Return code indicating success or failure
 */
int L1cSendToLogAgent(void *p_buf, uint32_t msg_size, uint16_t msg_id, uint16_t cell_id)
{
    return LogShmSend(p_buf, msg_size, msg_id, cell_id, 0);
}

/**
 * @brief Function to receive data from L1c to Log Agent
 *
 * @details This function is used to receive data from L1c to Log Agent
 *
 * @param[in]   p_buf         Pointer to the buffer containing the data
 * @param[out]  msg_size      Size of the data received
 * @param[in]   msg_id        Message Id of the data received
 * @param[out]  rtc           Pointer to receive result code
 * @param[in]   cell_id       Cell ID
 *
 * @return      void
 */
void LogAgentRecvFromL1c(void *p_buf, uint32_t *msg_size, uint16_t msg_id, int16_t *rtc, uint16_t cell_id)
{
    LogShmRecv(p_buf, msg_size, msg_id, rtc, cell_id);
}

void LogAgentRecvTinyLogFromL1c(void *p_buf, uint32_t *msg_size, uint16_t log_channel_id, int16_t *rtc, uint16_t cell_id)
{
    static uint32_t seq_pre = 0;
    uint32_t *p32 = (uint32_t *)p_buf;
    if(queue_init_done_oam[cell_id] == 0)
    {
        *rtc = IPC_NOT_INIT_ERR;
        fmsh_print("ERROR in queue(%d) not initilized.\r\n",log_channel_id);
        return;
    }
    if(NULL == p_buf)
    {
        fmsh_print("ERROR in MacShmRecv pBuf == NULL.\r\n");
        *rtc = IPC_INPUT_PARA_ERR;
        return;
    }
    *msg_size = 0;
     uint32_t addr = CP_R50_LOG_SHM_ADDR_START + sizeof(memory_header_s) +\
            (queue_log_config[MSGQ_R50_A53_SHMA_LOG_RPT_IND].num_seg) * \
            (queue_log_config[MSGQ_R50_A53_SHMA_LOG_RPT_IND].size_buffer_per_seg + sizeof(uint32_t));
    if(log_channel_id == MSGQ_R51_A53_SHMA_LOG_RPT_IND)
    {
        addr = CP_R51_LOG_SHM_ADDR_START + sizeof(memory_header_s) +\
            (queue_log_config[MSGQ_R51_A53_SHMA_LOG_RPT_IND].num_seg) * \
            (queue_log_config[MSGQ_R51_A53_SHMA_LOG_RPT_IND].size_buffer_per_seg + sizeof(uint32_t));
    }
    ring_buffer_t *psShMem = (ring_buffer_t *)addr;
    //ring_buffer_t *psShMem = (ring_buffer_t *)0x37f7610;
    if(ring_buffer_size(psShMem) > SIZE_TINY_LOG_MESSAGE)
    {
        *msg_size = ring_buffer_get_bulk(psShMem,p_buf,SIZE_TINY_LOG_MESSAGE);
    }
    
    if(*msg_size > 0)
    {
        *rtc=IPC_SUCCESS;
        if((seq_pre + 1) != p32[3])
        {
          fmsh_print("A53:%d preSeq %d now %d\r\n",*msg_size,seq_pre,p32[3]);
          
        }
        uint32_t *pTem = (uint32_t *)&(p32[SIZE_TINY_LOG_MESSAGE - 32]);
        seq_pre = pTem[3];
    }
    else
    {
        *rtc = IPC_RECV_BUF_ERR;
        //fmsh_print("ERROR in LogAgentRecvTinyLogFromL1c, msg_size:%d.\r\n",*msg_size);
    }
    return;
}

int L1cSendToOam(void *pBuf, uint32_t dlSize, uint16_t dlMsgId, uint16_t cell_id)
{
    return OamShmSend(pBuf, dlSize, dlMsgId, cell_id, 0);
}

void L1cRecvFromOam(void *pBuf, uint32_t *ulSize, uint16_t ulMsgId, int16_t *rtc, uint16_t cell_id)
{
    OamShmRecv(pBuf, ulSize, ulMsgId, rtc, cell_id);
}

int OamSendToL1c(void *pBuf, uint32_t dlSize, uint16_t dlMsgId, uint16_t cell_id)
{
    return OamShmSend(pBuf, dlSize, dlMsgId, cell_id, 0);
}

void OamRecvFromL1c(void *pBuf, uint32_t *ulSize, uint16_t ulMsgId, int16_t *rtc, uint16_t cell_id)
{
    OamShmRecv(pBuf, ulSize, ulMsgId, rtc, cell_id);
}

char* get_msg_id_name(uint16_t id)
{
    switch (id)
    {
    case MSGQ_R50_A53_SHMA_LOG_RPT_IND:
      return "R50";
    case MSGQ_R51_A53_SHMA_LOG_RPT_IND:
      return "R51";
    default:
      return "Unsupported Msg Id";
    }
}