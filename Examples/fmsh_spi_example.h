#ifndef _FMSH_SPI_VERIFY_H_ 
#define _FMSH_SPI_VERIFY_H_

#ifdef __cplusplus
extern "C" {
#endif
    
/**********************************Include File*********************************/    
#include "fmsh_spi_lib.h"
#include "fmsh_spi_eeprom.h"
    
/**********************************Constant Definition**************************/
    
/**********************************Type Definition******************************/

/**********************************Macro (inline function) Definition***********/

/**********************************Variable Definition**************************/
       
/**********************************Function Prototype***************************/   
int spi_init(FSpiPs_T *spiPtr, u16 device_id);
int fmsh_spi_verify();


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif 


