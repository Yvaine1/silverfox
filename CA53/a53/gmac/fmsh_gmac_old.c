#include "fmsh_gmac_example.h"
#include "fmsh_gmac_list.h"



#include "sys.h"
#include "semphr.h"
#include "sys_arch.h"
#include "FreeRTOS.h"

xSemaphoreHandle  gmac_tx_0 = NULL;
xSemaphoreHandle  gmac_tx_1 = NULL;
xSemaphoreHandle  gmac_tx_2 = NULL;
xSemaphoreHandle  gmac_tx_3 = NULL;

xSemaphoreHandle  gmac_sem_rx_0 = NULL;
xSemaphoreHandle  gmac_sem_rx_1 = NULL;
xSemaphoreHandle  gmac_sem_rx_2 = NULL;
xSemaphoreHandle  gmac_sem_rx_3 = NULL;

__attribute__ ((section(".addrbuffer"), aligned (64)))
__no_init GmacDataAddrNode node_pool_rx_0[RX_NODE_SIZE] __attribute__ ((aligned (64)));/*  Receive buffer */

__attribute__ ((section(".addrbuffer"), aligned (64)))
__no_init GmacDataAddrNode node_pool_rx_1[RX_NODE_SIZE] __attribute__ ((aligned (64)));/*  Receive buffer */

__attribute__ ((section(".addrbuffer"), aligned (64)))
__no_init GmacDataAddrNode node_pool_rx_2[RX_NODE_SIZE] __attribute__ ((aligned (64)));/*  Receive buffer */

__attribute__ ((section(".addrbuffer"), aligned (64)))
__no_init GmacDataAddrNode node_pool_rx_3[RX_NODE_SIZE] __attribute__ ((aligned (64)));/*  Receive buffer */


__attribute__ ((section(".addrbuffer"), aligned (64)))
__no_init GmacDataAddrNode node_pool_tx_0[TX_NODE_SIZE] __attribute__ ((aligned (64)));/*  Send buffer */

__attribute__ ((section(".addrbuffer"), aligned (64)))
__no_init GmacDataAddrNode node_pool_tx_1[TX_NODE_SIZE] __attribute__ ((aligned (64)));/*  Send buffer */

__attribute__ ((section(".addrbuffer"), aligned (64)))
__no_init GmacDataAddrNode node_pool_tx_2[TX_NODE_SIZE] __attribute__ ((aligned (64)));/*  Send buffer */

__attribute__ ((section(".addrbuffer"), aligned (64)))
__no_init GmacDataAddrNode node_pool_tx_3[TX_NODE_SIZE] __attribute__ ((aligned (64)));/*  Send buffer */


__attribute__((section(".addrbuffer"))) int node_index_rx0;      
__attribute__((section(".addrbuffer"))) int node_index_rx1;     
__attribute__((section(".addrbuffer"))) int node_index_rx2;     
__attribute__((section(".addrbuffer"))) int node_index_rx3;  

__attribute__((section(".addrbuffer"))) int node_index_tx0;      
__attribute__((section(".addrbuffer"))) int node_index_tx1;     
__attribute__((section(".addrbuffer"))) int node_index_tx2;     
__attribute__((section(".addrbuffer"))) int node_index_tx3; 


__attribute__((section(".addrbuffer"))) GmacDataAddrNode *free_list0 = NULL;      
__attribute__((section(".addrbuffer"))) GmacDataAddrNode *free_list1 = NULL;         
__attribute__((section(".addrbuffer"))) GmacDataAddrNode *free_list2 = NULL;      
__attribute__((section(".addrbuffer"))) GmacDataAddrNode *free_list3 = NULL;      

__attribute__((section(".addrbuffer"))) DataAddrBuffer rx_buffer0;       
__attribute__((section(".addrbuffer"))) DataAddrBuffer rx_buffer1; 
__attribute__((section(".addrbuffer"))) DataAddrBuffer rx_buffer2; 
__attribute__((section(".addrbuffer"))) DataAddrBuffer rx_buffer3; 

__attribute__((section(".addrbuffer"))) DataAddrBuffer tx_buffer0;       
__attribute__((section(".addrbuffer"))) DataAddrBuffer tx_buffer1; 
__attribute__((section(".addrbuffer"))) DataAddrBuffer tx_buffer2; 
__attribute__((section(".addrbuffer"))) DataAddrBuffer tx_buffer3;


void gmac_data_addr_buffer0_init() {
 
    rx_buffer0.head = NULL;
    rx_buffer0.tail = NULL;
    rx_buffer0.count = 0;
    node_index_rx0 = 0;
    gmac_sem_rx_0 = xSemaphoreCreateBinary();

    tx_buffer0.head = NULL;
    tx_buffer0.tail = NULL;
    tx_buffer0.count = 0;
    node_index_tx0 = 0;
    gmac_tx_0 = xSemaphoreCreateMutex();

}

void gmac_data_addr_buffer1_init() {
    rx_buffer1.head = NULL;
    rx_buffer1.tail = NULL;
    rx_buffer1.count = 0;
    node_index_rx1 = 0;
    gmac_sem_rx_1 = xSemaphoreCreateBinary();

    tx_buffer1.head = NULL;
    tx_buffer1.tail = NULL;
    tx_buffer1.count = 0;
    node_index_tx1 = 0;
    gmac_tx_1 = xSemaphoreCreateMutex();
}

void gmac_data_addr_buffer2_init() {
    rx_buffer2.head = NULL;
    rx_buffer2.tail = NULL;
    rx_buffer2.count = 0;
    node_index_rx2 = 0;
    gmac_sem_rx_2 = xSemaphoreCreateBinary();

    tx_buffer2.head = NULL;
    tx_buffer2.tail = NULL;
    tx_buffer2.count = 0;
    node_index_tx2 = 0;
    gmac_tx_2 = xSemaphoreCreateMutex();
}

void gmac_data_addr_buffer3_init() {
    rx_buffer3.head = NULL;
    rx_buffer3.tail = NULL;
    rx_buffer3.count = 0;
    node_index_rx3 = 0;
    gmac_sem_rx_3 = xSemaphoreCreateBinary();

    tx_buffer3.head = NULL;
    tx_buffer3.tail = NULL;
    tx_buffer3.count = 0;
    node_index_tx3 = 0;
    gmac_tx_3 = xSemaphoreCreateMutex();
}

// buffer_mode 0:rx 1:tx 
void gmac_data_addr_buffer0_destroy(DataAddrBuffer *buffer,u8 buffer_mode) {
     
    buffer->head = NULL;
    buffer->tail = NULL;
    buffer->count = 0;
    if (buffer_mode)
    {
        node_index_tx0 = 0;
    }
    else 
    {
        node_index_rx0 = 0;
    }
}

void gmac_data_addr_buffer1_destroy(DataAddrBuffer *buffer,u8 buffer_mode) {
    
    buffer->head = NULL;
    buffer->tail = NULL;
    buffer->count = 0;
    if (buffer_mode)
    {
        node_index_tx1 = 0;
    }
    else 
    {
        node_index_rx1 = 0;
    }
}

void gmac_data_addr_buffer2_destroy(DataAddrBuffer *buffer,u8 buffer_mode) {
      
    buffer->head = NULL;
    buffer->tail = NULL;
    buffer->count = 0;
    if (buffer_mode)
    {
        node_index_tx2 = 0;
    }
    else 
    {
        node_index_rx2 = 0;
    }
}

void gmac_data_addr_buffer3_destroy(DataAddrBuffer *buffer,u8 buffer_mode) {
       
    buffer->head = NULL;
    buffer->tail = NULL;
    buffer->count = 0;
    if (buffer_mode)
    {
        node_index_tx3 = 0;
    }
    else 
    {
        node_index_rx3 = 0;
    }
}


int gmac_data_addr_buffer_rx_0_write(DataAddrBuffer *buffer, void *data_addr, u32 len) {
    if (data_addr == NULL) {
        return -1; 
    }
    
    GmacDataAddrNode *new_node;
    
    if (free_list0 != NULL){
      new_node = free_list0;
      free_list0 = free_list0->next;
      
    }else if(node_index_rx0<4096){     
      new_node= &node_pool_rx_0[node_index_rx0++];
    }else {
      return -1;
    }

    
    new_node->gmac_data_addr = data_addr;
    new_node->length =len;
    new_node->next = NULL;

    if (buffer->tail == NULL) {
        buffer->head = new_node;
        buffer->tail = new_node;
    } else {
        buffer->tail->next = new_node;
        buffer->tail = new_node;
    }
    buffer->count++;

    return 0;
}

int gmac_data_addr_buffer_rx_1_write(DataAddrBuffer *buffer, void *data_addr, u32 len) {
    if (data_addr == NULL) {
        return -1;
    }    
    GmacDataAddrNode *new_node;
    
    if (free_list1 != NULL){
      new_node = free_list1;
      free_list1 = free_list1->next;
      
    }else if(node_index_rx1<4096){     
      new_node= &node_pool_rx_1[node_index_rx1++];
    }else {
      return -1;
    }

    
    new_node->gmac_data_addr = data_addr;
    new_node->length =len;
    new_node->next = NULL;

    if (buffer->tail == NULL) {
        buffer->head = new_node;
        buffer->tail = new_node;
    } else {
        buffer->tail->next = new_node;
        buffer->tail = new_node;
    }
    buffer->count++;

    return 0;
}
int gmac_data_addr_buffer_rx_2_write(DataAddrBuffer *buffer, void *data_addr, u32 len) {
    if (data_addr == NULL) {
        return -1;
    }
    
    GmacDataAddrNode *new_node;
    
    if (free_list2 != NULL){
      new_node = free_list2;
      free_list2 = free_list2->next;
      
    }else if(node_index_rx2<4096){     
      new_node= &node_pool_rx_2[node_index_rx2++];
    }else {
      return -1;
    }
   
    new_node->gmac_data_addr = data_addr;
    new_node->length =len;
    new_node->next = NULL;

    if (buffer->tail == NULL) {
        buffer->head = new_node;
        buffer->tail = new_node;
    } else {
        buffer->tail->next = new_node;
        buffer->tail = new_node;
    }
    buffer->count++;

    return 0;
}
int gmac_data_addr_buffer_rx_3_write(DataAddrBuffer *buffer, void *data_addr, u32 len) {
    if (data_addr == NULL) {
        return -1;
    }
    
    GmacDataAddrNode *new_node;
    
    if (free_list3 != NULL){
      new_node = free_list3;
      free_list3 = free_list3->next;
      
    }else if(node_index_rx3<4096){     
      new_node= &node_pool_rx_3[node_index_rx3++];
    }else {
      return -1;
    }  
    
    new_node->gmac_data_addr = data_addr;
    new_node->length =len;
    new_node->next = NULL;

    if (buffer->tail == NULL) {
        buffer->head = new_node;
        buffer->tail = new_node;
    } else {
        buffer->tail->next = new_node;
        buffer->tail = new_node;
    }
    buffer->count++;

    return 0;
}

void *gmac_data_addr_buffer_rx_0_read(DataAddrBuffer *buffer,u32 *out_len) {

    
    if (buffer->head == NULL) {
        return NULL;
    }
    
    GmacDataAddrNode *node = buffer->head;
    void *gmac_data_addr = node->gmac_data_addr;
    *out_len = node->length;
    
    buffer->head = node->next;
    if (buffer->head == NULL) {
        buffer->tail = NULL;
    }
    buffer->count--;
       
    return (void*)((UINTPTR)gmac_data_addr & ~(UINTPTR)1);
}


void *gmac_data_addr_buffer_rx_1_read(DataAddrBuffer *buffer,u32 *out_len) {
  
    if (buffer->head == NULL) {
        return NULL;
    }
    
    GmacDataAddrNode *node = buffer->head;
    void *gmac_data_addr = node->gmac_data_addr;
    *out_len = node->length;
    
    buffer->head = node->next;
    if (buffer->head == NULL) {
        buffer->tail = NULL;
    }
    buffer->count--;

    node->next = free_list1;
    free_list1 = node;
       
    return (void*)((UINTPTR)gmac_data_addr & ~(UINTPTR)1);
}

void *gmac_data_addr_buffer_rx_2_read(DataAddrBuffer *buffer,u32 *out_len) {

    
    if (buffer->head == NULL) {
        return NULL; 
    }
    
    GmacDataAddrNode *node = buffer->head;
    void *gmac_data_addr = node->gmac_data_addr;
    *out_len = node->length;
    
    buffer->head = node->next;
    if (buffer->head == NULL) {
        buffer->tail = NULL;
    }
    buffer->count--;

    node->next = free_list2;
    free_list2 = node;
    
    
    return (void*)((UINTPTR)gmac_data_addr & ~(UINTPTR)1);
}

void *gmac_data_addr_buffer_rx_3_read(DataAddrBuffer *buffer,u32 *out_len) {

    
    if (buffer->head == NULL) {
        return NULL;
    }
    
    GmacDataAddrNode *node = buffer->head;
    void *gmac_data_addr = node->gmac_data_addr;
    *out_len = node->length;
    
    buffer->head = node->next;
    if (buffer->head == NULL) {
        buffer->tail = NULL;
    }
    buffer->count--;

    node->next = free_list3;
    free_list3 = node;
    
    
    return (void*)((UINTPTR)gmac_data_addr & ~(UINTPTR)1);
}

void * gmac_data_read_rx_list(u32 gmac_index, u32 * data_len)
{
    void  *tmp_addr;
    switch(gmac_index)
    {
        case 0: {
            tmp_addr = gmac_data_addr_buffer_rx_0_read(&rx_buffer0,data_len); 
            break;}
        case 1: {
            tmp_addr = gmac_data_addr_buffer_rx_1_read(&rx_buffer1,data_len); 
            break;}
        case 2: {
            tmp_addr = gmac_data_addr_buffer_rx_2_read(&rx_buffer2,data_len); 
            break;}
        case 3: {
            tmp_addr = gmac_data_addr_buffer_rx_3_read(&rx_buffer3,data_len); 
            break;}
        default: 
            tmp_addr = NULL;
    }

    return tmp_addr;
}

size_t gmac_data_addr_rx_buffer_count(u32 gmac_index) {

    DataAddrBuffer *buffer;
    switch(gmac_index)
    {
        case 0: {
            buffer = &rx_buffer0;
            break;}
        case 1: {
            buffer = &rx_buffer1;
            break;}
        case 2: {
            buffer = &rx_buffer2;
            break;}
        case 3: {
            buffer = &rx_buffer3;
            break;}
        default:
            buffer = &rx_buffer0;
            break;
    }
    size_t count = buffer->count;

    return count;
}

int gmac_data_addr_buffer_tx_0_write(void *data_addr, u32 len)
{
    if (data_addr == NULL) 
    {
        return FMSH_FAILURE;
    }

    DataAddrBuffer *buffer = &tx_buffer0;
    GmacDataAddrNode *new_node;

    if (buffer->count <= 1024)
    {
        len %= 0x680;
        new_node= &node_pool_tx_0[node_index_tx0];
        new_node->gmac_data_addr = data_addr;
        new_node->length =len;
        new_node->next = NULL;
        node_index_tx0 += 1;
        node_index_tx0 %= 1024;
    }
    else 
    {
        return FMSH_FAILURE;
    }

    if (buffer->tail == NULL) {
        buffer->head = new_node;
        buffer->tail = new_node;
    } else {
        buffer->tail->next = new_node;
        buffer->tail = new_node;
    }
    buffer->count++;

    return FMSH_SUCCESS;
}

int gmac_data_addr_buffer_tx_1_write(void *data_addr, u32 len)
{
    if (data_addr == NULL) 
    {
        return FMSH_FAILURE;
    }

    DataAddrBuffer *buffer = &tx_buffer1;
    GmacDataAddrNode *new_node;

    if (buffer->count <= 1024)
    {
        len %= 0x680;
        new_node= &node_pool_tx_1[node_index_tx1];
        new_node->gmac_data_addr = data_addr;
        new_node->length =len;
        new_node->next = NULL;
        node_index_tx1 += 1;
        node_index_tx1 %= 1024;
    }
    else 
    {
        return FMSH_FAILURE;
    }

    if (buffer->tail == NULL) {
        buffer->head = new_node;
        buffer->tail = new_node;
    } else {
        buffer->tail->next = new_node;
        buffer->tail = new_node;
    }
    buffer->count++;

    return FMSH_SUCCESS;
}

int gmac_data_addr_buffer_tx_2_write(void *data_addr, u32 len)
{
    if (data_addr == NULL) 
    {
        return FMSH_FAILURE;
    }

    if (gmac_tx_2 != NULL)
    {
        xSemaphoreTake(gmac_tx_2,0);
    }
    else
    {
        return FMSH_FAILURE;
    }

    DataAddrBuffer *buffer = &tx_buffer2;
    GmacDataAddrNode *new_node;

    if (buffer->count <= 1024)
    {
        len %= 0x680;
        new_node= &node_pool_tx_2[node_index_tx2];
        new_node->gmac_data_addr = data_addr;
        new_node->length =len;
        new_node->next = NULL;
        node_index_tx2 += 1;
        node_index_tx2 %= 1024;
    }
    else 
    {
        return FMSH_FAILURE;
    }

    if (buffer->tail == NULL) {
        buffer->head = new_node;
        buffer->tail = new_node;
    } else {
        buffer->tail->next = new_node;
        buffer->tail = new_node;
    }
    buffer->count++;

    return FMSH_SUCCESS;
}

int gmac_data_addr_buffer_tx_3_write(void *data_addr, u32 len)
{
    if (data_addr == NULL) 
    {
        return FMSH_FAILURE;
    }

    if (gmac_tx_3 != NULL)
    {
        xSemaphoreTake(gmac_tx_3,0);
    }
    else
    {
        return FMSH_FAILURE;
    }

    DataAddrBuffer *buffer = &tx_buffer3;
    GmacDataAddrNode *new_node;

    if (buffer->count <= 1024)
    {
        len %= 0x680;
        new_node= &node_pool_tx_3[node_index_tx3];
        new_node->gmac_data_addr = data_addr;
        new_node->length =len;
        new_node->next = NULL;
        node_index_tx3 += 1;
        node_index_tx3 %= 1024;
    }
    else 
    {
        return FMSH_FAILURE;
    }

    if (buffer->tail == NULL) {
        buffer->head = new_node;
        buffer->tail = new_node;
    } else {
        buffer->tail->next = new_node;
        buffer->tail = new_node;
    }
    buffer->count++;

    return FMSH_SUCCESS;
}

void *gmac_data_addr_buffer_tx_0_read(u32 *out_len) {

    DataAddrBuffer *buffer = &tx_buffer0;

    if (buffer->head == NULL) {
        return NULL;
    }
    
    GmacDataAddrNode *node = buffer->head;
    void *gmac_data_addr = node->gmac_data_addr;
    *out_len = node->length;
    
    buffer->head = node->next;
    if (buffer->head == NULL) {
        buffer->tail = NULL;
    }
    buffer->count--;
    
    return (void*)((UINTPTR)gmac_data_addr);
}

void *gmac_data_addr_buffer_tx_1_read(u32 *out_len) {

    DataAddrBuffer *buffer = &tx_buffer1;

    if (buffer->head == NULL) {
        return NULL;
    }
    
    GmacDataAddrNode *node = buffer->head;
    void *gmac_data_addr = node->gmac_data_addr;
    *out_len = node->length;
    
    buffer->head = node->next;
    if (buffer->head == NULL) {
        buffer->tail = NULL;
    }
    buffer->count--;
    
    return (void*)((UINTPTR)gmac_data_addr);
}

void *gmac_data_addr_buffer_tx_2_read(u32 *out_len) {

    DataAddrBuffer *buffer = &tx_buffer2;

    if (buffer->head == NULL) {
        return NULL;
    }
    
    GmacDataAddrNode *node = buffer->head;
    void *gmac_data_addr = node->gmac_data_addr;
    *out_len = node->length;
    
    buffer->head = node->next;
    if (buffer->head == NULL) {
        buffer->tail = NULL;
    }
    buffer->count--;
    
    return (void*)((UINTPTR)gmac_data_addr);
}

void *gmac_data_addr_buffer_tx_3_read(u32 *out_len) {
   
    DataAddrBuffer *buffer = &tx_buffer3;

    if (buffer->head == NULL) {
        return NULL;
    }
    
    GmacDataAddrNode *node = buffer->head;
    void *gmac_data_addr = node->gmac_data_addr;
    *out_len = node->length;
    
    buffer->head = node->next;
    if (buffer->head == NULL) {
        buffer->tail = NULL;
    }
    buffer->count--;
    
    return (void*)((UINTPTR)gmac_data_addr);
}

size_t gmac_data_addr_tx_buffer_count(u32 gmac_index) {

    DataAddrBuffer *buffer;
    xSemaphoreHandle *temp_lock;
    switch(gmac_index)
    {
        case 0: {
            buffer = &tx_buffer0;
            break;}
        case 1: {
            buffer = &tx_buffer1;
            break;}
        case 2: {
            buffer = &tx_buffer2;
            break;}
        case 3: {
            buffer = &tx_buffer3;
            break;}
        default:
            buffer = &tx_buffer0;
            break;
    }

    size_t count = buffer->count;

    return count;
}

/*---------------------------------------------------------------------------*
* Routine:  gmac_rx_sem_wait
*---------------------------------------------------------------------------*
* Description:
*      Blocks the thread while waiting for the semaphore to be
*      signaled. If the "timeout" argument is non-zero, the thread should
*      only be blocked for the specified time (measured in
*      milliseconds).
*
*      If the timeout argument is non-zero, the return value is the number of
*      milliseconds spent waiting for the semaphore to be signaled. If the
*      semaphore wasn't signaled within the specified time, the return value is
*      SYS_ARCH_TIMEOUT. If the thread didn't have to wait for the semaphore
*      (i.e., it was already signaled), the function may return zero.
*
* Inputs:
*      u32 gmac_index          -- Semaphore of gmac taged to wait on
*      u32_t timeout           -- Number of milliseconds until timeout
* Outputs:
*      u32_t                   -- Time elapsed or SYS_ARCH_TIMEOUT.
*---------------------------------------------------------------------------*/
u32_t gmac_rx_sem_wait(u32 gmac_index, u32 ulTimeout)
{
    portTickType xStartTime, xEndTime, xElapsed;
    unsigned long ulReturn;
    portBASE_TYPE xHigherPriorityTaskWoken = pdFALSE;
    
    xStartTime = xTaskGetTickCount();
  
    xSemaphoreHandle* sem_tmp = NULL;
    switch(gmac_index)
    {
        case 0: {
            sem_tmp = &gmac_sem_rx_0;
            break;
        }
        case 1: {
            sem_tmp = &gmac_sem_rx_1;
            break;
        }
        case 2: {
            sem_tmp = &gmac_sem_rx_2;
            break;
        }
        case 3: {
            sem_tmp = &gmac_sem_rx_3;
            break;
        }
        default:
            break;
    }
    
    if (NULL == sem_tmp) 
    {
        return 0;
    }
  
  if( ulTimeout != 0UL )
  {
    if( xSemaphoreTake( *sem_tmp, ulTimeout / portTICK_RATE_MS ) == pdTRUE )
    {
       xEndTime = xTaskGetTickCount();
       xElapsed = (xEndTime - xStartTime) * portTICK_RATE_MS;
       ulReturn = xElapsed;
    }
    else
    {
       ulReturn = SYS_ARCH_TIMEOUT;
    }
  }
  else
  {
    xSemaphoreTake( *sem_tmp, portMAX_DELAY );
    xEndTime = xTaskGetTickCount();
    xElapsed = ( xEndTime - xStartTime ) * portTICK_RATE_MS;
    
    if( xElapsed == 0UL )
    {
      xElapsed = 1UL;
    }
    
    ulReturn = xElapsed;
  }
  
  return ulReturn;
}
