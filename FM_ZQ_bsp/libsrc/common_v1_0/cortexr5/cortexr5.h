#ifndef _CORTEXR5_H_ /* prevent circular inclusions */
#define _CORTEXR5_H_ /* by using protection macros */

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

#ifndef CORTEX_R5
#define CORTEX_R5
#endif

/* Current Processor Status Register (CPSR) Bits */
#define ARM_MODE_ABT 0x17
#define ARM_MODE_FIQ 0x11
#define ARM_MODE_IRQ 0x12
#define ARM_MODE_SVC 0x13
#define ARM_MODE_SYS 0x1F
#define ARM_MODE_UND 0x1B

#define I_BIT        0x80
#define F_BIT        0x40

#define N_BIT        0x80000000U
#define Z_BIT        0x40000000U
#define C_BIT        0x20000000U
#define V_BIT        0x10000000U

/* MPU region defines */
#define REGION_32B   0x00000004U
#define REGION_64B   0x00000005U
#define REGION_128B  0x00000006U
#define REGION_256B  0x00000007U
#define REGION_512B  0x00000008U
#define REGION_1K    0x00000009U
#define REGION_2K    0x0000000AU
#define REGION_4K    0x0000000BU
#define REGION_8K    0x0000000CU
#define REGION_16K   0x0000000DU
#define REGION_32K   0x0000000EU
#define REGION_64K   0x0000000FU
#define REGION_128K  0x00000010U
#define REGION_256K  0x00000011U
#define REGION_512K  0x00000012U
#define REGION_1M    0x00000013U
#define REGION_2M    0x00000014U
#define REGION_4M    0x00000015U
#define REGION_8M    0x00000016U
#define REGION_16M   0x00000017U
#define REGION_32M   0x00000018U
#define REGION_64M   0x00000019U
#define REGION_128M  0x0000001AU
#define REGION_256M  0x0000001BU
#define REGION_512M  0x0000001CU
#define REGION_1G    0x0000001DU
#define REGION_2G    0x0000001EU
#define REGION_4G    0x0000001FU

#define REGION_EN    0x00000001U

#define SHAREABLE    0x00000004U     /*shareable */
#define STRONG_ORDERD_SHARED                                    \
    0x00000000U                      /*strongly ordered, always \
                                        shareable*/

#define DEVICE_SHARED    0x00000001U /*device, shareable*/
#define DEVICE_NONSHARED 0x00000010U /*device, non shareable*/

#define NORM_NSHARED_WT_NWA                                        \
    0x00000002U /*Outer and Inner write-through, no write-allocate \
                   non-shareable*/
#define NORM_SHARED_WT_NWA \
    0x00000006U /*Outer and Inner write-through, no write-allocate shareable*/

#define NORM_NSHARED_WB_NWA                                         \
    0x00000003U /*Outer and Inner write-back, no write-allocate non \
                   shareable*/
#define NORM_SHARED_WB_NWA \
    0x00000007U /*Outer and Inner write-back, no write-allocate shareable*/

#define NORM_NSHARED_NCACHE \
    0x00000008U /*Outer and Inner Non cacheable  non shareable*/
#define NORM_SHARED_NCACHE \
    0x0000000CU /*Outer and Inner Non cacheable shareable*/

#define NORM_NSHARED_WB_WA                       \
    0x0000000BU /*Outer and Inner write-back non \
                   shared*/
#define NORM_SHARED_WB_WA     0x0000000FU /*Outer and Inner write-back shared*/

/* inner and outer cache policies can be combined for different combinations */
#define NORM_IN_POLICY_NCACHE 0x00000020U /*inner non cacheable*/
#define NORM_IN_POLICY_WB_WA  0x00000021U /*inner write back write allocate*/
#define NORM_IN_POLICY_WT_NWA \
    0x00000022U /*inner write through no write allocate*/
#define NORM_IN_POLICY_WB_NWA               \
    0x00000023U /*inner write back no write \
                   allocate*/

#define NORM_OUT_POLICY_NCACHE 0x00000020U /*outer non cacheable*/
#define NORM_OUT_POLICY_WB_WA  0x00000028U /*outer write back write allocate*/
#define NORM_OUT_POLICY_WT_NWA \
    0x00000030U                  /*outer write through no write allocate*/
#define NORM_OUT_POLICY_WB_NWA \
    0x00000038U                  /*outer write back no write allocate*/

#define NO_ACCESS       (0x0U)   /*No access*/
#define PRIV_RW_USER_NA (0x100U) /*Privileged access only*/
#define PRIV_RW_USER_RO \
    (0x200U) /*Writes in User mode generate permission faults*/
#define PRIV_RW_USER_RW                  (0x300U)  /*Full Access*/
#define PRIV_RO_USER_NA                  (0x500U)  /*Privileged eead only*/
#define PRIV_RO_USER_RO                  (0x600U)  /*Privileged/User read-only*/

#define EXECUTE_NEVER                    (0x1000U) /* Bit 12*/

/* CP15 defines */

/* C0 Register defines */
#define CP15_MAIN_ID                     "p15, 0, %0,  c0,  c0, 0"
#define CP15_CACHE_TYPE                  "p15, 0, %0,  c0,  c0, 1"
#define CP15_TCM_TYPE                    "p15, 0, %0,  c0,  c0, 2"
#define CP15_MPU_TYPE                    "p15, 0, %0,  c0,  c0, 4"
#define CP15_MULTI_PROC_AFFINITY         "p15, 0, %0,  c0,  c0, 5"

#define CP15_PROC_FEATURE_0              "p15, 0, %0,  c0,  c1, 0"
#define CP15_PROC_FEATURE_1              "p15, 0, %0,  c0,  c1, 1"
#define CP15_DEBUG_FEATURE_0             "p15, 0, %0,  c0,  c1, 2"
#define CP15_AUX_FEATURE_0               "p15, 0, %0,  c0,  c1, 3"
#define CP15_MEMORY_FEATURE_0            "p15, 0, %0,  c0,  c1, 4"
#define CP15_MEMORY_FEATURE_1            "p15, 0, %0,  c0,  c1, 5"
#define CP15_MEMORY_FEATURE_2            "p15, 0, %0,  c0,  c1, 6"
#define CP15_MEMORY_FEATURE_3            "p15, 0, %0,  c0,  c1, 7"

#define CP15_INST_FEATURE_0              "p15, 0, %0,  c0,  c2, 0"
#define CP15_INST_FEATURE_1              "p15, 0, %0,  c0,  c2, 1"
#define CP15_INST_FEATURE_2              "p15, 0, %0,  c0,  c2, 2"
#define CP15_INST_FEATURE_3              "p15, 0, %0,  c0,  c2, 3"
#define CP15_INST_FEATURE_4              "p15, 0, %0,  c0,  c2, 4"
#define CP15_INST_FEATURE_5              "p15, 0, %0,  c0,  c2, 5"

#define CP15_CACHE_SIZE_ID               "p15, 1, %0,  c0,  c0, 0"
#define CP15_CACHE_LEVEL_ID              "p15, 1, %0,  c0,  c0, 1"
#define CP15_AUXILARY_ID                 "p15, 1, %0,  c0,  c0, 7"

#define CP15_CACHE_SIZE_SEL              "p15, 2, %0,  c0,  c0, 0"

/* C1 Register Defines */
#define CP15_SYS_CONTROL                 "p15, 0, %0,  c1,  c0, 0"
#define CP15_AUX_CONTROL                 "p15, 0, %0,  c1,  c0, 1"
#define CP15_CP_ACCESS_CONTROL           "p15, 0, %0,  c1,  c0, 2"

/* XREG_CP15_CONTROL bit defines */
#define CP15_CONTROL_IE_BIT              0x80000000U
#define CP15_CONTROL_TE_BIT              0x40000000U
#define CP15_CONTROL_AFE_BIT             0x20000000U
#define CP15_CONTROL_TRE_BIT             0x10000000U
#define CP15_CONTROL_NMFI_BIT            0x08000000U
#define CP15_CONTROL_EE_BIT              0x02000000U
#define CP15_CONTROL_VE_BIT              0x01000000U
#define CP15_CONTROL_FI_BIT              0x00200000U
#define CP15_CONTROL_DZ_BIT              0x00080000U
#define CP15_CONTROL_BR_BIT              0x00020000U
#define CP15_CONTROL_RR_BIT              0x00004000U
#define CP15_CONTROL_V_BIT               0x00002000U
#define CP15_CONTROL_I_BIT               0x00001000U
#define CP15_CONTROL_Z_BIT               0x00000800U
#define CP15_CONTROL_SW_BIT              0x00000400U
#define CP15_CONTROL_C_BIT               0x00000004U
#define CP15_CONTROL_A_BIT               0x00000002U
#define CP15_CONTROL_M_BIT               0x00000001U
/* C2 Register Defines */
/* Not Used */

/* C3 Register Defines */
/* Not Used */

/* C4 Register Defines */
/* Not Used */

/* C5 Register Defines */
#define CP15_DATA_FAULT_STATUS           "p15, 0, %0,  c5,  c0, 0"
#define CP15_INST_FAULT_STATUS           "p15, 0, %0,  c5,  c0, 1"

#define CP15_AUX_DATA_FAULT_STATUS       "p15, 0, %0,  c5,  c1, 0"
#define CP15_AUX_INST_FAULT_STATUS       "p15, 0, %0,  c5,  c1, 1"

/* C6 Register Defines */
#define CP15_DATA_FAULT_ADDRESS          "p15, 0, %0,  c6,  c0, 0"
#define CP15_INST_FAULT_ADDRESS          "p15, 0, %0,  c6,  c0, 2"

#define CP15_MPU_REG_BASEADDR            "p15, 0, %0,  c6,  c1, 0"
#define CP15_MPU_REG_SIZE_EN             "p15, 0, %0,  c6,  c1, 2"
#define CP15_MPU_REG_ACCESS_CTRL         "p15, 0, %0,  c6,  c1, 4"

#define CP15_MPU_MEMORY_REG_NUMBER       "p15, 0, %0,  c6,  c2, 0"

/* C7 Register Defines */
#define CP15_NOP                         "p15, 0, %0,  c7,  c0, 4"

#define CP15_INVAL_IC_POU                "p15, 0, %0,  c7,  c5, 0"
#define CP15_INVAL_IC_LINE_MVA_POU       "p15, 0, %0,  c7,  c5, 1"

/* The CP15 register access below has been deprecated in favor of the new
 * isb instruction in Cortex R5.
 */
#define CP15_INST_SYNC_BARRIER           "p15, 0, %0,  c7,  c5, 4"
#define CP15_INVAL_BRANCH_ARRAY          "p15, 0, %0,  c7,  c5, 6"
#define CP15_INVAL_BRANCH_ARRAY_LINE     "p15, 0, %0,  c7,  c5, 7"

#define CP15_INVAL_DC_LINE_MVA_POC       "p15, 0, %0,  c7,  c6, 1"
#define CP15_INVAL_DC_LINE_SW            "p15, 0, %0,  c7,  c6, 2"

#define CP15_CLEAN_DC_LINE_MVA_POC       "p15, 0, %0,  c7, c10, 1"
#define CP15_CLEAN_DC_LINE_SW            "p15, 0, %0,  c7, c10, 2"

#define CP15_INVAL_DC_ALL                "p15, 0, %0,  c15, c5, 0"
/* The next two CP15 register accesses below have been deprecated in favor
 * of the new dsb and dmb instructions in Cortex R5.
 */
#define CP15_DATA_SYNC_BARRIER           "p15, 0, %0,  c7, c10, 4"
#define CP15_DATA_MEMORY_BARRIER         "p15, 0, %0,  c7, c10, 5"

#define CP15_CLEAN_DC_LINE_MVA_POU       "p15, 0, %0,  c7, c11, 1"

#define CP15_NOP2                        "p15, 0, %0,  c7, c13, 1"

#define CP15_CLEAN_INVAL_DC_LINE_MVA_POC "p15, 0, %0,  c7, c14, 1"
#define CP15_CLEAN_INVAL_DC_LINE_SW      "p15, 0, %0,  c7, c14, 2"

/* C8 Register Defines */
/* Not Used */

/* C9 Register Defines */

#define CP15_ATCM_REG_SIZE_ADDR          "p15, 0, %0,  c9, c1, 1"
#define CP15_BTCM_REG_SIZE_ADDR          "p15, 0, %0,  c9, c1, 0"
#define CP15_TCM_SELECTION               "p15, 0, %0,  c9, c2, 0"

#define CP15_PERF_MONITOR_CTRL           "p15, 0, %0,  c9, c12, 0"
#define CP15_COUNT_ENABLE_SET            "p15, 0, %0,  c9, c12, 1"
#define CP15_COUNT_ENABLE_CLR            "p15, 0, %0,  c9, c12, 2"
#define CP15_V_FLAG_STATUS               "p15, 0, %0,  c9, c12, 3"
#define CP15_SW_INC                      "p15, 0, %0,  c9, c12, 4"
#define CP15_EVENT_CNTR_SEL              "p15, 0, %0,  c9, c12, 5"

#define CP15_PERF_CYCLE_COUNTER          "p15, 0, %0,  c9, c13, 0"
#define CP15_EVENT_TYPE_SEL              "p15, 0, %0,  c9, c13, 1"
#define CP15_PERF_MONITOR_COUNT          "p15, 0, %0,  c9, c13, 2"

#define CP15_USER_ENABLE                 "p15, 0, %0,  c9, c14, 0"
#define CP15_INTR_ENABLE_SET             "p15, 0, %0,  c9, c14, 1"
#define CP15_INTR_ENABLE_CLR             "p15, 0, %0,  c9, c14, 2"

/* C10 Register Defines */
/* Not used */

/* C11 Register Defines */
/* Not used */

/* C12 Register Defines */
/* Not used */

/* C13 Register Defines */
#define CP15_CONTEXT_ID                  "p15, 0, %0, c13,  c0, 1"
#define USER_RW_THREAD_PID               "p15, 0, %0, c13,  c0, 2"
#define USER_RO_THREAD_PID               "p15, 0, %0, c13,  c0, 3"
#define USER_PRIV_THREAD_PID             "p15, 0, %0, c13,  c0, 4"

/* C14 Register Defines */
/* not used */

/* C15 Register Defines */
#define CP15_SEC_AUX_CTRL                "p15, 0, %0, c15,  c0, 0"
#define CP15_NORM_AXI_IFACE_REG          "p15, 0, %0, c15,  c0, 1"
#define CP15_VIRT_AXI_IFACE_REG          "p15, 0, %0, c15,  c0, 2"
#define CP15_AHB_IFACE_REG               "p15, 0, %0, c15,  c0, 3"

#define CP15_nVAL_IRQ_EN_SET             "p15, 0, %0, c15,  c1, 0"
#define CP15_nVAL_FIQ_EN_SET             "p15, 0, %0, c15,  c1, 1"
#define CP15_nVAL_RST_EN_SET             "p15, 0, %0, c15,  c1, 2"
#define CP15_nVAL_DBGREQ_EN              "p15, 0, %0, c15,  c1, 3"
#define CP15_nVAL_IRQ_EN_CLR             "p15, 0, %0, c15,  c1, 4"
#define CP15_nVAL_FIQ_EN_CLR             "p15, 0, %0, c15,  c1, 5"
#define CP15_nVAL_RST_EN_CLR             "p15, 0, %0, c15,  c1, 6"
#define CP15_nVAL_DBGREQ_EN_CLR          "p15, 0, %0, c15,  c1, 7"

#define CP15_BUILD_OPTIONS1              "p15, 0, %0, c15,  c2, 0"
#define CP15_BUILD_OPTIONS2              "p15, 0, %0, c15,  c2, 1"
#define CP15_PIN_OPTIONS                 "p15, 0, %0, c15,  c2, 7"

#define CP15_CORR_FAULT_LOC              "p15, 0, %0, c15,  c3, 0"

/* PMU */
#define PMU_CTRCOUNT                     (3U)
/* The following constants define the Cortex-R5 Performance Monitor Events */

/*
 * Software increment. The register is incremented only on writes to the
 * Software Increment Register
 */
#define PMU_EVENT_SOFTINCR               0x00U
/*
 * Instruction cache miss
 * Each Instruction fetch from normal cacheable memory that causes a refill from
 * the level 2 memory system generates this event.Accesses that do not cause a
 * new cache refill, but are satisfied from refilling data of a previous miss
 * are not counted. Where instruction fetches consists of multiple instructions,
 * these accesses count as single events.
 */
#define PMU_EVENT_INSTRCACHEMISS         0x01
/*
 * Data cache miss
 * Each data read from or write to normal cacheable memory that causes a refill
 * from the level 2 memory system generates this event.Accesses that do not
 * cause a new cache refill, but are satisfied from refilling data of a previous
 * miss are not counted. Each access to a cache line to normal cacheable memory
 * that causes a new linefill is counted,including the multiple transactions of
 * an LDM and STM
 */
#define PMU_EVENT_DATACACHEMISS          0x03
/*
 * Each access to a cache line is counted including the multiple transactions of
 * an LDM, STM, or other operations.
 */
#define PMU_EVENT_DATACACHEACCESS        0x04
/*
 *Data read architecturally executed
 *This evevt occurs for every every instruction that explicitly reads data,
 *including SWP.
 */
#define PMU_EVENT_DATAREAD               0x06
/*
 *Data write architecturally executed
 *This evevt occurs for every every instruction that explicitly writes data,
 *including SWP.
 */
#define PMU_EVENT_DATAWRITE              0x07
/*
 *Instruction architecturally executed
 */
#define PMU_EVENT_INSTR                  0x08
/*
 * Dual-issued pair of instructions architecturally executed
 */
#define PMU_EVENT_DUALINSTR              0x5E
/*
 * Exception taken
 * This event occurs on each exception taken
 */
#define PMU_EVENT_EXCEPTION              0x09
/*
 * Exception return architecturally executed.
 * This event occurs on every exception return
 * for example:
 * RFE,MOVS PC,LDM Rn, {..,PC}^
 */
#define PMU_EVENT_EXCEPTIONRET           0x0A
/*
 * Change to Context ID Executed
 */
#define PMU_EVENT_CHANGETOCONTEXID       0x0B
/*
 * Software Change of PC, except by an exception,
 * architecturally executed
 */
#define PMU_EVENT_SWCHANGE               0x0C
/*
 * B immediate, BL immediate or BLX immediate instruction
 * architecturally executed
 *
 */
#define PMU_EVENT_IMMEDIATEINSTR         0x0D
/*
 * Procedure return architecturally executed, other than exception returns
 * For example:
 * BZ Rm, "LDM Rn, {..,PC}"
 */
#define PMU_EVENT_PROCEDURERET           0x0E
/*
 * Unaligned access architecturally executed
 * This event occurs for each instruction that was to an unaligned
 * address that either triggered an alignment fault, or would have done
 * so if the SCTLR A-bit had been set.
 */
#define PMU_EVENT_UNALIGNACCESS          0x0F
/*
 * Branch mispredicted or not predicted
 * This event ocurs for every pipeline flush
 * caused by a branch
 */
#define PMU_EVENT_BRANCHMISPREDICT       0x10
/*
 * Counts clock cycles when the Cortex-r5 processor is not in WFE/WFI. This
 * event is not exported on the EVENT bus
 */
#define PMU_EVENT_CLOCKCYCLES            0x11U
/*
 * Branches or other change in program flow that could have
 * been predicted by the branch prediction resources of the processor.
 */
#define PMU_EVENT_BRANCHPREDICT          0x12
/*
 * Stall because instruction buffer cannot deliver an instruction
 * This can indicate a cache miss. This event occurs every cycle where
 * the conditions is present.
 */
#define PMU_EVENT_INSTRSTALL             0x40
/*
 * Stall because of data dependency between instructions.
 * This event occurs every cycle where the condition is present.
 */
#define PMU_EVENT_DATASTALL              0x41
/*
 * Data cache write-back
 * This event occurs once for each line that is written back from the cache.
 */
#define PMU_EVENT_DATACACHEWRITE         0x42
/*
 * External memory request
 * Examples of this are cache refill, Non-cacheable accesses, write through
 * writes, cache line evictions(write-back)
 */
#define PMU_EVENT_EXTERNALMEMREQ         0x43U
/*
 * Stall because of LSU being busy
 * This event takes place each clock cycle where the condition is met.
 * A high incidence of this event indicates the pipeline is often waiting
 * for transactions to complete on the external bus.
 */
#define PMU_EVENT_LSUSTALL               0x44
/*
 * Store Buffer was forced to drain completely
 * Examples of this Fir cortex-R5 are DMB, Strongly ordered memory access,
 * or similar events.
 */
#define PMU_EVENT_FORCEDRAINSTORE        0x45
// Instruction cache tag RAm parity or correctable ECC error
#define PMU_EVENT_INSTRTAGPARITY         0x4A
// Instruction cache data RAm parity or correctable ECC error
#define PMU_EVENT_INSTRDATAPARITY        0x4B
/*
 * Data cache tag or dirty RAM parity error or correctable ECC error,
 * from data-side or ACP
 */
#define PMU_EVENT_DATATAGPARITY          0x4C
// Data cache data RAM parity error or correctable ECC error
#define PMU_EVENT_DATADATAPARITY         0x4D
// TCM fatal ECC error reported from the prefetch unit
#define PMU_EVENT_TCMERRORPREFETCH       0x4E
// TCM fatal ECC error reported from the load/store unit
#define PMU_EVENT_TCMERRORSTORE          0x4F
// Instruction cache access
#define PMU_EVENT_INSTRCACHEACCESS       0x58
// Dual issue case A(branch)
#define PMU_EVENT_DUALISSUEA             0x5A
// Dual issue case B1,B2,F2(load/store), F2D.
#define PMU_EVENT_DUALISSUEB             0x5B
// Dual issue other case
#define PMU_EVENT_DUALISSUEOTHER         0x5C
// Double precision floating point arithmetic or conversion instruction
// executed.
#define PMU_EVENT_FPA                    0x5D
// Data cache data RAM fatal ECC error
#define PMU_EVENT_DATACACHEDATAERROR     0x60
// Data cache tag/dirty RAM fatal ECC error, from data-side or ACP
#define PMU_EVENT_DATACACHETAGERROR      0x61
// Processor livelock because of hard errors or exception vector
#define PMU_EVENT_PROCESSORLIVELOCK      0x62
// ATCM Multi-bit error
#define PMU_EVENT_ATCMMULTIBITERROR      0x64
// B0TCM Multi-bit error
#define PMU_EVENT_B0TCMMULTIBITERROR     0x65
// B1TCM Multi-bit error
#define PMU_EVENT_B1TCMMULTIBITERROR     0x66
// ATCM Single-bit error
#define PMU_EVENT_ATCMSINGLEBITERROR     0x67
// B0TCM Single-bit error
#define PMU_EVENT_B0TCMSINGLEBITERROR    0x68
// B1TCM Single-bit error
#define PMU_EVENT_B1TCMSINGLEBITERROR    0x69
// TCM correctable ECC error reported by load/store unit
#define PMU_EVENT_TCMERRORLSU            0x6A
// TCM correctable ECC error reported by prefetch unit
#define PMU_EVENT_TCMERRORPFU            0x6B
// TCM fatal ECC error reported by AXI slave unit
#define PMU_EVENT_TCMFATALERRORAXI       0x6C
// TCM correctable ECC error reported by AXI slave unit
#define PMU_EVENT_TCMERRORAXI            0x6D
// All correctable event
#define PMU_CORR_EVENTS                  0x6E
// All fatal bus faults
#define PMU_FATAL_EVENTS                 0x6F
// All correctable bus faults
#define PMU_CORR_BUS_FAULTS              0x70
// All fatal bus faults
#define PMU_FATAL_BUS_FAULTS             0x71
// ACP D-cache access, lookup or invalidate
#define PMU_EVENT_DCACHEACCESS           0x72
// ACP D-cache invalidate
#define PMU_EVENT_DCACHEINVALIDATE       0x73

#define PMU_CYCLE_CNT_DIV                0x8
#define PMU_CYCLE_CNT_RESET              0x4
#define PMU_EVENT_CNT_RESET              0x2
#define PMU_CNT_ENABLE                   0x1

#define PMU_CYCLE_CNT_MASK               0x80000000
#define PMU_EVENT_CNT_2_MASK             0x4
#define PMU_EVENT_CNT_1_MASK             0x2
#define PMU_EVENT_CNT_0_MASK             0x1

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* CORTEXR5_H */
