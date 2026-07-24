/* The overview of translation table memory attributes is described below.
*
*|                       | Memory Range                | Definition in Translation Table   |
*|-----------------------|-----------------------------|-----------------------------------|
*| DDR                   | 0x0000000000 - 0x007FFFFFFF | Normal write-back Cacheable       |
*| PL                    | 0x0080000000 - 0x00BFFFFFFF | Strongly Ordered                  |
*| QSPI, lower PCIe      | 0x00C0000000 - 0x00EFFFFFFF | Strongly Ordered                  |
*| CP slaves             | 0x00F0000000 - 0x00F1FFFFFF | Strongly Ordered                  |
*| Reserved              | 0x00F2000000 - 0x00F3FFFFFF | Unassigned                        |
*| DDR PHY               | 0x00F4000000 - 0x00F5FFFFFF | Strongly Ordered                  |
*| Reserved              | 0x00F6000000 - 0x00F7FFFFFF | Unassigned                        |
*| STM Coresight         | 0x00F8000000 - 0x00F8FFFFFF | Strongly Ordered                  |
*| GIC                   | 0x00F9000000 - 0x00F91FFFFF | Strongly Ordered                  |
*| Reserved              | 0x00F9200000 - 0x00FA7FFFFF | Unassigned                        |
*| PCIE REG              | 0x00FA800000 - 0x00FAFFFFFF | Strongly Ordered                  |
*| Reserved              | 0x00FB000000 - 0x00FCFFFFFF | Strongly Ordered                  |
*| FPS, LPS slaves       | 0x00FD000000 - 0x00FFBFFFFF | Strongly Ordered                  |
*| CSU, PMU              | 0x00FFC00000 - 0x00FFDFFFFF | Strongly Ordered                  |
*| TCM, OCM              | 0x00FFE00000 - 0x00FFFFFFFF | Normal inner write-back cacheable |
*| Reserved              | 0x0100000000 - 0x03FFFFFFFF | Unassigned                        |
*| PL, PCIe              | 0x0400000000 - 0x07FFFFFFFF | Strongly Ordered                  |
*| DDR                   | 0x0800000000 - 0x0FFFFFFFFF | Normal inner write-back cacheable |
*| PL, PCIe              | 0x1000000000 - 0xBFFFFFFFFF | Strongly Ordered                  |
*| Reserved              | 0xC000000000 - 0xFFFFFFFFFF | Unassigned                        |
*/

#include "armv8/bspconfig.h"

    public MMUTableL0
    public MMUTableL1
    public MMUTableL2
    //public MMUTableL3

#define REGION_SIZE_L1              (1024)
#define REGION_SIZE_L2              (2)
#define DDR_SIZE_REPT               (DDR_SIZE_MB_LOW / REGION_SIZE_L2)
#define DDR_CABOOT_SIZE_REPT        (16 / REGION_SIZE_L2)   /*CA53 BOOT 16M cachable 0x0000__0000 - 0x00ff_ffff*/
#define DDR_LOG_MSG_SIZE_REPT       (64 / REGION_SIZE_L2)  /*LOG DATA MSG 64M nocachable 0x0100__0000 - 0x04ff_ffff*/
#define DDR_NR_SIZE_REPT            (32 / REGION_SIZE_L2)  /*NR 32M cachable 0x0500__0000 - 0x06ff_ffff*/
#define DDR_MSG_SIZE_REPT           (32 / REGION_SIZE_L2)  /*LOG DATA MSG 32M nocachable 0x0700__0000 - 0x08ff_ffff*/
#define DDR_R50_R51_SIZE_REPT       (256 / REGION_SIZE_L2)  /*R50 R51 256M cachable 0x0900__0000 - 0x18ff_ffff*/

#define DDR_NCACHE_SIZE_REPT        (32 / REGION_SIZE_L2)   /* DDR last 32MB noncacheable*/
#define DDR_CACHE_SIZE_REPT         (DDR_SIZE_REPT - DDR_NCACHE_SIZE_REPT)
/*CA53 r0 code cachable 0x0900__0000 - 0x7fff_ffff - DDR_NCACHE_SIZE_REPT*/
#define DDR_CA53_SIZE_REPT          (DDR_SIZE_REPT - DDR_CABOOT_SIZE_REPT - DDR_LOG_MSG_SIZE_REPT - DDR_MSG_SIZE_REPT - DDR_NR_SIZE_REPT - DDR_NCACHE_SIZE_REPT - DDR_R50_R51_SIZE_REPT)  
/* high ddr size */
#define DDR_1_SIZE  (DDR_SIZE_MB_HIGH * 1024 * 1024)

//#ifndef EL3_SECURE
//memory           set 0x445 | (2 << 8) // | (1 << 5)  /*NS and outer shareable   */
memory           set 0x405 | (2 << 8) // | (1 << 5)  /*NS and outer shareable   */ // (1<<6) el0 for read/w
device           set 0x409 | (1 << 53) | (1 << 54) // | (1 << 5) /* The Privileged and Unprivileged execute-never   */
nc_memory        set 0x401 | (2 << 8) 
memory_read_only set 0x405 | (2 << 8) | (3 <<6)
//#else
//memory           set 0x405 | (2 << 8)       /*Secure and outer shareable   */
//device           set 0x409 | (1 << 53) | (1 << 54) /* The Privileged and Unprivileged execute-never   */
//#endif

table_descriptor set (0x3) //|(1<<63)   /* next level table located in nonsecure memory  */
l3_page          set (0x3)
read_only        set (3 <<6)
reserved         set 0


    section .mmu_tbl:CONST:NOROOT(12)
MMUTableL0:
// .set SECT, MMUTableL1		
    dc64	MMUTableL1 + 0x3 + (0<<63) /* 0x0000_0000 -  0x7F_FFFF_FFFF */
    dc64	MMUTableL1 + 0x1000 + 0x3  /* 0x80_0000_0000 - 0xFF_FFFF_FFFF */

    section .mmu_tbl:CONST:NOROOT(12)
MMUTableL1:
/* 1GB DDR,use block descripter,Normal, Inner/Outer WB/WA/RA,outer sharable,read-write permission */
BASE_ADDR   set 0
    dc64    MMUTableL2 + table_descriptor          /* 1GB DDR */    
    dc64    MMUTableL2 + 0x1000 + table_descriptor   /* 1GB DDR */   

BASE_ADDR   set 0x80000000
    dc64    BASE_ADDR + device /* 1GB PL */
    dc64    MMUTableL2 + 0x2000 + table_descriptor
  //  dc64    MMUTableL2 + 0x1000 + table_descriptor /* 1GB other devices n memory */

    /* 4GB ~ ...*/
BASE_ADDR   set 0x100000000

    rept    0xc                     /* 0x0001_0000_0000 - 0x0003_FFFF_FFFF */
    dc64    BASE_ADDR + reserved    /* 12GB Reserved */
BASE_ADDR   set BASE_ADDR + 0x40000000
    endr

    rept    0x10                    /* 0x0004_0000_0000 - 0x0007_FFFF_FFFF */
    dc64    BASE_ADDR + device    /* 8GB PL, 8GB PCIe */
BASE_ADDR   set BASE_ADDR + 0x40000000
    endr

    /* high DDR */
#ifdef DDR_1_SIZE
#if DDR_1_SIZE > 0x800000000
/* If DDR size is larger than 32GB, truncate to 32GB */
DDR_1_REG  set 0x20
#else
DDR_1_REG  set DDR_1_SIZE/0x40000000
#endif
#else
DDR_1_REG  set 0x0
//    #warning "There's no DDR_1_SIZE defined. MMU translation table marks 32 GB DDR address space as undefined"
#endif

UNDEF_1_REG set 0x20-DDR_1_REG

    rept    DDR_1_REG                    /* high ddr size */
    dc64    BASE_ADDR + memory    /* DDR */
BASE_ADDR   set BASE_ADDR + 0x40000000
    endr

    rept    UNDEF_1_REG                  /* reserved for region where ddr is absent */
    dc64    BASE_ADDR + reserved   
BASE_ADDR   set BASE_ADDR + 0x40000000
    endr

    rept    0x1C0                /* 0x0010_0000_0000 - 0x007F_FFFF_FFFF */
    dc64    BASE_ADDR + device   /* 448 GB PL */
BASE_ADDR   set BASE_ADDR + 0x40000000
    endr
    
    rept    0x100                /* 0x0080_0000_0000 - 0x00BF_FFFF_FFFF */
    dc64    BASE_ADDR + device   /* 256GB PCIe */
BASE_ADDR   set BASE_ADDR + 0x40000000
    endr
    
    rept    0x100                /* 0x00C0_0000_0000 - 0x00FF_FFFF_FFFF */
    dc64    BASE_ADDR + reserved /* 256GB reserved */
BASE_ADDR   set BASE_ADDR + 0x40000000
    endr

    section .mmu_tbl:CONST:NOROOT(12)
  
MMUTableL2:
/* low ddr */
UNDEF_0_REG set     0x400-DDR_SIZE_REPT
BASE_ADDR   set 0x00000000
#if (USE_DDR == 1) && (PS_PREINITED == 1)

    /* DDR cacheable area*/
/*
    rept    DDR_CACHE_SIZE_REPT
    dc64    BASE_ADDR + memory
BASE_ADDR   set BASE_ADDR + 0x200000
    endr
*/
    /*CA53 BOOT 16M cachable 0x0000__0000 - 0x00ff_ffff*/
    rept    DDR_CABOOT_SIZE_REPT
    dc64    BASE_ADDR + memory
BASE_ADDR   set BASE_ADDR + 0x200000
    endr
    
    /*LOG DATA MSG 64M nocachable 0x0100__0000 - 0x04ff_ffff*/
    rept    DDR_LOG_MSG_SIZE_REPT
    dc64    BASE_ADDR + nc_memory
BASE_ADDR   set BASE_ADDR + 0x200000
    endr

    /*NR cachable 0x0500__0000 - 0x6fff_ffff - DDR_NCACHE_SIZE_REPT*/
    rept    DDR_NR_SIZE_REPT
    dc64    BASE_ADDR + memory
BASE_ADDR   set BASE_ADDR + 0x200000
    endr

    /*MSG 32M nocachable 0x0700__0000 - 0x08ff_ffff*/
    rept    DDR_MSG_SIZE_REPT
    dc64    BASE_ADDR + nc_memory
BASE_ADDR   set BASE_ADDR + 0x200000
    endr

    /*R50 R51 256M nocachable 0x0900__0000 - 0x18ff_ffff*/
    rept    DDR_R50_R51_SIZE_REPT
    dc64    BASE_ADDR + memory_read_only
BASE_ADDR   set BASE_ADDR + 0x200000
    endr

    /*CA53 r0 code cachable 0x1900__0000 - 0x7fff_ffff - DDR_NCACHE_SIZE_REPT*/
    rept    DDR_CA53_SIZE_REPT
    dc64    BASE_ADDR + memory
BASE_ADDR   set BASE_ADDR + 0x200000
    endr
    
    /* DDR last 16MB noncacheable*/
    rept    DDR_NCACHE_SIZE_REPT
    dc64    BASE_ADDR + nc_memory
BASE_ADDR   set BASE_ADDR + 0x200000
    endr
    
#else //(PS_PREINITED == 1) && (USE_DDR == 1)

    /* DDR area*/
    rept	DDR_SIZE_REPT
    dc64    BASE_ADDR + reserved  
BASE_ADDR   set BASE_ADDR + 0x200000 
    endr
    
#endif

    /* UNDEF_DDR 0 reserved */
    rept	UNDEF_0_REG
    dc64    BASE_ADDR + reserved  
BASE_ADDR   set BASE_ADDR + 0x200000 
    endr
    
BASE_ADDR   set 0xc0000000
    rept	0x0100			    /* 0xC000_0000 - 0xDFFF_FFFF */
    dc64    BASE_ADDR + device  /* 512MB QSPI */
BASE_ADDR   set BASE_ADDR + 0x200000 
    endr
    
    rept	0x80                /* 0xE000_0000 - 0xEFFF_FFFF */
    dc64    BASE_ADDR + device  /* 256MB lower PCIe */
BASE_ADDR   set BASE_ADDR + 0x200000
    endr

    rept	0x10			        /* 0xF000_0000 - 0xF1FF_FFFF */
    dc64    BASE_ADDR + device	    /* 32MB CP salves */
BASE_ADDR   set BASE_ADDR + 0x200000
    endr

    rept	0x010			        /* 0xF200_0000 - 0xF3FF_FFFF */
    dc64    BASE_ADDR + reserved	/* 32MB Reserved */
BASE_ADDR   set BASE_ADDR + 0x200000
    endr
    
    rept	0x010			        /* 0xF400_0000 - 0xF5FF_FFFF */
    dc64    BASE_ADDR + device	/* 32MB DDR PHY */
BASE_ADDR   set BASE_ADDR + 0x200000
    endr
    
    rept	0x010			        /* 0xF600_0000 - 0xF7FF_FFFF */
    dc64    BASE_ADDR + reserved	/* 32MB Reserved */
BASE_ADDR   set BASE_ADDR + 0x200000
    endr
    
    rept	0x8			            /* 0xF800_0000 - 0xF8FF_FFFF */
    dc64    BASE_ADDR + device	/* 16MB coresight */
BASE_ADDR   set BASE_ADDR + 0x200000
    endr
    
    /* 1MB RPU LLP is marked for 2MB region as the minimum block size in
   translation table is 2MB and adjacent 63MB reserved region is
   converted to 62MB */
    rept	0x1			        /* 0xF900_0000 - 0xF91F_FFFF */
    dc64    BASE_ADDR + device	/* 2MB RPU low latency port */
BASE_ADDR   set BASE_ADDR + 0x200000
    endr

    rept	0xb         			/* 0xF920_0000 - 0xFA7F_FFFF */
    dc64    BASE_ADDR + reserved	/* 22MB Reserved */
BASE_ADDR   set BASE_ADDR + 0x200000
    endr

    rept	4			            /* 0xFA80_0000 - 0xFAFF_FFFF */
    dc64    BASE_ADDR + device	/* 8MB PCIE REG */
BASE_ADDR   set BASE_ADDR + 0x200000
    endr
    
    rept	0x10			        /* 0xFB00_0000 - 0xFCFF_FFFF */
    dc64    BASE_ADDR + reserved	/* 32MB Reserved */
BASE_ADDR   set BASE_ADDR + 0x200000
    endr

    rept	0x8			/* 0xFD00_0000 - 0xFDFF_FFFF */
    dc64    BASE_ADDR + device	/* 16MB FPS */
BASE_ADDR   set BASE_ADDR + 0x200000
    endr
    
    rept	0xE			/* 0xFE00_0000 -  0xFFBF_FFFF */
    dc64    BASE_ADDR + device	/* 28MB LPS */
BASE_ADDR   set BASE_ADDR + 0x200000
    endr
                            /* 0xFFC0_0000 - 0xFFDF_FFFF */
    dc64    BASE_ADDR + device	/*2MB PMU/CSU */
BASE_ADDR   set BASE_ADDR + 0x200000

    dc64    BASE_ADDR + nc_memory  /*2MB OCM/TCM*/

    section .mmu_tbl:CONST:NOROOT(12)
MMUTableL3:

    END
    