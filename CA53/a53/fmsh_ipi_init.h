
#ifndef __FMSH_IPI_INIT_H_
#define __FMSH_IPI_INIT_H_




void ipi_demo();
void prvSendRpuMessage (void *pvParameters);
void prvRecvRpuMessage (void *pvParameters);
void prvSendIpiOneShot (void *pvParameters);







#endif
