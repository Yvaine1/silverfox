/******************************************************************************
 *
 * Copyright (C) 2018 - 2028 FMSH, Inc.  All rights reserved.
 *
 ******************************************************************************/
/*****************************************************************************/
/**
 * @file  fmsh_efuse.c
 *
 * This file contains
 *
 * @note		None.
 *
 * MODIFICATION HISTORY:
 *
 *<pre>
 * Ver   Who  Date     Changes
 * ----- ---- -------- ---------------------------------------------
 * 0.01   lq  6/23/2021  First Release
 *</pre>
 ******************************************************************************/

/***************************** Include Files *********************************/
#include "fmsh_efuse.h"
#include "fmsh_psu_parameters.h"

/************************** Constant Definitions *****************************/
#define SAC_EFUSE_CTRL_OFFSET (0x200)
#define EFUSE_POLL_READ_TIMEOUT_VAL  0xFFFFFF
#define EFUSE_POLL_WRITE_TIMEOUT_VAL 0xFFFFFF
#define EFUSE_POLL_LOAD_TIMEOUT_VAL 0xFFFFFFFF
/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

/************************** Variable Definitions *****************************/

/****************************************************************************/
/**
 *
* @description
*  This functions read efuse data.
*
* @param   
*  bRowAddr is the row address in the range of 0 to 255. 
*  lRdData  is specified address data.
*
* @return  
*  -FMSH_SUCCESS  -- if successful
*  -FMSH_FAILURE  -- if timeout
*
* @note    
*  
* SOURCE
 *
 ****************************************************************************/
uint32_t FEfusePs_readData (uint32_t bRowAddr,uint32_t* lRdData)
{
    uint32_t status_reg = 0U;
    uint32_t timeout=EFUSE_POLL_READ_TIMEOUT_VAL;
    
    FEfusePs_Portmap_T
        *pPortmap = (FEfusePs_Portmap_T *)(FPS_CSU_BASEADDR +
                                           SAC_EFUSE_CTRL_OFFSET);
    *lRdData=0;
    EFUSE_OUTP(bRowAddr, pPortmap->rd_addr);

    status_reg = EFUSE_INP(pPortmap->status);
    while((status_reg & EFUSE_STATUS_RD_DONE_MASK)!=EFUSE_STATUS_RD_DONE_MASK)
    {
       status_reg = EFUSE_INP(pPortmap->status);
       timeout--;
       if(timeout==0){
          return FMSH_FAILURE;
       }
    }

    *lRdData = EFUSE_INP(pPortmap->rd_data);
    return FMSH_SUCCESS;
}


/****************************************************************************/
/**
 *
 * Program the data in efuse.
 *
 * @param    cs is chip select.
 * @param    bRowAddr is row address.
 * @param    bStartCol is first index of col.
 * @param    bEndCol is end index of col.
 * @param    lWrdatav is date for writing to the row.
 *
 * @return   none.
 *
 * @note     None.
 *
 ****************************************************************************/
uint32_t FEfusePs_programRowData (enum FMSH_efuse_chip cs, uint32_t bRowAddr,
                              uint8_t bStartCol, uint8_t bEndCol,
                              uint32_t lWrdata)
{
    uint32_t status_reg = 0;
    uint8_t indexCol = 0;
    uint32_t flag = 0;
    uint32_t ldata = lWrdata << bStartCol;
    uint32_t timeout=EFUSE_POLL_WRITE_TIMEOUT_VAL; 
    uint32_t Status=FMSH_SUCCESS;
    uint32_t read_data=0;
    FEfusePs_Portmap_T
        *pPortmap = (FEfusePs_Portmap_T *)(FPS_CSU_BASEADDR +
                                           SAC_EFUSE_CTRL_OFFSET);

    // efuse0 ps
    if (cs == Efuse_chip_0)
    {
        EFUSE_OUTP(0x11, pPortmap->cfg);
    }
    else
    {
        EFUSE_OUTP(0x21, pPortmap->cfg);
    }

    if( (bStartCol > 31) || (bEndCol > 31) || (bStartCol > bEndCol) )
    {
        return FMSH_FAILURE;
    }

    if(bRowAddr>7)
    {
      Status=FEfusePs_readData(bRowAddr,&read_data);
      if(Status!=FMSH_SUCCESS){
         return Status;
      }
    }
    
    for (indexCol = bStartCol; indexCol <= bEndCol; indexCol++)
    {
        flag = 1 << indexCol;
        if(read_data&flag){
            continue;
        }
        
        if (ldata & flag)
        {
            // write
            EFUSE_OUTP(bRowAddr + (indexCol << 8), pPortmap->pgm_addr);

            do
            {
                status_reg = EFUSE_INP(pPortmap->status);
                // pgm_lock
                if((status_reg & EFUSE_STATUS_PGM_LOCK_MASK)==EFUSE_STATUS_PGM_LOCK_MASK)
                {
                    break;
                }
                timeout--;
                if(timeout==0){
                    return FMSH_FAILURE;
                }
            } while((status_reg & EFUSE_STATUS_PGM_DONE_MASK)!=EFUSE_STATUS_PGM_DONE_MASK);
        }
    }
    return FMSH_SUCCESS;
}

/*****************************************************************************
*
* @description
*  This functions program one row's data in the specified address of efuse.
*
* @param   
*  bRowAddr is the row address.
*  lWrdata is the program data.
*
* @return  
*  -FMSH_SUCCESS  -- if successful
*  -FMSH_FAILURE  -- if timeout
*
* @note    
*  
* SOURCE
*
****************************************************************************/
uint32_t FEfusePs_writeRow (uint32_t bRowAddr, uint32_t lWrdata)
{
    uint32_t status_reg=0;
    uint32_t read_data=0;
    uint32_t Status=FMSH_SUCCESS;
    uint32_t timeout=EFUSE_POLL_WRITE_TIMEOUT_VAL; 
    
    uint8_t indexCol=0;
    uint32_t flag=0;
    FEfusePs_Portmap_T
        *pPortmap = (FEfusePs_Portmap_T *)(FPS_CSU_BASEADDR +
                                           SAC_EFUSE_CTRL_OFFSET);
    if(bRowAddr>7)
    {
      Status=FEfusePs_readData(bRowAddr,&read_data);
      if(Status!=FMSH_SUCCESS){
         return Status;
      }
    }
    
    //efuse0 ps
    EFUSE_OUTP(EFUSE_CFG_0_PGM_MASK,pPortmap->cfg);
    for(indexCol=0;indexCol<=31;indexCol++)
    {
      flag=1<<indexCol;
      //exist
      if(read_data&flag)
      {
        continue;
      }
      else if(lWrdata&flag)
      {
        timeout=EFUSE_POLL_WRITE_TIMEOUT_VAL;
        //write
        EFUSE_OUTP(bRowAddr+(indexCol<<8),pPortmap->pgm_addr);
             
        do{
           status_reg = EFUSE_INP(pPortmap->status);
           //pgm_lock
           if((status_reg & EFUSE_STATUS_PGM_LOCK_MASK)==EFUSE_STATUS_PGM_LOCK_MASK){
             break;
           }
           timeout--;
           if(timeout==0){
              return FMSH_FAILURE;
           }
        }while((status_reg & EFUSE_STATUS_PGM_DONE_MASK)!=EFUSE_STATUS_PGM_DONE_MASK);
      }
      else{
        ;/* no deal with */
      }
    }

#ifdef DOUBLE_BIT  
    //efuse1 ps
    EFUSE_OUTP(EFUSE_CFG_1_PGM_MASK,pPortmap->cfg);
    
    for(indexCol=0;indexCol<=31;indexCol++)
    {
      flag=1<<indexCol;
      //exist
      if(read_data&flag)
      {
        continue;
      }
      else if(lWrdata&flag)
      {
        timeout=EFUSE_POLL_WRITE_TIMEOUT_VAL;
        //write
        EFUSE_OUTP(bRowAddr+(indexCol<<8),pPortmap->pgm_addr);
             
        do{
           status_reg = EFUSE_INP(pPortmap->status);
           //pgm_lock
           if((status_reg & EFUSE_STATUS_PGM_LOCK_MASK)==EFUSE_STATUS_PGM_LOCK_MASK)
             break;
           timeout--;
           if(timeout==0)
              return FMSH_FAILURE;
        }while((status_reg & EFUSE_STATUS_PGM_DONE_MASK)!=EFUSE_STATUS_PGM_DONE_MASK);
      }
    }
#endif
    return FMSH_SUCCESS;
}

/*****************************************************************************
*
* @description
*  This functions load the data of efuse to the reg.
*
* @param   
*  none
*
* @return  
*  -FMSH_SUCCESS  -- if successful
*  -FMSH_FAILURE  -- if timeout
*
* @note    
*  
* SOURCE
*
****************************************************************************/
uint32_t FEfusePs_cacheLoad (void)
{
    uint32_t status_reg = 0;
    uint32_t timeout=EFUSE_POLL_LOAD_TIMEOUT_VAL; 
    FEfusePs_Portmap_T
        *pPortmap = (FEfusePs_Portmap_T *)(FPS_CSU_BASEADDR +
                                           SAC_EFUSE_CTRL_OFFSET);
    // load
    EFUSE_OUTP(0x11, pPortmap->cache_load);

    do
    {
        status_reg = EFUSE_INP(pPortmap->status);
        timeout--;
       if(timeout==0)
          return FMSH_FAILURE;
    }while((status_reg & EFUSE_STATUS_CACHE_DONE_MASK)!=EFUSE_STATUS_CACHE_DONE_MASK);
    
    return FMSH_SUCCESS;
}
 
/*****************************************************************************
*
* @description
*  This functions is used to check key crc.
*
* @param   
*  crcValue is calculated by function. 
*
* @return  
*  -FMSH_SUCCESS  -- if successful
*  -FMSH_FAILURE  -- if failed
*
* @note     
*
* SOURCE
*
****************************************************************************/
uint32_t FEfusePs_crcCheck (uint32_t crcValue)
{
    uint32_t status_reg = 0;
    uint32_t timeout=EFUSE_POLL_LOAD_TIMEOUT_VAL; 
    FEfusePs_Portmap_T
        *pPortmap = (FEfusePs_Portmap_T *)(FPS_CSU_BASEADDR +
                                           SAC_EFUSE_CTRL_OFFSET);

    EFUSE_OUTP(crcValue, pPortmap->aes_crc);

    do
    {
       status_reg = EFUSE_INP(pPortmap->status);
       timeout--;
       if(timeout==0){
            return FMSH_FAILURE;
       }
    }while((status_reg & EFUSE_STATUS_CRC_DONE_MASK)!=EFUSE_STATUS_CRC_DONE_MASK);

    status_reg = EFUSE_INP(pPortmap->status);
    if((status_reg&EFUSE_STATUS_CRC_PASS_MASK)!=EFUSE_STATUS_CRC_PASS_MASK){
       return FMSH_FAILURE;
    }
    
    return FMSH_SUCCESS;
}

/****************************************************************************/
/**
 *
 * get user key
 *
 * @param    bIndex the index of key
 *
 * @return   key value
 *
 * @note     None.
 *
 ****************************************************************************/
uint32_t FEfusePs_getUserKey (uint8_t bIndex)
{
    uint32_t read_reg = 0;

    switch (bIndex)
    {
    case 0:
        read_reg = FMSH_ReadReg(FPS_CSU_BASEADDR, SAC_USER_REG_0_OFFSET);
        break;
    case 1:
        read_reg = FMSH_ReadReg(FPS_CSU_BASEADDR, SAC_USER_REG_1_OFFSET);
        break;
    case 2:
        read_reg = FMSH_ReadReg(FPS_CSU_BASEADDR, SAC_USER_REG_2_OFFSET);
        break;
    case 3:
        read_reg = FMSH_ReadReg(FPS_CSU_BASEADDR, SAC_USER_REG_3_OFFSET);
        break;
    case 4:
        read_reg = FMSH_ReadReg(FPS_CSU_BASEADDR, SAC_USER_REG_4_OFFSET);
        break;
    case 5:
        read_reg = FMSH_ReadReg(FPS_CSU_BASEADDR, SAC_USER_REG_5_OFFSET);
        break;
    case 6:
        read_reg = FMSH_ReadReg(FPS_CSU_BASEADDR, SAC_USER_REG_6_OFFSET);
        break;
    case 7:
        read_reg = FMSH_ReadReg(FPS_CSU_BASEADDR, SAC_USER_REG_7_OFFSET);
        break;
    default:
        break;
    }
    return read_reg;
}

/****************************************************************************/
/**
 *
 * get value of rsvd
 *
 * @param    bIndex the index of rsvd
 *
 * @return   rsvd value
 *
 * @note     None.
 *
 ****************************************************************************/
uint32_t FEfusePs_getRsvd (uint8_t bIndex)
{
    uint32_t read_reg = 0;

    switch (bIndex)
    {
    case 0:
        read_reg = FMSH_ReadReg(FPS_CSU_BASEADDR, SAC_RSVD_REG_0_OFFSET);
        break;
    case 1:
        read_reg = FMSH_ReadReg(FPS_CSU_BASEADDR, SAC_RSVD_REG_1_OFFSET);
        break;
    case 2:
        read_reg = FMSH_ReadReg(FPS_CSU_BASEADDR, SAC_RSVD_REG_2_OFFSET);
        break;
    case 3:
        read_reg = FMSH_ReadReg(FPS_CSU_BASEADDR, SAC_RSVD_REG_3_OFFSET);
        break;
    case 4:
        read_reg = FMSH_ReadReg(FPS_CSU_BASEADDR, SAC_RSVD_REG_4_OFFSET);
        break;
    case 5:
        read_reg = FMSH_ReadReg(FPS_CSU_BASEADDR, SAC_RSVD_REG_5_OFFSET);
        break;
    case 6:
        read_reg = FMSH_ReadReg(FPS_CSU_BASEADDR, SAC_RSVD_REG_6_OFFSET);
        break;
    case 7:
        read_reg = FMSH_ReadReg(FPS_CSU_BASEADDR, SAC_RSVD_REG_7_OFFSET);
        break;
    default:
        break;
    }
    return read_reg;
}

/****************************************************************************/
/**
 *
 * get value of ppk0hash
 *
 * @param    bIndex the index of ppk0hash
 *
 * @return   ppk0 hash value
 *
 * @note     None.
 *
 ****************************************************************************/

uint32_t FEfusePs_getPpk0hash (uint8_t bIndex)
{
    uint32_t read_reg = 0;

    switch (bIndex)
    {
    case 0:
        read_reg = FMSH_ReadReg(FPS_CSU_BASEADDR, SAC_PPK0_HASH_0_OFFSET);
        break;
    case 1:
        read_reg = FMSH_ReadReg(FPS_CSU_BASEADDR, SAC_PPK0_HASH_1_OFFSET);
        break;
    case 2:
        read_reg = FMSH_ReadReg(FPS_CSU_BASEADDR, SAC_PPK0_HASH_2_OFFSET);
        break;
    case 3:
        read_reg = FMSH_ReadReg(FPS_CSU_BASEADDR, SAC_PPK0_HASH_3_OFFSET);
        break;
    case 4:
        read_reg = FMSH_ReadReg(FPS_CSU_BASEADDR, SAC_PPK0_HASH_4_OFFSET);
        break;
    case 5:
        read_reg = FMSH_ReadReg(FPS_CSU_BASEADDR, SAC_PPK0_HASH_5_OFFSET);
        break;
    case 6:
        read_reg = FMSH_ReadReg(FPS_CSU_BASEADDR, SAC_PPK0_HASH_6_OFFSET);
        break;
    case 7:
        read_reg = FMSH_ReadReg(FPS_CSU_BASEADDR, SAC_PPK0_HASH_7_OFFSET);
        break;
    case 8:
        read_reg = FMSH_ReadReg(FPS_CSU_BASEADDR, SAC_PPK0_HASH_8_OFFSET);
        break;
    case 9:
        read_reg = FMSH_ReadReg(FPS_CSU_BASEADDR, SAC_PPK0_HASH_9_OFFSET);
        break;
    case 10:
        read_reg = FMSH_ReadReg(FPS_CSU_BASEADDR, SAC_PPK0_HASH_10_OFFSET);
        break;
    case 11:
        read_reg = FMSH_ReadReg(FPS_CSU_BASEADDR, SAC_PPK0_HASH_11_OFFSET);
        break;
    default:
        break;
    }
    return read_reg;
}

/****************************************************************************/
/**
 *
 * get value of ppk1 hash
 *
 * @param    bIndex the index of ppk1 hash
 *
 * @return   ppk1 hash value
 *
 * @note     None.
 *
 ****************************************************************************/
uint32_t FEfusePs_getPpk1hash (uint8_t bIndex)
{
    uint32_t read_reg = 0;

    switch (bIndex)
    {
    case 0:
        read_reg = FMSH_ReadReg(FPS_CSU_BASEADDR, SAC_PPK1_HASH_0_OFFSET);
        break;
    case 1:
        read_reg = FMSH_ReadReg(FPS_CSU_BASEADDR, SAC_PPK1_HASH_1_OFFSET);
        break;
    case 2:
        read_reg = FMSH_ReadReg(FPS_CSU_BASEADDR, SAC_PPK1_HASH_2_OFFSET);
        break;
    case 3:
        read_reg = FMSH_ReadReg(FPS_CSU_BASEADDR, SAC_PPK1_HASH_3_OFFSET);
        break;
    case 4:
        read_reg = FMSH_ReadReg(FPS_CSU_BASEADDR, SAC_PPK1_HASH_4_OFFSET);
        break;
    case 5:
        read_reg = FMSH_ReadReg(FPS_CSU_BASEADDR, SAC_PPK1_HASH_5_OFFSET);
        break;
    case 6:
        read_reg = FMSH_ReadReg(FPS_CSU_BASEADDR, SAC_PPK1_HASH_6_OFFSET);
        break;
    case 7:
        read_reg = FMSH_ReadReg(FPS_CSU_BASEADDR, SAC_PPK1_HASH_7_OFFSET);
        break;
    case 8:
        read_reg = FMSH_ReadReg(FPS_CSU_BASEADDR, SAC_PPK1_HASH_8_OFFSET);
        break;
    case 9:
        read_reg = FMSH_ReadReg(FPS_CSU_BASEADDR, SAC_PPK1_HASH_9_OFFSET);
        break;
    case 10:
        read_reg = FMSH_ReadReg(FPS_CSU_BASEADDR, SAC_PPK1_HASH_10_OFFSET);
        break;
    case 11:
        read_reg = FMSH_ReadReg(FPS_CSU_BASEADDR, SAC_PPK1_HASH_11_OFFSET);
        break;
    default:
        break;
    }
    return read_reg;
}

/****************************************************************************/
/**
 *
 * get value of golden
 *
 * @param    bIndex the index of golden
 *
 * @return   golden value
 *
 * @note     None.
 *
 ****************************************************************************/
uint32_t FEfusePs_getGolden (uint8_t bIndex)
{
    uint32_t read_reg = 0;

    switch (bIndex)
    {
    case 0:
        read_reg = FMSH_ReadReg(FPS_CSU_BASEADDR, SAC_SPUROM_HASH_0_OFFSET);
        break;
    case 1:
        read_reg = FMSH_ReadReg(FPS_CSU_BASEADDR, SAC_SPUROM_HASH_1_OFFSET);
        break;
    case 2:
        read_reg = FMSH_ReadReg(FPS_CSU_BASEADDR, SAC_SPUROM_HASH_2_OFFSET);
        break;
    case 3:
        read_reg = FMSH_ReadReg(FPS_CSU_BASEADDR, SAC_SPUROM_HASH_3_OFFSET);
        break;
    case 4:
        read_reg = FMSH_ReadReg(FPS_CSU_BASEADDR, SAC_SPUROM_HASH_4_OFFSET);
        break;
    case 5:
        read_reg = FMSH_ReadReg(FPS_CSU_BASEADDR, SAC_SPUROM_HASH_5_OFFSET);
        break;
    case 6:
        read_reg = FMSH_ReadReg(FPS_CSU_BASEADDR, SAC_SPUROM_HASH_6_OFFSET);
        break;
    case 7:
        read_reg = FMSH_ReadReg(FPS_CSU_BASEADDR, SAC_SPUROM_HASH_7_OFFSET);
        break;
    case 8:
        read_reg = FMSH_ReadReg(FPS_CSU_BASEADDR, SAC_SPUROM_HASH_8_OFFSET);
        break;
    case 9:
        read_reg = FMSH_ReadReg(FPS_CSU_BASEADDR, SAC_SPUROM_HASH_9_OFFSET);
        break;
    case 10:
        read_reg = FMSH_ReadReg(FPS_CSU_BASEADDR, SAC_SPUROM_HASH_10_OFFSET);
        break;
    case 11:
        read_reg = FMSH_ReadReg(FPS_CSU_BASEADDR, SAC_SPUROM_HASH_11_OFFSET);
        break;
    default:
        break;
    }
    return read_reg;
}



/*****************************************************************************
*
* @description
*  This functions is used to adjust the tPgm time.
*
* @param   
*  tPGM is the count of CSU clock.
*
* @return  
*  -FMSH_SUCCESS  -- if successful
*  -FMSH_FAILURE  -- if failed
*
* @note    
*  tPGM:12us
*
* SOURCE
*
****************************************************************************/
uint32_t FEfusePs_adjustTPGM (uint32_t tPGM)
{
    uint32_t read_reg = 0;
    FEfusePs_Portmap_T
        *pPortmap = (FEfusePs_Portmap_T *)(FPS_CSU_BASEADDR +
                                           SAC_EFUSE_CTRL_OFFSET);

    EFUSE_OUTP(tPGM, pPortmap->tpgm);
    read_reg = EFUSE_INP(pPortmap->tpgm);
    if (read_reg != tPGM)
    {
        return FMSH_FAILURE;
    }

    return FMSH_SUCCESS;
}

/*****************************************************************************
*
* @description
*  This functions is used to adjust the TRD time.
*
* @param   
*  tRD is the count of ahb clock.
*
* @return  
*  -FMSH_SUCCESS  -- if successful
*  -FMSH_FAILURE  -- if failed
*
* @note    
*  tRD:100ns-200ns
*
* SOURCE
*
****************************************************************************/
uint32_t FEfusePs_adjustTRD (uint32_t tRD)
{
    uint32_t read_reg = 0;
    FEfusePs_Portmap_T
        *pPortmap = (FEfusePs_Portmap_T *)(FPS_CSU_BASEADDR +
                                           SAC_EFUSE_CTRL_OFFSET);

    EFUSE_OUTP(tRD, pPortmap->trd);
    read_reg = EFUSE_INP(pPortmap->trd);
    if (read_reg != tRD)
    {
        return FMSH_FAILURE;
    }

    return FMSH_SUCCESS;
}

/*****************************************************************************
*
* @description
*  This functions is used to adjust the PS to CS time.
*
* @param   
*  tPsCs is the count of ahb clock.
*
* @return  
*  -FMSH_SUCCESS  -- if successful
*  -FMSH_FAILURE  -- if failed
*
* @note    
*  tPsCs:50-80ns
*
* SOURCE
*
****************************************************************************/
uint32_t FEfusePs_adjustTPSCS (uint32_t tPsCs)
{
    uint32_t read_reg = 0;
    FEfusePs_Portmap_T
        *pPortmap = (FEfusePs_Portmap_T *)(FPS_CSU_BASEADDR +
                                           SAC_EFUSE_CTRL_OFFSET);

    EFUSE_OUTP(tPsCs, pPortmap->tsu_h_ps_cs);
    read_reg = EFUSE_INP(pPortmap->tsu_h_ps_cs);
    if (read_reg != tPsCs)
    {
        return FMSH_FAILURE;
    }

    return FMSH_SUCCESS;
}

/*****************************************************************************
*
* @description
*  This functions is used to adjust the CS to Strobe time.
*
* @param   
*  tCsStrobe is the count of ahb clock.
*
* @return  
*  -FMSH_SUCCESS  -- if successful
*  -FMSH_FAILURE  -- if failed
*
* @note    
*  tCsStrobe: 5-8ns
*
* SOURCE
*
****************************************************************************/
uint32_t FEfusePs_adjustTCSStrobe (uint32_t tCsStrobe)
{
    uint32_t read_reg = 0;
    FEfusePs_Portmap_T
        *pPortmap = (FEfusePs_Portmap_T *)(FPS_CSU_BASEADDR +
                                           SAC_EFUSE_CTRL_OFFSET);

    EFUSE_OUTP(tCsStrobe, pPortmap->tsu_h_cs);
    read_reg = EFUSE_INP(pPortmap->tsu_h_cs);
    if (read_reg != tCsStrobe)
    {
        return FMSH_FAILURE;
    }

    return FMSH_SUCCESS;
}

