/*Copyright(C),2016-2022,Nanjing Digitgate Communication Technology Co., LTD. All Rights Reserved.*/
/**
 * @file    dg_common.h
 * @brief   This head file provide common definition  
 */
#ifndef _DG_COMMON_H_
#define _DG_COMMON_H_

/* Includes ------------------------------------------------------------------*/

/* Exported types ------------------------------------------------------------*/


/** 
  * @brief  Common variable type define  
  */
/*******************************************************************/
/*  Caution: All variable declaration must use following type.     */
/*           Otherwise the code merge request will be rejected!!   */
/*******************************************************************/
typedef char                  CHAR;
typedef signed char           INT8;
typedef unsigned char         UINT8;
typedef short                 INT16;
typedef unsigned short        UINT16;
typedef int                   INT32;
typedef unsigned int          UINT32;
typedef long long             INT64;
typedef unsigned long long    UINT64;

/** 
  * @brief  Common return status type define  
  */
/*******************************************************************/
/*  Caution: The API return value must be OK or ERROR              */
/*           for keeping code concise and clear                    */
/*******************************************************************/
typedef enum {
    OK=0,
    ERROR
} STATUS;

typedef enum {
    DISABLE = 0,
    ENABLE  = !DISABLE  
}FUNCSTATE;
/** 
  * @brief  The function pointer define of the interrupt service routine  
  */
typedef void (*isr_callback)(INT32) ;

#ifdef __cplusplus
extern "C" {
#endif

CHAR* get_cmd_param();
/* Exported constants --------------------------------------------------------*/


/* Exported macro ------------------------------------------------------------*/


/* Exported functions --------------------------------------------------------*/


#ifdef __cplusplus
}
#endif

#endif /*_DG_COMMON_H_*/

