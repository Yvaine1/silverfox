#ifndef FMSH_EFUSE_H
#define FMSH_EFUSE_H

#include "fmsh_common.h"
#include "stdbool.h"

#ifdef __cplusplus
extern "C"
{  // allow C++ to use these headers
#endif

#define EFUSE_INP(p)                (uint32_t) * ((uint32_t *)(&p))
#define EFUSE_OUTP(v, p)            *((volatile uint32_t *)(&p)) = (v)

//EFUSE CFG
#define EFUSE_CFG_0_PGM_MASK (0x00000011)
#define EFUSE_CFG_1_PGM_MASK (0x00000021)
//EFUSE STATUS 
#define EFUSE_STATUS_PGM_DONE_MASK    (0x00000001U) 
#define EFUSE_STATUS_RD_DONE_MASK     (0x00000002U)    
#define EFUSE_STATUS_CACHE_DONE_MASK  (0x00000004U) 
#define EFUSE_STATUS_PGM_LOCK_MASK    (0x00000008U)
#define EFUSE_STATUS_CRC_DONE_MASK    (0x00000010U)
#define EFUSE_STATUS_CRC_PASS_MASK    (0x00000020U)
  
#define SAC_MISC_USER_CTRL_OFFSET   (0x250)
#define SAC_RSA_EN_OFFSET           (0x254)
#define SAC_SPK_ID_OFFSET           (0x258)
#define SAC_USER_REG_0_OFFSET       (0x25c)
#define SAC_USER_REG_1_OFFSET       (0x260)
#define SAC_USER_REG_2_OFFSET       (0x264)
#define SAC_USER_REG_3_OFFSET       (0x268)
#define SAC_USER_REG_4_OFFSET       (0x26c)
#define SAC_USER_REG_5_OFFSET       (0x270)
#define SAC_USER_REG_6_OFFSET       (0x274)
#define SAC_USER_REG_7_OFFSET       (0x278)
#define SAC_RSVD_REG_0_OFFSET       (0x27c)
#define SAC_RSVD_REG_1_OFFSET       (0x280)
#define SAC_RSVD_REG_2_OFFSET       (0x284)
#define SAC_RSVD_REG_3_OFFSET       (0x288)
#define SAC_RSVD_REG_4_OFFSET       (0x28c)
#define SAC_RSVD_REG_5_OFFSET       (0x290)
#define SAC_RSVD_REG_6_OFFSET       (0x294)
#define SAC_RSVD_REG_7_OFFSET       (0x298)

#define SAC_DNA_0_OFFSET            (0x2fc)
#define SAC_DNA_1_OFFSET            (0x300)
#define SAC_DNA_2_OFFSET            (0x304)
#define SAC_EX_IDCODE               (0x308)
#define SAC_PPK0_KEY_WR_LOCK_OFFSET (0x30C)
#define SAC_PPK1_KEY_WR_LOCK_OFFSET (0x310)
#define SAC_SPU_ROM_WR_LOCK_OFFSET  (0x314)
#define SAC_A7_ROM_CRC_OFFSET       (0x318)
#define SAC_EFUSE_ROW_71_OFFSET     (0x31C)
#define SAC_EFUSE_ROW_72_OFFSET     (0x330)
#define SAC_EFUSE_ROW_73_OFFSET     (0x320)
#define SAC_EFUSE_ROW_74_OFFSET     (0x324)
#define SAC_EFUSE_ROW_75_OFFSET     (0x328)
#define SAC_EFUSE_ROW_76_OFFSET     (0x32C)
#define SAC_EFUSE_ROW_77_OFFSET     (0x334)
#define SAC_EFUSE_ROW_78_OFFSET     (0x338)
#define SAC_EFUSE_ROW_79_OFFSET     (0x33C)
#define SAC_EFUSE_ROW_80_OFFSET     (0x340)
#define SAC_EFUSE_ROW_81_OFFSET     (0x344)
#define SAC_EFUSE_ROW_82_OFFSET     (0x348)
#define SAC_EFUSE_ROW_83_OFFSET     (0x34C)
#define SAC_EFUSE_ROW_84_OFFSET     (0x350)
#define SAC_EFUSE_ROW_85_OFFSET     (0x354)
#define SAC_EFUSE_ROW_86_OFFSET     (0x358)
#define SAC_EFUSE_ROW_87_OFFSET     (0x35C)
#define SAC_EFUSE_ROW_92_OFFSET     (0x370)
#define SAC_EFUSE_ROW_113_OFFSET    (0x3C4)
#define SAC_EFUSE_ROW_114_OFFSET    (0x3C8)
#define SAC_EFUSE_ROW_115_OFFSET    (0x3CC)
#define SAC_EFUSE_ROW_116_OFFSET    (0x3D0)
#define SAC_EFUSE_ROW_117_OFFSET    (0x3D4)
#define SAC_EFUSE_ROW_118_OFFSET    (0x3D8)
#define SAC_EFUSE_ROW_119_OFFSET    (0x3DC)
#define SAC_EFUSE_ROW_120_OFFSET    (0x3E0)
#define SAC_EFUSE_ROW_121_OFFSET    (0x3E4)
#define SAC_EFUSE_ROW_122_OFFSET    (0x3E8)
#define SAC_EFUSE_ROW_123_OFFSET    (0x3EC)
#define SAC_EFUSE_ROW_124_OFFSET    (0x3F0)
#define SAC_EFUSE_ROW_125_OFFSET    (0x3F4)
#define SAC_EFUSE_ROW_126_OFFSET    (0x3F8)
#define SAC_EFUSE_ROW_127_OFFSET    (0x3FC)

#define SAC_PPK0_HASH_0_OFFSET      (0x1000)
#define SAC_PPK0_HASH_1_OFFSET      (0x1004)
#define SAC_PPK0_HASH_2_OFFSET      (0x1008)
#define SAC_PPK0_HASH_3_OFFSET      (0x100C)
#define SAC_PPK0_HASH_4_OFFSET      (0x1010)
#define SAC_PPK0_HASH_5_OFFSET      (0x1014)
#define SAC_PPK0_HASH_6_OFFSET      (0x1018)
#define SAC_PPK0_HASH_7_OFFSET      (0x101C)
#define SAC_PPK0_HASH_8_OFFSET      (0x1020)
#define SAC_PPK0_HASH_9_OFFSET      (0x1024)
#define SAC_PPK0_HASH_10_OFFSET     (0x1028)
#define SAC_PPK0_HASH_11_OFFSET     (0x102C)
#define SAC_PPK1_HASH_0_OFFSET      (0x1030)
#define SAC_PPK1_HASH_1_OFFSET      (0x1034)
#define SAC_PPK1_HASH_2_OFFSET      (0x1038)
#define SAC_PPK1_HASH_3_OFFSET      (0x103C)
#define SAC_PPK1_HASH_4_OFFSET      (0x1040)
#define SAC_PPK1_HASH_5_OFFSET      (0x1044)
#define SAC_PPK1_HASH_6_OFFSET      (0x1048)
#define SAC_PPK1_HASH_7_OFFSET      (0x104C)
#define SAC_PPK1_HASH_8_OFFSET      (0x1050)
#define SAC_PPK1_HASH_9_OFFSET      (0x1054)
#define SAC_PPK1_HASH_10_OFFSET     (0x1058)
#define SAC_PPK1_HASH_11_OFFSET     (0x105C)
#define SAC_SPUROM_HASH_0_OFFSET    (0x1060)
#define SAC_SPUROM_HASH_1_OFFSET    (0x1064)
#define SAC_SPUROM_HASH_2_OFFSET    (0x1068)
#define SAC_SPUROM_HASH_3_OFFSET    (0x106C)
#define SAC_SPUROM_HASH_4_OFFSET    (0x1070)
#define SAC_SPUROM_HASH_5_OFFSET    (0x1074)
#define SAC_SPUROM_HASH_6_OFFSET    (0x1078)
#define SAC_SPUROM_HASH_7_OFFSET    (0x107C)
#define SAC_SPUROM_HASH_8_OFFSET    (0x1080)
#define SAC_SPUROM_HASH_9_OFFSET    (0x1084)
#define SAC_SPUROM_HASH_10_OFFSET   (0x1088)
#define SAC_SPUROM_HASH_11_OFFSET   (0x108C)
#define SAC_PMUROM_HASH_0_OFFSET    (0x374)
#define SAC_PMUROM_HASH_1_OFFSET    (0x378)
#define SAC_PMUROM_HASH_2_OFFSET    (0x37C)
#define SAC_PMUROM_HASH_3_OFFSET    (0x380)
#define SAC_PMUROM_HASH_4_OFFSET    (0x384)
#define SAC_PMUROM_HASH_5_OFFSET    (0x388)
#define SAC_PMUROM_HASH_6_OFFSET    (0x38C)
#define SAC_PMUROM_HASH_7_OFFSET    (0x390)
#define SAC_PMUROM_HASH_8_OFFSET    (0x394)
#define SAC_PMUROM_HASH_9_OFFSET    (0x398)
#define SAC_PMUROM_HASH_10_OFFSET   (0x39C)
#define SAC_PMUROM_HASH_11_OFFSET   (0x3A0)

typedef struct FEfusePs_Portmap {
    volatile uint32_t cfg;          // control register          (0x00)
    volatile uint32_t status;       // status                    (0x04)
    volatile uint32_t pgm_addr;     //  address                  (0x08)
    volatile uint32_t rd_addr;      // EFUSE_RD_ADDR             (0x0c)
    volatile uint32_t rd_data;      // EFUSE_RD_DATA             (0x10)
    volatile uint32_t tpgm;         // EFUSE_TPGM_REG            (0x14)
    volatile uint32_t trd;          // EFUSE_TRD_REG             (0x18)
    volatile uint32_t tsu_h_ps;     // EFUSE_TSU_H_PS            (0x1c)
    volatile uint32_t tsu_h_ps_cs;  // EFUSE_TSU_H_PS_CS         (0x20)
    volatile uint32_t tsu_h_cs;     // EFUSE_TSU_H_CS            (0x24)
    volatile uint32_t cache_load;   // FEfusePs_cacheLoad        (0x28)
    volatile uint32_t id_lock;      // SPK_ID_LOCK               (0x2c)
    volatile uint32_t aes_crc;      // EFUSE_AES_CRC             (0x30)
} FEfusePs_Portmap_T;

enum FMSH_efuse_chip {
    Efuse_chip_0 = 0,  //
    Efuse_chip_1 = 1   //
};

uint32_t FEfusePs_readData (uint32_t bRowAddr,uint32_t* lRdData);
uint32_t FEfusePs_programRowData(enum FMSH_efuse_chip cs,uint32_t bRowAddr,uint8_t bStartCol, 
                             uint8_t bEndCol,uint32_t lWrdata);
uint32_t FEfusePs_writeRow(uint32_t bRowAddr, uint32_t lWrdata);
uint32_t FEfusePs_cacheLoad(void);
uint32_t FEfusePs_crcCheck (uint32_t crcValue);
uint32_t FEfusePs_getUserKey(uint8_t bIndex);
uint32_t FEfusePs_getRsvd(uint8_t bIndex);
uint32_t FEfusePs_getPpk0hash(uint8_t bIndex);
uint32_t FEfusePs_getPpk1hash(uint8_t bIndex);
uint32_t FEfusePs_getGolden(uint8_t bIndex);

uint32_t FEfusePs_adjustTPGM(uint32_t tPGM);
uint32_t FEfusePs_adjustTRD(uint32_t tRD);
uint32_t FEfusePs_adjustTPSCS(uint32_t tPsCs);
uint32_t FEfusePs_adjustTCSStrobe(uint32_t tCsStrobe);


#ifdef __cplusplus
}
#endif

#endif
