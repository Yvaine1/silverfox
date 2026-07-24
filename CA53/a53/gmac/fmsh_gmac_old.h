#ifndef FMSH_GMAC_LIST_H
#define FMSH_GMAC_LIST_H

#define RX_NODE_SIZE   0x1000
#define TX_NODE_SIZE   0x400


// 数据地址缓冲区节点结构
typedef struct GmacDataAddrNode {
    void *gmac_data_addr;            // 指向数据存储的地址
    u32 length;                      //  数据长度
    struct GmacDataAddrNode *next;   // 下一个节点指针
} GmacDataAddrNode;

// 数据地址缓冲区链表结构
typedef struct {
    GmacDataAddrNode *head;         // 链表头
    GmacDataAddrNode *tail;         // 链表尾
    size_t count;                   // 节点计数
} DataAddrBuffer;


void *gmac_data_addr_buffer_rx_0_read(DataAddrBuffer *buffer,u32 *out_len);
void *gmac_data_addr_buffer_rx_1_read(DataAddrBuffer *buffer,u32 *out_len);
void *gmac_data_addr_buffer_rx_2_read(DataAddrBuffer *buffer,u32 *out_len);
void *gmac_data_addr_buffer_rx_3_read(DataAddrBuffer *buffer,u32 *out_len);

int gmac_data_addr_buffer_tx_0_write(void *data_addr, u32 len);
int gmac_data_addr_buffer_tx_1_write(void *data_addr, u32 len);
int gmac_data_addr_buffer_tx_2_write(void *data_addr, u32 len);
int gmac_data_addr_buffer_tx_3_write(void *data_addr, u32 len);

u32_t gmac_rx_sem_wait(u32 gmac_index, u32 ultimeout);
size_t gmac_data_addr_rx_buffer_count(u32 gmac_index);

void * gmac_data_read_rx_list(u32 gmac_index, u32 * data_len);

#endif
