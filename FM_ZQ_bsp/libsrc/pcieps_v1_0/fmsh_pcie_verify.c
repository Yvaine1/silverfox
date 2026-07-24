#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "verification_config.h"

#include "fmsh_common.h"
#include "fmsh_gic.h"
#include "fmsh_pcie_verify.h"

#define TEST_ADDR   0x400

#ifdef CORTEX_A53

#include "fmsh_apm_verify.h"
#include "fmsh_mpu_verify.h"
#include "fmsh_ppu_verify.h"

static u32 s_test_addr;

int Pcie_InboundForApmPmu() {
    /* bar0 -4k 64-bit   bar2 -16M 64 bit   bar4 -64k 64 bit*/ 
    FMSH_WriteReg(PCIE_LOCAL_MGMT_REG_BASE, 0x240, 0x05f105e9);
    FMSH_WriteReg(PCIE_LOCAL_MGMT_REG_BASE, 0x244, 0x050505e9);
    
    /* AXI address */
    FMSH_WriteReg(PCIE_AXI_CFG_REG_BASE, 0x840, 0xffff0000);    //OCM
    FMSH_WriteReg(PCIE_AXI_CFG_REG_BASE, 0x850, 0x1600000);     //DDR
    FMSH_WriteReg(PCIE_AXI_CFG_REG_BASE, 0x860, 0xfd480000);    //SLCR
    
    /* BAR address */
    FMSH_WriteReg(PCIE_CFG_SPACE_REG_BASE, 0x10, PCIE_XILINE_BROAD_RP);
    FMSH_WriteReg(PCIE_CFG_SPACE_REG_BASE, 0x18, PCIE_XILINE_BROAD_RP + 0x1000000);
    FMSH_WriteReg(PCIE_CFG_SPACE_REG_BASE, 0x20, PCIE_XILINE_BROAD_RP + 0x2000000);

	printf("verfiy APM & PMU, Link patern is Xiline\n");
    printf("pci 0x60000000  -- OCM  axi 0xffff0000\n");
    printf("pci 0x61000000  -- DDR  axi 0x01000000\n");
    printf("pci 0x62000000  -- SLCR axi 0xfd480000\n");
}

/* read & write */
static void link_partner_access() 
{
    fmsh_print_info("target addr : 0x%x!\n", s_test_addr);
        
    //write
    fmsh_print_info("link partner start read/write!\n");
    
    //read
    fmsh_print_info("end\n");
}


static void link_partner_write() 
{
    int ret;
    int i;
    u8 *srcPtr;
        
    srcPtr = (u8*)s_test_addr;
    
    fmsh_print_info("target addr : 0x%x!\n", s_test_addr);
 
    fmsh_print_info("\n");
}

static void link_partner_read() 
{
    int ret;
    int i;
    u8 *srcPtr;
        
    srcPtr = (u8*)s_test_addr;
    
    fmsh_print_info("target addr : 0x%x!\n", s_test_addr);
 
    fmsh_print_info("\n");
}

__attribute__((unused)) static int xapm_test()
{
    int ret;
    Pcie_InboundForApmPmu();
    fmsh_print_info("===========================\r\n");
    fmsh_print_info("INFO(sdmmc xamp test):\r\n");

    fmsh_print_info("initialize xamp\r\n");
    FApmPsu_VerifyConfig_t config;
    ret = fmsh_apm_init();
    if(ret)
        return ret;
    
    for (int i = 0; i < 10; i++) {
        config.metric_set[i] = metric_set[i];
    }
    
    //apm
    fmsh_print_info("start transfer...\r\n");
    fmsh_print_info("-------------------------------\r\n");
    fmsh_print_info("ocm slot0, write & read\r\n");
    s_test_addr = 0xffff1000;
    fmsh_apm_master_test(&OcmApm, &config, 0, link_partner_access);
    fmsh_print_info("-------------------------------\r\n");
    fmsh_print_info("cci slot0, write & read\r\n");  
    fmsh_apm_master_test(&CciApm, &config, 0, link_partner_access);
    
    
    
    fmsh_print_info("-------------------------------\r\n");
    fmsh_print_info("ddr slot1, write & read\r\n");
    s_test_addr = 0x10000000;
    fmsh_apm_master_test(&DdrApm, &config, 1, link_partner_access);
    fmsh_print_info("-------------------------------\r\n");
    fmsh_print_info("ddr slot2, write & read\r\n");
    fmsh_apm_master_test(&DdrApm, &config, 2, link_partner_access);
    
    
    return FMSH_SUCCESS;
}


//attrib
#define SYSMPU_NSCHECK_TYPE    0x10
#define SYSMPU_REGION_NS       0x8
#define SYSMPU_WR_ALLOWED      0x4
#define SYSMPU_RD_ALLOWED      0x2
#define SYSMPU_ENABLE          0x1

static void xmpu_config_region(u32 mpubase, u32 region, 
                       u32 start, u32 end, u32 mid, u32 mid_mask,
                       u32 attrib) 
{        
    FMSH_WriteReg(mpubase, 0x100+(region*16), start >> 12);
    FMSH_WriteReg(mpubase, 0x104+(region*16), end >> 12);
    FMSH_WriteReg(mpubase, 0x108+(region*16), (mid_mask << 16) | mid);
    FMSH_WriteReg(mpubase, 0x10c+(region*16), attrib);          
}

static int xmpu_result(u32 mpubase)
{
    int ret;
    u32 value, value2;
    
    value = FMSH_ReadReg(mpubase, 0x10);
    FMSH_WriteReg(mpubase, 0x10, 0xF);
    if(value & 0x1) {
        fmsh_print_info("INV-APB occured\r\n");
    }
    if(value & 0x2) {
        fmsh_print_info("RdPermVIO occured\r\n");
    }
    if(value & 0x4) {
        fmsh_print_info("WrPermVIO occured\r\n");
    }
    if(value & 0x8) {
        fmsh_print_info("SecurityVIO occured\r\n");
    }
    if(value) {
        value2 = FMSH_ReadReg(mpubase, 0x4); 
        fmsh_print_info("ERROR Address: 0x%x\r\n", value2 << 12);
        value2 = FMSH_ReadReg(mpubase, 0x8); 
        fmsh_print_info("ERROR Master ID: 0x%x\r\n", value2);
    }       
    
    return FMSH_SUCCESS;
}

static void xmpu_config(u32 mid, u32 mid_mask) 
{       
    u32 mpubase;
    u32 start, end;
    u32 attrib;
            
    ///config OCM-MPU
    mpubase = 0xFFA70000;
    FMSH_WriteReg(mpubase, MPU_CTRL, 0x3);
    FMSH_WriteReg(mpubase, MPU_ISR, 0xF);
    FMSH_WriteReg(mpubase, MPU_IMR, 0xE);
    FMSH_WriteReg(mpubase, 0xc, 0x100000);

    //ocm secure area read-only
    start = 0xffff1000;
    end = 0xffff1fff;
    attrib = SYSMPU_NSCHECK_TYPE | SYSMPU_RD_ALLOWED | SYSMPU_ENABLE;
    xmpu_config_region(mpubase, 12, start, end, mid, mid_mask, attrib);
    
    //ocm secure area write-only
    start = 0xffff2000;
    end = 0xffff2fff;
    attrib = SYSMPU_NSCHECK_TYPE | SYSMPU_WR_ALLOWED  | SYSMPU_ENABLE;
    xmpu_config_region(mpubase, 13, start, end, mid, mid_mask, attrib);
    
    //ocm non-secure area read-only
    start = 0xffff3000;
    end = 0xffff3fff;
    attrib = SYSMPU_NSCHECK_TYPE | SYSMPU_REGION_NS | SYSMPU_RD_ALLOWED | SYSMPU_ENABLE;
    xmpu_config_region(mpubase, 14, start, end, mid, mid_mask, attrib);
        
    //ocm non-secure area write-only
    start = 0xffff4000;
    end = 0xffff4fff;
    attrib = SYSMPU_NSCHECK_TYPE | SYSMPU_REGION_NS | SYSMPU_WR_ALLOWED | SYSMPU_ENABLE;
    xmpu_config_region(mpubase, 15, start, end, mid, mid_mask, attrib);
    
    ///config DDR-MPU1
    mpubase = 0xFD010000;
    FMSH_WriteReg(mpubase, MPU_CTRL, 0x3);
    FMSH_WriteReg(mpubase, MPU_ISR, 0xF);
    FMSH_WriteReg(mpubase, MPU_IMR, 0xE);
    //FMSH_WriteReg(mpubase, MPU_IEN, 0xE);
    
    //ddr secure area read-only
    start = 0x1400000;
    end = 0x14fffff;
    attrib = SYSMPU_NSCHECK_TYPE | SYSMPU_RD_ALLOWED | SYSMPU_ENABLE;
    xmpu_config_region(mpubase, 12, start, end, mid, mid_mask, attrib);
    
    //ddr secure area write-only
    start = 0x1500000;
    end = 0x15fffff;
    attrib = SYSMPU_NSCHECK_TYPE | SYSMPU_WR_ALLOWED | SYSMPU_ENABLE;
    xmpu_config_region(mpubase, 13, start, end, mid, mid_mask, attrib);
    
    //ddr non-secure area read-only
    start = 0x1600000;
    end = 0x16fffff;
    attrib = SYSMPU_NSCHECK_TYPE | SYSMPU_REGION_NS | SYSMPU_RD_ALLOWED | SYSMPU_ENABLE;
    xmpu_config_region(mpubase, 14, start, end, mid, mid_mask, attrib);
     
    //ddr non-secure area write-only
    start = 0x1700000;
    end = 0x17fffff;
    attrib = SYSMPU_NSCHECK_TYPE | SYSMPU_REGION_NS | SYSMPU_WR_ALLOWED | SYSMPU_ENABLE;
    xmpu_config_region(mpubase, 15, start, end, mid, mid_mask, attrib);
    
    ///config DDR-MPU2
    mpubase = 0xFD020000 ;
    FMSH_WriteReg(mpubase, MPU_CTRL, 0x3);
    FMSH_WriteReg(mpubase, MPU_ISR, 0xF);
    FMSH_WriteReg(mpubase, MPU_IMR, 0xE);
    //FMSH_WriteReg(mpubase, MPU_IEN, 0xE);
    
    //ddr secure area read-only
    start = 0x1400000;
    end = 0x14fffff;
    attrib = SYSMPU_NSCHECK_TYPE | SYSMPU_RD_ALLOWED | SYSMPU_ENABLE;
    xmpu_config_region(mpubase, 12, start, end, mid, mid_mask, attrib);
    
    //ddr secure area write-only
    start = 0x1500000;
    end = 0x15fffff;
    attrib = SYSMPU_NSCHECK_TYPE | SYSMPU_WR_ALLOWED | SYSMPU_ENABLE;
    xmpu_config_region(mpubase, 13, start, end, mid, mid_mask, attrib);
    
    //ddr non-secure area read-only
    start = 0x1600000;
    end = 0x16fffff;
    attrib = SYSMPU_NSCHECK_TYPE | SYSMPU_REGION_NS | SYSMPU_RD_ALLOWED | SYSMPU_ENABLE;
    xmpu_config_region(mpubase, 14, start, end, mid, mid_mask, attrib);
     
    //ddr non-secure area write-only
    start = 0x1700000;
    end = 0x17fffff;
    attrib = SYSMPU_NSCHECK_TYPE | SYSMPU_REGION_NS | SYSMPU_WR_ALLOWED | SYSMPU_ENABLE;
    xmpu_config_region(mpubase, 15, start, end, mid, mid_mask, attrib);

    //config FPD-MPU
    mpubase = 0xFD5D0000;
    FMSH_WriteReg(mpubase, MPU_CTRL, 0x3);
    FMSH_WriteReg(mpubase, MPU_ISR, 0xF);
    FMSH_WriteReg(mpubase, MPU_IMR, 0xE);
    //FMSH_WriteReg(mpubase, MPU_IEN, 0xE);   

    //not allowed read or write
    start = 0xfd480000;
    end = 0xfd483fff;
    attrib = SYSMPU_NSCHECK_TYPE | SYSMPU_ENABLE;
    xmpu_config_region(mpubase, 0, start, end, mid, mid_mask, attrib);

    //write-only
    start = 0xfd484000;
    end = 0xfd48ffff;
    attrib = SYSMPU_NSCHECK_TYPE | SYSMPU_WR_ALLOWED | SYSMPU_ENABLE;
    xmpu_config_region(mpubase, 1, start, end, mid, mid_mask, attrib);
}

/* OCM access can be reject; DDR can`t because of fake ddr use in haps */
__attribute__((unused)) static int xmpu_test()
{
    int ret;
    int mid = 0xd0;

    fmsh_print_info("===========================\r\n");
    fmsh_print_info("INFO(sdmmc xmpu test):\r\n");

    u32 mpubase;
    
    xmpu_config(mid, 0x3FF);
    
    Pcie_InboundForApmPmu();

    /**************************************************************************/
    //enablesd secure access
    FMSH_WriteReg(0xFD690000, FPD_SLCR_SECURE_PCIE, 0x80008);
    mpubase = 0xFFA70000;
    fmsh_print_info("----------------------------------------------------\r\n");
    fmsh_print_info("OCM-MPU: sd secure read to non-secure read-only area\r\n");
    FMSH_WriteReg(mpubase, MPU_ISR, 0xF);
    s_test_addr = 0xffff3000;
    link_partner_read();
    xmpu_result(mpubase);
    fmsh_print_info("----------------------------------------------------\r\n");
    fmsh_print_info("OCM-MPU: sd secure write to non-secure write-only area\r\n");
    FMSH_WriteReg(mpubase, MPU_ISR, 0xF);
    s_test_addr = 0xffff4000;
    link_partner_write();
    xmpu_result(mpubase);
    fmsh_print_info("----------------------------------------------------\r\n");
    fmsh_print_info("OCM-MPU: sd secure write to secure read-only area\r\n");
    FMSH_WriteReg(mpubase, MPU_ISR, 0xF);
    s_test_addr = 0xffff1000;
    link_partner_write(); 
    xmpu_result(mpubase);
    fmsh_print_info("----------------------------------------------------\r\n");
    fmsh_print_info("OCM-MPU: sd secure read to secure write-only area\r\n");
    FMSH_WriteReg(mpubase, MPU_ISR, 0xF);
    s_test_addr = 0xffff2000;
    link_partner_read();   
    xmpu_result(mpubase);
    
    //enablesd non-secure access
    FMSH_WriteReg(0xFD690000, FPD_SLCR_SECURE_PCIE, 0xa000a);
    mpubase = 0xFFA70000;
    fmsh_print_info("----------------------------------------------------\r\n");
    fmsh_print_info("OCM-MPU: sd non-secure read to secure read-only area\r\n");
    FMSH_WriteReg(mpubase, MPU_ISR, 0xF);
    s_test_addr = 0xffff1000;
    link_partner_read();
    xmpu_result(mpubase);
    fmsh_print_info("----------------------------------------------------\r\n");
    fmsh_print_info("OCM-MPU: sd non-secure write to secure write-only area\r\n");
    FMSH_WriteReg(mpubase, MPU_ISR, 0xF);
    s_test_addr = 0xffff2000;
    link_partner_write();
    xmpu_result(mpubase);
    fmsh_print_info("----------------------------------------------------\r\n");
    fmsh_print_info("OCM-MPU: sd non-secure write to non-secure read-only area\r\n");
    FMSH_WriteReg(mpubase, MPU_ISR, 0xF);
    s_test_addr = 0xffff3000;
    link_partner_write();   
    xmpu_result(mpubase);
    fmsh_print_info("----------------------------------------------------\r\n");
    fmsh_print_info("OCM-MPU: sd non-secure read to non-secure write-only area\r\n");
    FMSH_WriteReg(mpubase, MPU_ISR, 0xF);
    s_test_addr = 0xffff4000;
    link_partner_read(); 
    xmpu_result(mpubase);
    FMSH_WriteReg(0xFD690000, FPD_SLCR_SECURE_PCIE, 0x0);

    /**************************************************************************/
    //enablesd secure access
    FMSH_WriteReg(0xFD690000, FPD_SLCR_SECURE_PCIE, 0x80008);
    mpubase = 0xFD010000;
    fmsh_print_info("----------------------------------------------------\r\n");
    fmsh_print_info("DDR1-MPU: sd secure read to non-secure read-only area\r\n");
    FMSH_WriteReg(mpubase, MPU_ISR, 0xF);
    s_test_addr = 0x1600000;
    link_partner_read();
    xmpu_result(mpubase);
    fmsh_print_info("----------------------------------------------------\r\n");
    fmsh_print_info("DDR1-MPU: sd secure write to non-secure write-only area\r\n");
    FMSH_WriteReg(mpubase, MPU_ISR, 0xF);
    s_test_addr = 0x1700000;
    link_partner_write();
    xmpu_result(mpubase);
    fmsh_print_info("----------------------------------------------------\r\n");
    fmsh_print_info("DDR1-MPU: sd secure write to secure read-only area\r\n");
    FMSH_WriteReg(mpubase, MPU_ISR, 0xF);
    s_test_addr = 0x1400000;
    link_partner_write(); 
    xmpu_result(mpubase);
    fmsh_print_info("----------------------------------------------------\r\n");
    fmsh_print_info("DDR1-MPU: sd secure read to secure write-only area\r\n");
    FMSH_WriteReg(mpubase, MPU_ISR, 0xF);
    s_test_addr = 0x1500000;
    link_partner_read();   
    xmpu_result(mpubase);
    
    //enablesd non-secure access
    FMSH_WriteReg(0xFD690000, FPD_SLCR_SECURE_PCIE, 0xa000a);
    mpubase = 0xFD010000;
    fmsh_print_info("----------------------------------------------------\r\n");
    fmsh_print_info("DDR1-MPU: sd non-secure read to secure read-only area\r\n");
    FMSH_WriteReg(mpubase, MPU_ISR, 0xF);
    s_test_addr = 0x1400000;
    link_partner_read();
    xmpu_result(mpubase);
    fmsh_print_info("----------------------------------------------------\r\n");
    fmsh_print_info("DDR1-MPU: sd non-secure write to secure write-only area\r\n");
    FMSH_WriteReg(mpubase, MPU_ISR, 0xF);
    s_test_addr = 0x1500000;
    link_partner_write();
     xmpu_result(mpubase);
    fmsh_print_info("----------------------------------------------------\r\n");
    fmsh_print_info("DDR1-MPU: sd non-secure write to non-secure read-only area\r\n");
    FMSH_WriteReg(mpubase, MPU_ISR, 0xF);
    s_test_addr = 0x1600000;
    link_partner_write();   
    xmpu_result(mpubase);
    fmsh_print_info("----------------------------------------------------\r\n");
    fmsh_print_info("DDR1-MPU: sd non-secure read to non-secure write-only area\r\n");
    FMSH_WriteReg(mpubase, MPU_ISR, 0xF);
    s_test_addr = 0x1700000;
    link_partner_read(); 
    xmpu_result(mpubase);    
    FMSH_WriteReg(0xFD690000, FPD_SLCR_SECURE_PCIE, 0x0);
    
    /**************************************************************************/
    //enablesd secure access
    FMSH_WriteReg(0xFD690000, FPD_SLCR_SECURE_PCIE, 0x80008);
    mpubase = 0xFD020000;
    fmsh_print_info("----------------------------------------------------\r\n");
    fmsh_print_info("DDR2-MPU: sd secure read to non-secure read-only area\r\n");
    FMSH_WriteReg(mpubase, MPU_ISR, 0xF);
    s_test_addr = 0x1600000;
    link_partner_read();
    xmpu_result(mpubase);
    fmsh_print_info("----------------------------------------------------\r\n");
    fmsh_print_info("DDR2-MPU: sd secure write to non-secure write-only area\r\n");
    FMSH_WriteReg(mpubase, MPU_ISR, 0xF);
    s_test_addr = 0x1700000;
    link_partner_write();
    xmpu_result(mpubase);
    fmsh_print_info("----------------------------------------------------\r\n");
    fmsh_print_info("DDR2-MPU: sd secure write to secure read-only area\r\n");
    FMSH_WriteReg(mpubase, MPU_ISR, 0xF);
    s_test_addr = 0x1400000;
    link_partner_write(); 
    xmpu_result(mpubase);
    fmsh_print_info("----------------------------------------------------\r\n");
    fmsh_print_info("DDR2-MPU: sd secure read to secure write-only area\r\n");
    FMSH_WriteReg(mpubase, MPU_ISR, 0xF);
    s_test_addr = 0x1500000;
    link_partner_read();   
    xmpu_result(mpubase);
    
    //enablesd non-secure access
    FMSH_WriteReg(0xFD690000, FPD_SLCR_SECURE_PCIE, 0xa000a);
    mpubase = 0xFD020000;
    fmsh_print_info("----------------------------------------------------\r\n");
    fmsh_print_info("DDR2-MPU: sd non-secure read to secure read-only area\r\n");
    FMSH_WriteReg(mpubase, MPU_ISR, 0xF);
    s_test_addr = 0x1400000;
    link_partner_read();
    xmpu_result(mpubase);
    fmsh_print_info("----------------------------------------------------\r\n");
    fmsh_print_info("DDR2-MPU: sd non-secure write to secure write-only area\r\n");
    FMSH_WriteReg(mpubase, MPU_ISR, 0xF);
    s_test_addr = 0x1500000;
    link_partner_write();
     xmpu_result(mpubase);
    fmsh_print_info("----------------------------------------------------\r\n");
    fmsh_print_info("DDR2-MPU: sd non-secure write to non-secure read-only area\r\n");
    FMSH_WriteReg(mpubase, MPU_ISR, 0xF);
    s_test_addr = 0x1600000;
    link_partner_write();   
    xmpu_result(mpubase);
    fmsh_print_info("----------------------------------------------------\r\n");
    fmsh_print_info("DDR2-MPU: sd non-secure read to non-secure write-only area\r\n");
    FMSH_WriteReg(mpubase, MPU_ISR, 0xF);
    s_test_addr = 0x1700000;
    link_partner_read(); 
    xmpu_result(mpubase);
    
    //enablesd secure access
    FMSH_WriteReg(0xFD690000, FPD_SLCR_SECURE_PCIE, 0x80008);
    mpubase = 0xFD5D0000;
    fmsh_print_info("----------------------------------------------------\r\n");
    fmsh_print_info("FPD-MPU: PCIe not allow access area\r\n");
    FMSH_WriteReg(mpubase, MPU_ISR, 0xF);
    s_test_addr = 0xfd480000;
    link_partner_write();   
    xmpu_result(mpubase);
    fmsh_print_info("----------------------------------------------------\r\n");
    fmsh_print_info("FPD-MPU: PCIe write-only area ??msi/msi-x??\r\n");
    FMSH_WriteReg(mpubase, MPU_ISR, 0xF);
    s_test_addr = 0xfd484000;
    link_partner_read(); 
    xmpu_result(mpubase);
    
    return FMSH_SUCCESS;
}

#endif //CORTEX_A53

u32 Pcie_MpsTest(struct fmsh_pcie *pcie, u8 type)
{
    u32 size = 0x1000;

    if (type)
        FSpcie_MaxPayLoadSizeChange(pcie, 1);

    //write, check rp memwr is 0x100
    FSpcie_DmaBulkInit(0, size);
    FSpcie_DmaBulkTrans(0, 1);
    delay_1ms();
    //read, check ep cpld is 0x100
    FSpcie_DmaBulkInit(0, size);
    FSpcie_DmaBulkTrans(0, 0);
    delay_1ms();

    return FMSH_SUCCESS;
}
u32 Pcie_RcbTest()
{
    u32 size;
    pcie_dma_bulk_axi_addr0 = 0x40000064;
    pcie_dma_bulk_axi_addr1 = 0x60000058;

    size = 0x114;
    FSpcie_DmaBulkInit(0, size);
    FSpcie_DmaBulkTrans(0, 0);

    //change RP rcb to 128 bytes
    delay_1ms();

    size = 0x1ac;
    FSpcie_DmaBulkInit(0, size);
    FSpcie_DmaBulkTrans(0, 0);
    return FMSH_SUCCESS;
}

u32 Pcie_MrrsTest()
{
    u32 i;

    for (i = 0; i < 6; i++) {
        FSpcie_MaxReadRequestSizeChange(i);
        FSpcie_DmaBulkInit(0, 0x1000);
        FSpcie_DmaBulkTrans(0, 0);
        delay_1ms();
    }
    return FMSH_SUCCESS;
}

u32 Pcie_NotFatalErrTest(struct fmsh_pcie *pcie)
{
    u32 value;
    u32 UncorrErrSts = 0x104;
    u32 PosinSts = BIT(12);

    Pcie_PoisonMemWriteSend();
    delay_1ms();
    value = FSpcie_CfgRead(pcie, 1, 0, 0, UncorrErrSts);
    if (value & PosinSts) {
        printf("EP receive Uncorrect Error\n");
        printf("%s success\n", __func__);
        return FMSH_SUCCESS;
    }

    printf("EP can not receive Uncorrect Error\n");
    printf("%s failed\n", __func__);
    return FMSH_FAILURE;
}

u32 Pcie_FatalErrTest(struct fmsh_pcie *pcie)
{
    u32 value;
    u32 UncorrErrSeve = 0x10c;
    u32 PosinSeve = BIT(12);

    value = FSpcie_CfgRead(pcie, 1, 0, 0, UncorrErrSeve);
    FSpcie_CfgWrite(pcie, 1, 0, 0, UncorrErrSeve, value | PosinSeve);
    Pcie_PoisonMemWriteSend();
    delay_1ms();
    
    if (value & PosinSeve) {
        printf("EP receive Fatal Error\n");
        printf("%s success\n", __func__);
        return FMSH_SUCCESS;
    }

    printf("EP can not receive Fatal Error\n");
    printf("%s failed\n", __func__);
    return FMSH_FAILURE;
}

u32 Pcie_CorrErrTest(struct fmsh_pcie *pcie)
{
    u32 value;
    u32 CorrErrSts = 0x110;
    u32 AdvisorySts = BIT(13);

    Pcie_PoisonMemWriteSend();
    delay_1ms();
    value = FSpcie_CfgRead(pcie, 1, 0, 0, CorrErrSts);
    if (value & AdvisorySts) {
        printf("EP receive Uncorrect Error\n");
        printf("%s success\n", __func__);
        return FMSH_SUCCESS;
    }

    printf("EP can not receive Uncorrect Error\n");
    printf("%s failed\n", __func__);
    return FMSH_FAILURE;
}

//EP INIT
void Pcie_CorrErrInit()
{
    u32 value;
    u32 mask = ~BIT(13);

    value = FMSH_ReadReg(PCIE_LOCAL_MGMT_REG_BASE, I_DEBUG_MUX_CONTROL_2_REG);
    FMSH_WriteReg(PCIE_LOCAL_MGMT_REG_BASE,
                  I_DEBUG_MUX_CONTROL_2_REG, value | BIT(10));
    
    value = FMSH_ReadReg(PCIE_CFG_SPACE_REG_BASE, 0x114);
    FMSH_WriteReg(PCIE_CFG_SPACE_REG_BASE, 0x114, value & mask);
}

u32 Pcie_VfMemTest()
{
    u32 vf;
    u32 value, flag = 0;
    u32 bar0, bar2, bar4;

    //PF0`s VF
    for (vf = 4; vf < 0x43; vf++) {
        FMSH_WriteReg(0x9000000000, (vf-4)*0x1000, 0xae860000 + vf);
        value = FMSH_ReadReg(0x9000000000, (vf-4)*0x1000);
        if (value != (0xae860000 + vf)) {
            flag = 1;
            printf("VF-%d bar0 mem error, write 0x%x, read 0x%x\n",
                                        vf, 0xae860000 + vf, value);
        }

        FMSH_WriteReg(0x9100000000, (vf-4)*0x1000, 0xae860000 + vf);
        value = FMSH_ReadReg(0x9100000000, (vf-4)*0x1000);
        if (value != (0xae860000 + vf)) {
            flag = 1;
            printf("VF-%d bar2 mem error, write 0x%x, read 0x%x\n",
                                        vf, 0xae860000 + vf, value);
        }

        FMSH_WriteReg(0x9200000000, (vf-4)*0x1000, 0xae860000 + vf);
        value = FMSH_ReadReg(0x9200000000, (vf-4)*0x1000);
        if (value != (0xae860000 + vf)) {
            flag = 1;
            printf("VF-%d bar4 mem error, write 0x%x, read 0x%x\n",
                                        vf, 0xae860000 + vf, value);
        }
    }

    //PF1`s VF
    for (vf = 0x43; vf < 0x82; vf++) {
        FMSH_WriteReg(0x9400000000, (vf-0x43)*0x1000, 0xae860000 + vf);
        value = FMSH_ReadReg(0x9400000000, (vf-0x43)*0x1000);
        if (value != (0xae860000 + vf)) {
            flag = 1;
            printf("VF-%d bar0 mem error, write 0x%x, read 0x%x\n",
                                        vf, 0xae860000 + vf, value);
        }

        FMSH_WriteReg(0x9500000000, (vf-0x43)*0x1000, 0xae860000 + vf);
        value = FMSH_ReadReg(0x9500000000, (vf-0x43)*0x1000);
        if (value != (0xae860000 + vf)) {
            flag = 1;
            printf("VF-%d bar2 mem error, write 0x%x, read 0x%x\n",
                                        vf, 0xae860000 + vf, value);
        }

        FMSH_WriteReg(0x9600000000, (vf-0x43)*0x1000, 0xae860000 + vf);
        value = FMSH_ReadReg(0x9600000000, (vf-0x43)*0x1000);
        if (value != (0xae860000 + vf)) {
            flag = 1;
            printf("VF-%d bar4 mem error, write 0x%x, read 0x%x\n",
                                        vf, 0xae860000 + vf, value);
        }
    }

    //PF2`s VF
    for (vf = 0x82; vf < 0xc1; vf++) {
        FMSH_WriteReg(0x9800000000, (vf-0x82)*0x1000, 0xae860000 + vf);
        value = FMSH_ReadReg(0x9800000000, (vf-0x82)*0x1000);
        if (value != (0xae860000 + vf)) {
            flag = 1;
            printf("VF-%d bar0 mem error, write 0x%x, read 0x%x\n",
                                        vf, 0xae860000 + vf, value);
        }

        FMSH_WriteReg(0x9900000000, (vf-0x82)*0x1000, 0xae860000 + vf);
        value = FMSH_ReadReg(0x9900000000, (vf-0x82)*0x1000);
        if (value != (0xae860000 + vf)) {
            flag = 1;
            printf("VF-%d bar2 mem error, write 0x%x, read 0x%x\n",
                                        vf, 0xae860000 + vf, value);
        }

        FMSH_WriteReg(0x9a00000000, (vf-0x82)*0x1000, 0xae860000 + vf);
        value = FMSH_ReadReg(0x9a00000000, (vf-0x82)*0x1000);
        if (value != (0xae860000 + vf)) {
            flag = 1;
            printf("VF-%d bar4 mem error, write 0x%x, read 0x%x\n",
                                        vf, 0xae860000 + vf, value);
        }
    }

    //PF3`s VF
    for (vf = 0xc1; vf < 0x100; vf++) {
        FMSH_WriteReg(0x9c00000000, (vf-0xc1)*0x1000, 0xae860000 + vf);
        value = FMSH_ReadReg(0x9c00000000, (vf-0xc1)*0x1000);
        if (value != (0xae860000 + vf)) {
            flag = 1;
            printf("VF-%d bar0 mem error, write 0x%x, read 0x%x\n",
                                        vf, 0xae860000 + vf, value);
        }

        FMSH_WriteReg(0x9d00000000, (vf-0xc1)*0x1000, 0xae860000 + vf);
        value = FMSH_ReadReg(0x9d00000000, (vf-0xc1)*0x1000);
        if (value != (0xae860000 + vf)) {
            flag = 1;
            printf("VF-%d bar2 mem error, write 0x%x, read 0x%x\n",
                                        vf, 0xae860000 + vf, value);
        }

        FMSH_WriteReg(0x9e00000000, (vf-0xc1)*0x1000, 0xae860000 + vf);
        value = FMSH_ReadReg(0x9e00000000, (vf-0xc1)*0x1000);
        if (value != (0xae860000 + vf)) {
            flag = 1;
            printf("VF-%d bar4 mem error, write 0x%x, read 0x%x\n",
                                        vf, 0xae860000 + vf, value);
        }
    }

    if (flag)
        return FMSH_FAILURE;

    printf("%s pass\n", __func__);
    return FMSH_SUCCESS;
}

u32 Pcie_VfCfgTest(struct fmsh_pcie *pcie)
{
    u32 pf, vf;
    u32 value, data = 0, flag = 0;
    u32 MsiCap = 0x90, MsiData = 0x9c;
    u16 bus = 1;

    for (vf = 4; vf < 256; vf++) {
        //enable msi
        FSpcie_AriCfgWrite(pcie, bus, vf, MsiCap, 0x1ddb005);

        FSpcie_AriCfgWrite(pcie, bus, vf, MsiData, data);
        value = FSpcie_AriCfgRead(pcie, bus, vf, MsiData);
        if (value != data) {
            flag = 1;
            printf("VF-%d error, write : 0x%x, read : 0x%x\n",
                    vf, data, value);
        }
        data++;
    }

    if (flag)
        return FMSH_FAILURE;

    printf("%s pass\n", __func__);
    return FMSH_SUCCESS;
}

u32 Pcie_FlrTest(struct fmsh_pcie *pcie)
{
    u32 pf, vf;
    u32 value, data = 0, flag = 0;
    u32 MsiCap = 0x90, MsiData = 0x9c;
    u32 DevCtrl = 0xc8;
    u32 mask = BIT(15);
    u16 bus = 1;

    for (vf = 0; vf < 256; vf++) {
        //enable msi
        FSpcie_AriCfgWrite(pcie, bus, vf, MsiCap, 0x1ddb005);

        FSpcie_AriCfgWrite(pcie, bus, vf, MsiData, data);
        value = FSpcie_AriCfgRead(pcie, bus, vf, MsiData);
        if (value != data) {
            flag = 1;
            printf("VF-%d error, write : 0x%x, read : 0x%x\n",
                    vf, data, value);
        }

        value = FSpcie_AriCfgRead(pcie, bus, vf, DevCtrl);
        FSpcie_AriCfgWrite(pcie, bus, vf, DevCtrl, value | mask);
        delay_1ms();
		value = FSpcie_AriCfgRead(pcie, bus, vf, MsiData);
        if (value != 0) {
            flag = 1;
            printf("Func-%d FLR error\n", vf);
        } else
            printf("Func-%d FLR success\n", vf);
        data++;
    }

    if (flag)
        return FMSH_FAILURE;

    printf("%s pass\n", __func__);
    return FMSH_SUCCESS;
}

int Pcie_MsgFifoTest(struct fmsh_pcie *pcie)
{
    u32 i;

    for (i = 0; i < 0x20; i++) {
        FSpcie_MsgSend(pcie, MSG_PME);
        delay_ms(10);//wait for interrut
        FSpcie_MsgSend(pcie, MSG_ASSERT_INTA);
        delay_ms(10);
        FSpcie_MsgSend(pcie, MSG_DEASSERT_INTA);
        delay_ms(10);
        FSpcie_MsgSend(pcie, MSG_ASSERT_INTB);
        delay_ms(10);
        FSpcie_MsgSend(pcie, MSG_DEASSERT_INTB);
        delay_ms(10);
        FSpcie_MsgSend(pcie, MSG_ASSERT_INTC);
        delay_ms(10);
        FSpcie_MsgSend(pcie, MSG_DEASSERT_INTC);
        delay_ms(10);
        FSpcie_MsgSend(pcie, MSG_ASSERT_INTD);
        delay_ms(10);
        FSpcie_MsgSend(pcie, MSG_DEASSERT_INTD);
        delay_ms(10);
        FSpcie_MsgSend(pcie, MSG_ERR_CORR);
        delay_ms(10);
        FSpcie_MsgSend(pcie, MSG_ERR_NO_FATAL);
        delay_ms(10);
        FSpcie_MsgSend(pcie, MSG_ERR_FATAL);
        delay_ms(10);
        FSpcie_MsgSend(pcie, MSG_PM_ACTIVE_STATE_NAK);
        delay_ms(10);
        FSpcie_MsgSend(pcie, MSG_PME_TO_ACK);
        delay_ms(10);
        FSpcie_MsgSend(pcie, MSG_INGNORED);
        delay_ms(10);
        FSpcie_MsgSend(pcie, MSG_LTR_MSG);
        delay_ms(10);
        FSpcie_MsgSend(pcie, MSG_SET_SLOT_POWER_LIMIT);
        delay_ms(10);
        FSpcie_VdMsgSend(pcie, MSG_VENDOR_MSG_TYPE0);
        delay_ms(10);
        FSpcie_VdMsgSend(pcie, MSG_VENDOR_MSG_TYPE1);
        delay_ms(10);
     }
     return FMSH_SUCCESS;
}



int Pcie_MsiMaskChange(struct fmsh_pcie *pcie, u32 pf_num)
{
    u32 MaskReg = 0xa0;

    FSpcie_CfgWrite(pcie, 1, 0, pf_num, MaskReg, 0x1);
}

int Pcie_MsiFifoTest(u32 type) 
{
    u32 i;
    u32 addr, header;
    
    for (i = 0; i < 0x80; i++) {
      addr = i * 4;
      
      if (i%2 == 0)
        header = 0xae860000;
      else
        header = 0x66cc0000;
      printf("MSIX trigger info, addr : 0x%x, data : 0x%x\n",
             PCIE_MSIX_SPACE + addr, header + i);
      
      /* trigger by RP*/
      if (type == 1) 
        FMSH_WriteReg(PCIE_MSIX_SPACE, addr, header + i);
      
      /* trigger by EP */
      else
        FSpcie_MsixSend(addr, header + i);
      
      /* wait for msi intrrupt */
      delay_1ms();
    }

    return FMSH_SUCCESS;
}

int Pcie_MsiVectorTest(u32 type) 
{
    u32 i;
    u32 addr, value, ret = 0;

    for (i = 0; i < 64; i++) {
      addr = i * i * 4;
      /* trigger by RP */
      if (type == 1)
        FMSH_WriteReg(PCIE_MSIX_SPACE, addr, i);
     
      /* trigger by EP */
      else
        FSpcie_MsixSend(addr, i);
      /* wait for msi intrrupt */
      printf("trigger msi-%d\n", i);
      delay_1ms();
    }
    
    return FMSH_SUCCESS;
}

/* Mio30 select PERST$ input */
int Pcie_PerstTest()
{
    u32 value;

    value = FSpcie_PerstStatusShow();
    if (value == 0) {
      printf("%s error, perst# init status error\n", __func__);
      return FMSH_FAILURE;
    }
      
    /* enable mio30, driver low */
    FMSH_WriteReg(0xff180000, 0x7c, 0x4);
    
    delay_ms(10);//wait for interrut
    if (FSpcie_PerstStatusShow()) {
      printf("%s error, perst# not driver low\n", __func__);
      return FMSH_FAILURE;
    }
    
    //* disable mio30, driver high */
    FMSH_WriteReg(0xff180000, 0x7c, 0);
    
    delay_ms(10);//wait for interrut
    value = FSpcie_PerstStatusShow();
    if (value == 0) {
      printf("%s error, perst# not driver high\n", __func__);
      return FMSH_FAILURE;
    }
    printf("%s pass\n", __func__);
    return FMSH_SUCCESS;
}

int Pcie_DmaBulkTest(u32 channel, u32 size)
{
    u32 i, reg, write, read, flag = 0;
    u32 axi_addr0 = pcie_dma_bulk_axi_addr0;
    u32 axi_addr1 = pcie_dma_bulk_axi_addr1;
    
    //init ddr
    for (i=0;i<size/4;i++)
    {
          reg = i * 4;
          write = reg + 0xae000000;
          FMSH_WriteReg(axi_addr0, reg, write);
          write = reg + 0x86000000;
          FMSH_WriteReg(axi_addr1, reg, write);
    }
    //write
    //pcie_max_playload_change(1);
    FSpcie_DmaBulkInit(channel, size);
    FSpcie_DmaBulkTrans(channel, 1);
    
    //waite for pcie read all ddr data to dma ram
    delay_ms(100);
    
    //clear ddr
    for (i=0;i<size/4;i++)
    {
         reg = i * 4;
         FMSH_WriteReg(axi_addr0, reg, 0);
         FMSH_WriteReg(axi_addr1, reg, 0);
    }
    //read back
    //pcie_max_read_request_change(3);
    FSpcie_DmaBulkInit(channel, size);
    FSpcie_DmaBulkTrans(channel, 0);
    
    //waite for pcie read all ddr data to dma ram
    delay_ms(100);
    
    //check
    for (i=0;i<size/4;i++)
    {
          reg = i * 4;
          write = reg + 0xae000000;
          read = FMSH_ReadReg(axi_addr0, reg);
          if (write != read) {
               flag = 1;
               printf("DMA BULK desc0 Error [write: 0x%x, read 0x%x]\n", write, read);
          }
          write = reg + 0x86000000;
          read = FMSH_ReadReg(axi_addr1, reg);
          if (write != read) {
               flag = 1;
               printf("DMA BULK desc1 Error [write: 0x%x, read 0x%x]\n", write, read);
          }
    }
    printf("%s pass\n", __func__);
    return FMSH_SUCCESS;
}

void Pcie_EpInboundTest()
{
    u32 bypass_bit, i, size;

    for(i = EP_APERTURE_SIZE_128B; i < EP_APERTURE_SIZE_8G; i++) {
        size = 0x80 * (1<<i);
        //head
        pcie_ddr_write(0, 0x10000000 + i);

        //end
        pcie_ddr_write(size - 0x4, 0x20000000 + i);
        printf("head (0) write 0x%x, end (0x%x) write 0x%x\n",
                0x10000000 + i, size - 0x4, 0x20000000 + i);
        delay_ms(10);
    }
}

void Pcie_RpInboundTest()
{
    u32 bypass_bit, i, size;

    for(i = RP_APERTURE_SIZE_4B; i < RP_APERTURE_SIZE_2G; i++) {
        size = 0x4 * (1<<i);
        //head
        pcie_ddr_write(0, 0x10000000 + i);

        //end
        pcie_ddr_write(size - 0x4, 0x20000000 + i);
        printf("head (0) write 0x%x, end (0x%x) write 0x%x\n",
                0x10000000 + i, size - 0x4, 0x20000000 + i);
        delay_ms(10);
    }
}

void Pcie_EpInboundTestInit()
{
    u32 bypass_bit, i;
    u32 value, size;

    //only use BAR0, 64bit
    FMSH_WriteReg(PCIE_LOCAL_MGMT_REG_BASE, I_PF_0_BAR_CONFIG_0_REG, 0xc0);
    for(i = EP_APERTURE_SIZE_128B; i < EP_APERTURE_SIZE_8G; i++) {
        FMSH_WriteReg(PCIE_LOCAL_MGMT_REG_BASE, I_PF_0_BAR_CONFIG_0_REG, 0xc0 | i);

        //AXI address
        FMSH_WriteReg(PCIE_AXI_CFG_REG_BASE,
                     INBOUND_EP_FUNC_NUM(0) + INBOUND_EP_BAR_OFFSET(0),
                     0x0);
        //config space
        FMSH_WriteReg(PCIE_CFG_SPACE_REG_BASE, 0x10, 0);
        size = 0x80 * (1<<i);
        printf("[EP]size : %x   ", size);

        /* break */
        delay_ms(10);
        printf("head 0x%x  ", FMSH_ReadReg(0, 0));
        printf("tail (0x%x) 0x%x\n", size - 0x4, FMSH_ReadReg(0, size - 0x4));
        delay_ms(10);
    }
}

void Pcie_RpInboundTestInit()
{
    u32 bypass_bit, i;
    u32 value, size;

    //only use BAR0, 64bit
    FMSH_WriteReg(PCIE_LOCAL_MGMT_REG_BASE, I_RC_BAR_CONFIG_REG, 0x180);
    for(i = RP_APERTURE_SIZE_4B; i < RP_APERTURE_SIZE_8G; i++) {
        FMSH_WriteReg(PCIE_LOCAL_MGMT_REG_BASE, I_RC_BAR_CONFIG_REG, 0x180 | i);

        bypass_bit = i+2; //256K
        //AXI address
        FMSH_WriteReg(PCIE_AXI_CFG_REG_BASE,
                     INBOUND_RP_REG_OFFSET(RP_BAR(0)) + INBOUND_AXI_LOW,
                     0x0 + bypass_bit);
        //config space
        FMSH_WriteReg(PCIE_CFG_SPACE_REG_BASE, 0x10, 0);
        size = 0x4 * (1<<i);
        printf("[RP] size : %x   ", size);

        /* break */
        delay_ms(10);
        printf("head 0x%x  ", FMSH_ReadReg(0, 0));
        printf("tail (0x%x) 0x%x\n", size - 0x4, FMSH_ReadReg(0, size - 0x4));
        delay_ms(10);
    }
}

int Pcie_DdrAccessTest(u32 offset, u32 dataSize) 
{
    u32 i, reg;
    u32 read, write, flag = 0;
	
    for (i=0;i<dataSize/4;i++)
    {
        reg = i * 4;
        write = reg + 0x11110000;
        pcie_ddr_write(reg + offset, write);
        read = pcie_ddr_read(reg + offset);
        if (write != read) {
             flag = 1;
             printf("DDR access Error [write: 0x%x, read 0x%x]\n", write, read);
        }
    }
    if (flag)
        return -1;
    printf("%s pass\n", __func__);
    return FMSH_SUCCESS;
}

int Pcie_OcmAccessTest(u32 offset, u32 dataSize) 
{
    u32 i, reg;
    u32 read, write, flag = 0;
	
    for (i=0;i<dataSize/4;i++)
    {
        reg = i * 4;
        write = reg + 0xae000000;
        pcie_ocm_write(reg + offset, write);
        read = pcie_ocm_read(reg + offset);
        if (write != read) {
             flag = 1;
             printf("OCM access Error [write: 0x%x, read 0x%x]\n", write, read);
        }
    }
    if (flag)
        return FMSH_FAILURE;
    printf("%s pass\n", __func__);
    return FMSH_SUCCESS;
}

int Pcie_DumpCfgSpace(struct fmsh_pcie *pcie, u16 bus, u16 dev, u16 func)
{
    u32 i, offset;
    u32 value[4];

    printf("config space:\n");
    for (i = 0; i < 0x100; i++) {
        offset = i* 0x10;
        value[0] = FSpcie_CfgRead(pcie, bus, dev, func, offset);
        value[1] = FSpcie_CfgRead(pcie, bus, dev, func, offset + 0x4);
        value[2] = FSpcie_CfgRead(pcie, bus, dev, func, offset + 0x8);
        value[3] = FSpcie_CfgRead(pcie, bus, dev, func, offset + 0xc);
        printf("%-3x :%-8x %-8x %-8x %-8x\n",
                offset, value[0], value[1], value[2], value[3]);
    }
}

int Pcie_CfgAccessTest(struct fmsh_pcie *pcie)
{
    u32 value, offset, i;
    u16 bus, dev, func;

    bus = 1;
    dev = 0;
    func = 0;
    offset = 0;

    value = FSpcie_CfgRead(pcie, bus, dev, func, offset);
    if (value != 0x10017cd)
      printf("PF0 Did+Vid read error, value: 0x%x\n", value);

    dev = 0;
    func = 1;
    offset = 0x10;
    FSpcie_CfgWrite(pcie, bus, dev, func, offset, 0xae000000);
    value = FSpcie_CfgRead(pcie, bus, dev, func, offset);
    if (value != 0xae000000)
      printf("PF1 bar0 write error, write: 0xae000000, read: 0x%x\n", value);
    
    dev = 0;
    func = 2;
    offset = 0x10;
    FSpcie_CfgWrite(pcie, bus, dev, func, offset, 0x86000000);
    value = FSpcie_CfgRead(pcie, bus, dev, func, offset);
    if (value != 0x86000000)
      printf("PF2 bar0 write error, write: 0x86000000, read: 0x%x\n", value);
    
    dev = 0;
    func = 3;
    offset = 0x10;
    FSpcie_CfgWrite(pcie, bus, dev, func, offset, 0x85000000);
    value = FSpcie_CfgRead(pcie, bus, dev, func, offset);
    if (value != 0x85000000)
      printf("PF3 bar0 write error, write: 0x85000000, read: 0x%x\n", value);
    
    printf("%s test success\n", __func__);
}

int fmsh_pcie_verify(void)
{    
    int ret, errcnt = 0;
    int i;
	struct fmsh_pcie *pcie;
        pcie = malloc(sizeof(struct fmsh_pcie));

	pcie->speed = PCIE_GEN;
	pcie->lane = PCIE_LANE_NUM;
	
#ifdef MPSOC_PCIE_RP 
	pcie->mode = 1;
    FSpcie_Init(pcie);
    FSpcie_LtssmTest();
    Pcie_CfgAccessTest(pcie);
    Pcie_OcmAccessTest(0, 0x1000);
    Pcie_DdrAccessTest(0x40000000, 0x1000);

    for (i = 0; i < 8; i++)
        Pcie_DmaBulkTest(i, 0x10000);

    /* msi / msi-x */
    for (i = 0; i < 4; i++)
        Pcie_MsiMaskChange(pcie, i); // for MSC-1 mask change intr
    //while(1);
    Pcie_MsiVectorTest(1);
    FSpcie_MsiFifoInit(1);
    //while(1);
    Pcie_MsiFifoTest(1);

    /* EP`s VF */
    for (i = 0; i < 4; i++)
        FSpcie_SriovInit(pcie, i);
    Pcie_VfCfgTest(pcie);
    Pcie_VfMemTest(); //need ep call Pcie_VfInit

    /* MSG FIFO (receive)*/
    FSpcie_MsgFifoPerpare();
    delay_ms(100);
    //FSpcie_MsgFifoFilterIntx();
    delay_ms(100);
    FSpcie_MsgFifoFilterErr();
    delay_ms(100);
    FSpcie_MsgFifoFilterPm();
    delay_ms(100);
    FSpcie_MsgFifoFilterSlt();
    delay_ms(100);
    FSpcie_MsgFifoFilterOth();
    delay_ms(100);
    FSpcie_MsgFifoFilterVendor();
    delay_ms(100);

    /* RCB/MPS/MRRS */
    Pcie_MpsTest(pcie, 1);
    Pcie_RcbTest();
    Pcie_MrrsTest();

    /* inbound test*/
    Pcie_EpInboundTest(); //need EP call Pcie_EpInboundTestInit
    Pcie_RpInboundTestInit();
	
    /* AER */
    Pcie_NotFatalErrTest(pcie);
    Pcie_FatalErrTest(pcie);
    Pcie_CorrErrTest(pcie); //need EP call Pcie_CorrErrInit

    /* reset */
    FSpcie_HotResetTrigger(); //EP receive HOT_RESET_IN (MSC1) intr

    Pcie_FlrTest(pcie);
#endif    
    
#ifdef MPSOC_PCIE_EP
	pcie->mode = 0;
    FSpcie_Init(pcie);
    FSpcie_LtssmTest();
    Pcie_OcmAccessTest(0, 0x1000);
    Pcie_DdrAccessTest(0x40000000, 0x1000);

    for (i = 0; i < 8; i++)
        Pcie_DmaBulkTest(i, 0x10000);

    /* msi / msi-x */
    Pcie_MsiVectorTest(0);
    Pcie_MsiFifoTest(0); //need RP call Pcie_MsiFifoInit
    
    /* INTx test */
    for (i = 0; i < 4; i++)
        Pcie_TriggerIntx(pcie, i);

    /* Msg FIFO */
    Pcie_MsgFifoTest(); //need RP call Pcie_MsgFifoPerpare

    /* RCB/MPS/MRRS */
    Pcie_MpsTest(0); //need rp call Pcie_MpsTest first
    Pcie_RcbTest();
    Pcie_MrrsTest();

    /* EP`s VF */
    Pcie_VfInit();

    /* inbound test*/
    Pcie_RpInboundTest(); //need RP call Pcie_RpInboundTestInit
    Pcie_EpInboundTestInit();
	
    /* AER */
    Pcie_CorrErrInit();
    ret = xapm_test();
    ret = xmpu_test();

    /* reset */
    Pcie_PerstTest();
#endif
    while(1);
    return FMSH_SUCCESS;
}
