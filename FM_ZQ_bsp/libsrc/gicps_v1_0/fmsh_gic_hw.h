/******************************************************************************
**
*
* @file fmsh_gic_hw.j
* @addtogroup scugic_v3_1
* @{
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- --------------------------------------------------------
* 1.00  lxl  29/12/2018 Fisrt edit
*
* </pre>
*

*
******************************************************************************/

#ifndef _FMSH_GIC_HW_H_ /* prevent circular inclusions */
#define _FMSH_GIC_HW_H_ /* by using protection macros */

#ifdef __cplusplus
extern "C"
{
#endif

/***************************** Include Files *********************************/

#include "exception_handler.h"
#include "fmsh_common.h"
#include "fmsh_parameters.h"

/************************** Constant Definitions *****************************/
#define GIC_DEVICE_ID                0U
#define FGicPs_PCELL_ID              0xB105F00DU
/* Canonical definitions for SCU GIC */
#define FPAR_SCUGIC_NUM_INSTANCES    1U
#define FPAR_SCUGIC_SINGLE_DEVICE_ID 0U
#define FPAR_SCUGIC_CPU_BASEADDR     (FPS_GIC_BASEADDR + 0x00020000U)
#define FPAR_SCUGIC_DIST_BASEADDR    (FPS_GIC_BASEADDR + 0x00010000U)
#define FPAR_SCUGIC_ACK_BEFORE       0U

/* Shared Peripheral Interrupts (SPI) */
#define rpu0_perf_mon_INT_ID         40U
#define rpu1_perf_mon_INT_ID         41U
#define ocm_one_bit_err_int_INT_ID   42U
#define lpd_apb_INT_ID               43U
#define RPU0_ECC_INT_ID              44U
#define RPU1_ECC_INT_ID              45U
#define NAND_INT_ID                  46U
#define QSPI_INT_ID                  47U
#define GPIO_INT_ID                  48U
#define I2C0_INT_ID                  49U
#define I2C1_INT_ID                  50U
#define SPI0_INT_ID                  51U
#define SPI1_INT_ID                  52U
#define UART0_INT_ID                 53U
#define UART1_INT_ID                 54U
#define CAN0_INT_ID                  55U
#define CAN1_INT_ID                  56U
#define LPD_APM_INT_ID               57U
#define RTC_Alarm_INT_ID             58U
#define RTC_Seconds_INT_ID           59U
#define ClkMon_INT_ID                60U
#define IPI_Ch7_INT_ID               61U
#define IPI_Ch8_INT_ID               62U
#define IPI_Ch9_INT_ID               63U
#define IPI_Ch10_INT_ID              64U
#define IPI_Ch1_INT_ID               65U
#define IPI_Ch2_INT_ID               66U
#define IPI_Ch0_INT_ID               67U
#define TTC0_1_INT_ID                68U
#define TTC0_2_INT_ID                69U
#define TTC0_3_INT_ID                70U
#define TTC1_1_INT_ID                71U
#define TTC1_2_INT_ID                72U
#define TTC1_3_INT_ID                73U
#define TTC2_1_INT_ID                74U
#define TTC2_2_INT_ID                75U
#define TTC2_3_INT_ID                76U
#define TTC3_1_INT_ID                77U
#define TTC3_2_INT_ID                78U
#define TTC3_3_INT_ID                79U
#define SDIO0_INT_ID                 80U
#define SDIO1_INT_ID                 81U
#define SDIO0_Wakeup_INT_ID          82U
#define SDIO1_Wakeup_INT_ID          83U
#define LPD_SWDT_INT_ID              84U
#define CSU_SWDT_INT_ID              85U
#define LPD_ATB_INT_ID               86U
#define AIB_INT_ID                   87U
#define SysMon_INT_ID                88U
#define GEM0_INT_ID                  89U
#define GEM0_Wakeup_INT_ID           90U
#define GEM1_INT_ID                  91U
#define GEM1_Wakeup_INT_ID           92U
#define GEM2_INT_ID                  93U
#define GEM2_Wakeup_INT_ID           94U
#define GEM3_INT_ID                  95U
#define GEM3_Wakeup_INT_ID           96U
#define USB0_Endpoint0_INT_ID        97U
#define USB0_Endpoint1_INT_ID        98U
#define USB0_INT_ID                  99U
#define USB0_Endpoint3_INT_ID        100U
#define USB0_OTG_INT_ID              101U
#define USB1_Endpoint0_INT_ID        102U
#define USB1_Endpoint1_INT_ID        103U
#define USB1_INT_ID                  104U
#define USB1_Endpoint3_INT_ID        105U
#define USB1_OTG_INT_ID              106U
#define USB0_Wakeup_INT_ID           107U
#define USB1_Wakeup_INT_ID           108U
#define LPD_DMA_CH0_INT_ID           109U
#define LPD_DMA_CH1_INT_ID           110U
#define LPD_DMA_CH2_INT_ID           111U
#define LPD_DMA_CH3_INT_ID           112U
#define LPD_DMA_CH4_INT_ID           113U
#define LPD_DMA_CH5_INT_ID           114U
#define LPD_DMA_CH6_INT_ID           115U
#define LPD_DMA_CH7_INT_ID           116U
#define PCAP_INT_ID                  117U
#define FPD_DMA_CMN_INT_ID           118U
#define LPD_DMA_CMN_INT_ID           119U
#define LPD_MPU_PPU_INT_ID           120U
#define PL0_INT_ID                   121U
#define PL1_INT_ID                   122U
#define PL2_INT_ID                   123U
#define PL3_INT_ID                   124U
#define PL4_INT_ID                   125U
#define PL5_INT_ID                   126U
#define PL6_INT_ID                   127U
#define PL7_INT_ID                   128U
#define ocm_two_bit_err_int_INT_ID   129U
#define DPU_INT_ID                   130U
#define PL8_INT_ID                   136U
#define PL9_INT_ID                   137U
#define PL10_INT_ID                  138U
#define PL11_INT_ID                  139U
#define PL12_INT_ID                  140U
#define PL13_INT_ID                  141U
#define PL14_INT_ID                  142U
#define PL15_INT_ID                  143U
#define DDR_INT_ID                   144U
#define FPD_SWDT_INT_ID              145U
#define PCIe_MSI0_INT_ID             146U
#define PCIe_MSI1_INT_ID             147U
#define PCIe_INTx_INT_ID             148U
#define PCIe_DMA_INT_ID              149U
#define PCIe_MSC_INT_ID              150U
#define DisplayPort_INT_ID           151U
#define FPD_APB_INT_ID               152U
#define FPD_ATB_INT_ID               153U
#define DPU_IRQ_INT_ID               154U
#define FPD_APM_INT_ID               155U
#define FPD_DMA_CH0_INT_ID           156U
#define FPD_DMA_CH1_INT_ID           157U
#define FPD_DMA_CH2_INT_ID           158U
#define FPD_DMA_CH3_INT_ID           159U
#define FPD_DMA_CH4_INT_ID           160U
#define FPD_DMA_CH5_INT_ID           161U
#define FPD_DMA_CH6_INT_ID           162U
#define FPD_DMA_CH7_INT_ID           163U
#define GPU_INT_ID                   164U
#define SATA_INT_ID                  165U
#define FPD_MPU_INT_ID               166U
#define APU0_VCPUMNT_INT_ID          167U
#define APU1_VCPUMNT_INT_ID          168U
#define APU2_VCPUMNT_INT_ID          169U
#define APU3_VCPUMNT_INT_ID          170U
#define CPU0_CTI_INT_ID              171U
#define CPU1_CTI_INT_ID              172U
#define CPU2_CTI_INT_ID              173U
#define CPU3_CTI_INT_ID              174U
#define PMU0_IRQ_INT_ID              175U
#define PMU1_IRQ_INT_ID              176U
#define PMU2_IRQ_INT_ID              177U
#define PMU3_IRQ_INT_ID              178U
#define APU0_Comm_INT_ID             179U
#define APU1_Comm_INT_ID             180U
#define APU2_Comm_INT_ID             181U
#define APU3_Comm_INT_ID             182U
#define L2_Cache_INT_ID              183U
#define APU_ExtError_INT_ID          184U
#define APU_RegError_INT_ID          185U
#define CCI_INT_ID                   186U
#define SMMU_INT_ID                  187U
#define VPU_INT_ID                   188U
#define FPD_TST_CLKMON_INT_ID        189U
#define LPD_TST_CLKMON_INT_ID        190U
/* Private Peripheral Interrupts (PPI) */
#define HYPERVISOR_TMR_INT_ID        26U /* SCU Global Timer interrupt */
#define VIRTUAL_TMR_INT_ID           27U /* SCU Global Timer interrupt */
#define XPS_FIQ_INT_ID               28U /* FIQ from FPGA fabric */
#define SCU_TMR_INT_ID               29U /* SCU Private Timer interrupt */
#define NON_SCU_WDT_INT_ID           30U /* SCU Private WDT interrupt */
#define XPS_IRQ_INT_ID               31U /* IRQ from FPGA fabric */

/*
 * The maximum number of interrupts supported by the hardware.
 */

#define FGicPs_MAX_NUM_INTR_INPUTS \
    192U /* Maximum number of interrupt defined by Zynq */

/*
 * The maximum priority value that can be used in the GIC.
 */
#define FGicPs_MAX_INTR_PRIO_VAL 248U
#define FGicPs_INTR_PRIO_MASK    0x000000F8U

/** @name Distributor Interface Register Map
 *
 * Define the offsets from the base address for all Distributor registers of
 * the interrupt controller, some registers may be reserved in the hardware
 * device.
 * @{
 */
#define FGicPs_DIST_EN_OFFSET                                      \
    0x00000000U                            /**< Distributor Enable \
                       Register */
#define FGicPs_IC_TYPE_OFFSET                                        \
    0x00000004U                            /**< Interrupt Controller \
                       Type Register */
#define FGicPs_DIST_IDENT_OFFSET                               \
    0x00000008U                            /**< Implementor ID \
                       Register */
#define FGicPs_SECURITY_OFFSET                                     \
    0x00000080U                            /**< Interrupt Security \
                       Register */
#define FGicPs_ENABLE_SET_OFFSET                           \
    0x00000100U                            /**< Enable Set \
                       Register */
#define FGicPs_DISABLE_OFFSET 0x00000180U  /**< Enable Clear Register */
#define FGicPs_PENDING_SET_OFFSET                           \
    0x00000200U                            /**< Pending Set \
                       Register */
#define FGicPs_PENDING_CLR_OFFSET                             \
    0x00000280U                            /**< Pending Clear \
                       Register */
#define FGicPs_ACTIVE_OFFSET   0x00000300U /**< Active Status Register */
#define FGicPs_PRIORITY_OFFSET 0x00000400U /**< Priority Level Register */
#define FGicPs_SPI_TARGET_OFFSET                           \
    0x00000800U                            /**< SPI Target \
                       Register 0x800-0x8FB */
#define FGicPs_INT_CFG_OFFSET                                           \
    0x00000C00U                            /**< Interrupt Configuration \
                       Register 0xC00-0xCFC */
#define FGicPs_PPI_STAT_OFFSET 0x00000D00U /**< PPI Status Register */
#define FGicPs_SPI_STAT_OFFSET                                      \
    0x00000D04U                            /**< SPI Status Register \
                       0xd04-0xd7C */
#define FGicPs_AHB_CONFIG_OFFSET                                  \
    0x00000D80U                            /**< AHB Configuration \
                       Register */
#define FGicPs_SFI_TRIG_OFFSET                                     \
    0x00000F00U                            /**< Software Triggered \
                       Interrupt Register */
#define FGicPs_PERPHID_OFFSET 0x00000FD0U  /**< Peripheral ID Reg */
#define FGicPs_PCELLID_OFFSET 0x00000FF0U  /**< Pcell ID Register */
/* @} */

/** @name  Distributor Enable Register
 * Controls if the distributor response to external interrupt inputs.
 * @{
 */
#define FGicPs_EN_INT_MASK    0x00000001U /**< Interrupt In Enable */
/* @} */

/** @name  Interrupt Controller Type Register
 * @{
 */
#define FGicPs_LSPI_MASK                                        \
    0x0000F800U                         /**< Number of Lockable \
                        Shared Peripheral                       \
                        Interrupts*/
#define FGicPs_DOMAIN_MASK  0x00000400U /**< Number os Security domains*/
#define FGicPs_CPU_NUM_MASK 0x000000E0U /**< Number of CPU Interfaces */
#define FGicPs_NUM_INT_MASK 0x0000001FU /**< Number of Interrupt IDs */
/* @} */

/** @name  Implementor ID Register
 * Implementor and revision information.
 * @{
 */
#define FGicPs_REV_MASK     0x00FFF000U /**< Revision Number */
#define FGicPs_IMPL_MASK    0x00000FFFU /**< Implementor */
/* @} */

/** @name  Interrupt Security Registers
 * Each bit controls the security level of an interrupt, either secure or non
 * secure. These registers can only be accessed using secure read and write.
 * There are registers for each of the CPU interfaces at offset 0x080.  A
 * register set for the SPI interrupts is available to all CPU interfaces.
 * There are up to 32 of these registers staring at location 0x084.
 * @{
 */
#define FGicPs_INT_NS_MASK                      \
    0x00000001U /**< Each bit corresponds to an \
INT_ID */
/* @} */

/** @name  Enable Set Register
 * Each bit controls the enabling of an interrupt, a 0 is disabled, a 1 is
 * enabled. Writing a 0 has no effect. Use the ENABLE_CLR register to set a
 * bit to 0.
 * There are registers for each of the CPU interfaces at offset 0x100. With up
 * to 8 registers aliased to the same address. A register set for the SPI
 * interrupts is available to all CPU interfaces.
 * There are up to 32 of these registers staring at location 0x104.
 * @{
 */
#define FGicPs_INT_EN_MASK                      \
    0x00000001U /**< Each bit corresponds to an \
INT_ID */
/* @} */

/** @name  Enable Clear Register
 * Each bit controls the disabling of an interrupt, a 0 is disabled, a 1 is
 * enabled. Writing a 0 has no effect. Writing a 1 disables an interrupt and
 * sets the corresponding bit to 0.
 * There are registers for each of the CPU interfaces at offset 0x180. With up
 * to 8 registers aliased to the same address.
 * A register set for the SPI interrupts is available to all CPU interfaces.
 * There are up to 32 of these registers staring at location 0x184.
 * @{
 */
#define FGicPs_INT_CLR_MASK                     \
    0x00000001U /**< Each bit corresponds to an \
INT_ID */
/* @} */

/** @name  Pending Set Register
 * Each bit controls the Pending or Active and Pending state of an interrupt, a
 * 0 is not pending, a 1 is pending. Writing a 0 has no effect. Writing a 1 sets
 * an interrupt to the pending state.
 * There are registers for each of the CPU interfaces at offset 0x200. With up
 * to 8 registers aliased to the same address.
 * A register set for the SPI interrupts is available to all CPU interfaces.
 * There are up to 32 of these registers staring at location 0x204.
 * @{
 */
#define FGicPs_PEND_SET_MASK                    \
    0x00000001U /**< Each bit corresponds to an \
INT_ID */
/* @} */

/** @name  Pending Clear Register
 * Each bit can clear the Pending or Active and Pending state of an interrupt, a
 * 0 is not pending, a 1 is pending. Writing a 0 has no effect. Writing a 1
 * clears the pending state of an interrupt.
 * There are registers for each of the CPU interfaces at offset 0x280. With up
 * to 8 registers aliased to the same address.
 * A register set for the SPI interrupts is available to all CPU interfaces.
 * There are up to 32 of these registers staring at location 0x284.
 * @{
 */
#define FGicPs_PEND_CLR_MASK                    \
    0x00000001U /**< Each bit corresponds to an \
INT_ID */
/* @} */

/** @name  Active Status Register
 * Each bit provides the Active status of an interrupt, a
 * 0 is not Active, a 1 is Active. This is a read only register.
 * There are registers for each of the CPU interfaces at offset 0x300. With up
 * to 8 registers aliased to each address.
 * A register set for the SPI interrupts is available to all CPU interfaces.
 * There are up to 32 of these registers staring at location 0x380.
 * @{
 */
#define FGicPs_ACTIVE_MASK                      \
    0x00000001U /**< Each bit corresponds to an \
  INT_ID */
/* @} */

/** @name  Priority Level Register
 * Each byte in a Priority Level Register sets the priority level of an
 * interrupt. Reading the register provides the priority level of an interrupt.
 * There are registers for each of the CPU interfaces at offset 0x400 through
 * 0x41C. With up to 8 registers aliased to each address.
 * 0 is highest priority, 0xFF is lowest.
 * A register set for the SPI interrupts is available to all CPU interfaces.
 * There are up to 255 of these registers staring at location 0x420.
 * @{
 */
#define FGicPs_PRIORITY_MASK                     \
    0x000000FFU /**< Each Byte corresponds to an \
INT_ID */
#define FGicPs_PRIORITY_MAX                      \
    0x000000FFU /**< Highest value of a priority \
actually the lowest priority*/
/* @} */

/** @name  SPI Target Register 0x800-0x8FB
 * Each byte references a separate SPI and programs which of the up to 8 CPU
 * interfaces are sent a Pending interrupt.
 * There are registers for each of the CPU interfaces at offset 0x800 through
 * 0x81C. With up to 8 registers aliased to each address.
 * A register set for the SPI interrupts is available to all CPU interfaces.
 * There are up to 255 of these registers staring at location 0x820.
 *
 * This driver does not support multiple CPU interfaces. These are included
 * for complete documentation.
 * @{
 */
#define FGicPs_SPI_CPU7_MASK 0x00000080U /**< CPU 7 Mask*/
#define FGicPs_SPI_CPU6_MASK 0x00000040U /**< CPU 6 Mask*/
#define FGicPs_SPI_CPU5_MASK 0x00000020U /**< CPU 5 Mask*/
#define FGicPs_SPI_CPU4_MASK 0x00000010U /**< CPU 4 Mask*/
#define FGicPs_SPI_CPU3_MASK 0x00000008U /**< CPU 3 Mask*/
#define FGicPs_SPI_CPU2_MASK 0x00000004U /**< CPU 2 Mask*/
#define FGicPs_SPI_CPU1_MASK 0x00000002U /**< CPU 1 Mask*/
#define FGicPs_SPI_CPU0_MASK 0x00000001U /**< CPU 0 Mask*/
/* @} */

/** @name  Interrupt Configuration Register 0xC00-0xCFC
 * The interrupt configuration registers program an SFI to be active HIGH level
 * sensitive or rising edge sensitive.
 * Each bit pair describes the configuration for an INT_ID.
 * SFI    Read Only    b10 always
 * PPI    Read Only    depending on how the PPIs are configured.
 *                    b01    Active HIGH level sensitive
 *                    b11 Rising edge sensitive
 * SPI                LSB is read only.
 *                    b01    Active HIGH level sensitive
 *                    b11 Rising edge sensitive/
 * There are registers for each of the CPU interfaces at offset 0xC00 through
 * 0xC04. With up to 8 registers aliased to each address.
 * A register set for the SPI interrupts is available to all CPU interfaces.
 * There are up to 255 of these registers staring at location 0xC08.
 * @{
 */
#define FGicPs_INT_CFG_MASK  0x00000003U /**< */
/* @} */

/** @name  PPI Status Register
 * Enables an external AMBA master to access the status of the PPI inputs.
 * A CPU can only read the status of its local PPI signals and cannot read the
 * status for other CPUs.
 * This register is aliased for each CPU interface.
 * @{
 */
#define FGicPs_PPI_C15_MASK  0x00008000U /**< PPI Status */
#define FGicPs_PPI_C14_MASK  0x00004000U /**< PPI Status */
#define FGicPs_PPI_C13_MASK  0x00002000U /**< PPI Status */
#define FGicPs_PPI_C12_MASK  0x00001000U /**< PPI Status */
#define FGicPs_PPI_C11_MASK  0x00000800U /**< PPI Status */
#define FGicPs_PPI_C10_MASK  0x00000400U /**< PPI Status */
#define FGicPs_PPI_C09_MASK  0x00000200U /**< PPI Status */
#define FGicPs_PPI_C08_MASK  0x00000100U /**< PPI Status */
#define FGicPs_PPI_C07_MASK  0x00000080U /**< PPI Status */
#define FGicPs_PPI_C06_MASK  0x00000040U /**< PPI Status */
#define FGicPs_PPI_C05_MASK  0x00000020U /**< PPI Status */
#define FGicPs_PPI_C04_MASK  0x00000010U /**< PPI Status */
#define FGicPs_PPI_C03_MASK  0x00000008U /**< PPI Status */
#define FGicPs_PPI_C02_MASK  0x00000004U /**< PPI Status */
#define FGicPs_PPI_C01_MASK  0x00000002U /**< PPI Status */
#define FGicPs_PPI_C00_MASK  0x00000001U /**< PPI Status */
/* @} */

/** @name  SPI Status Register 0xd04-0xd7C
 * Enables an external AMBA master to access the status of the SPI inputs.
 * There are up to 63 registers if the maximum number of SPI inputs are
 * configured.
 * @{
 */
#define FGicPs_SPI_N_MASK                           \
    0x00000001U /**< Each bit corresponds to an SPI \
input */
/* @} */

/** @name  AHB Configuration Register
 * Provides the status of the CFGBIGEND input signal and allows the endianess
 * of the GIC to be set.
 * @{
 */
#define FGicPs_AHB_END_MASK                                                 \
    0x00000004U                              /**< 0-GIC uses little Endian, \
                                              1-GIC uses Big Endian */
#define FGicPs_AHB_ENDOVR_MASK                                              \
    0x00000002U                              /**< 0-Uses CFGBIGEND control, \
                                              1-use the AHB_END bit */
#define FGicPs_AHB_TIE_OFF_MASK  0x00000001U /**< State of CFGBIGEND */

/* @} */

/** @name  Software Triggered Interrupt Register
 * Controls issueing of software interrupts.
 * @{
 */
#define FGicPs_SFI_SELFTRIG_MASK 0x02010000U
#define FGicPs_SFI_TRIG_TRGFILT_MASK                                            \
    0x03000000U                              /**< Target List filter            \
                                                  b00-Use the target List       \
                                                  b01-All CPUs except requester \
                                                  b10-To Requester              \
                                                  b11-reserved */
#define FGicPs_SFI_TRIG_CPU_MASK 0x00FF0000U /**< CPU Target list */
#define FGicPs_SFI_TRIG_SATT_MASK                                           \
    0x00008000U                              /**< 0= Use a secure interrupt \
                                              */
#define FGicPs_SFI_TRIG_INTID_MASK                                 \
    0x0000000FU                              /**< Set to the INTID \
                                                  signaled to the CPU*/
/* @} */

/** @name CPU Interface Register Map
 *
 * Define the offsets from the base address for all CPU registers of the
 * interrupt controller, some registers may be reserved in the hardware device.
 * @{
 */
#define FGicPs_CONTROL_OFFSET                                          \
    0x00000000U                             /**< CPU Interface Control \
                        Register */
#define FGicPs_CPU_PRIOR_OFFSET 0x00000004U /**< Priority Mask Reg */
#define FGicPs_BIN_PT_OFFSET    0x00000008U /**< Binary Point Register */
#define FGicPs_INT_ACK_OFFSET   0x0000000CU /**< Interrupt ACK Reg */
#define FGicPs_EOI_OFFSET       0x00000010U /**< End of Interrupt Reg */
#define FGicPs_RUN_PRIOR_OFFSET 0x00000014U /**< Running Priority Reg */
#define FGicPs_HI_PEND_OFFSET                                              \
    0x00000018U                             /**< Highest Pending Interrupt \
                        Register */
#define FGicPs_ALIAS_BIN_PT_OFFSET                                  \
    0x0000001CU                             /**< Aliased non-Secure \
                            Binary Point Register */

/**<  0x00000020 to 0x00000FBC are reserved and should not be read or written
 * to. */
/* @} */

/** @name Control Register
 * CPU Interface Control register definitions
 * All bits are defined here although some are not available in the non-secure
 * mode.
 * @{
 */
#define FGicPs_CNTR_SBPR_MASK               \
    0x00000010U /**< Secure Binary Pointer, \
                  0=separate registers,     \
                  1=both use bin_pt_s */
#define FGicPs_CNTR_FIQEN_MASK             \
    0x00000008U /**< Use nFIQ_C for secure \
                   interrupts,             \
                   0= use IRQ for both,    \
                   1=Use FIQ for secure, IRQ for non*/
#define FGicPs_CNTR_ACKCTL_MASK \
    0x00000004U /**< Ack control for secure or non secure */
#define FGicPs_CNTR_EN_NS_MASK 0x00000002U /**< Non Secure enable */
#define FGicPs_CNTR_EN_S_MASK \
    0x00000001U /**< Secure enable, 0=Disabled, 1=Enabled */
/* @} */

/** @name Priority Mask Register
 * Priority Mask register definitions
 * The CPU interface does not send interrupt if the level of the interrupt is
 * lower than the level of the register.
 * @{
 */
/*#define FGicPs_PRIORITY_MASK		0x000000FFU*/ /**< All interrupts */
/* @} */

/** @name Binary Point Register
 * Binary Point register definitions
 * @{
 */

#define FGicPs_BIN_PT_MASK                   \
    0x00000007U /**< Binary point mask value \
Value  Secure  Non-secure                    \
b000    0xFE    0xFF                         \
b001    0xFC    0xFE                         \
b010    0xF8    0xFC                         \
b011    0xF0    0xF8                         \
b100    0xE0    0xF0                         \
b101    0xC0    0xE0                         \
b110    0x80    0xC0                         \
b111    0x00    0x80                         \
*/
/*@}*/

/** @name Interrupt Acknowledge Register
 * Interrupt Acknowledge register definitions
 * Identifies the current Pending interrupt, and the CPU ID for software
 * interrupts.
 */
#define FGicPs_ACK_INTID_MASK    0x000003FFU /**< Interrupt ID */
#define FGicPs_CPUID_MASK        0x00000C00U /**< CPU ID */
/* @} */

/** @name End of Interrupt Register
 * End of Interrupt register definitions
 * Allows the CPU to signal the GIC when it completes an interrupt service
 * routine.
 */
#define FGicPs_EOI_INTID_MASK    0x000003FFU /**< Interrupt ID */

/* @} */

/** @name Running Priority Register
 * Running Priority register definitions
 * Identifies the interrupt priority level of the highest priority active
 * interrupt.
 */
#define FGicPs_RUN_PRIORITY_MASK 0x000000FFU /**< Interrupt Priority */
/* @} */

/*
 * Highest Pending Interrupt register definitions
 * Identifies the interrupt priority of the highest priority pending interupt
 */
#define FGicPs_PEND_INTID_MASK   0x000003FFU   /**< Pending Interrupt ID */
/*#define FGicPs_CPUID_MASK		0x00000C00U */ /**< CPU ID */
                                               /* @} */

/***************** Macros (Inline Functions) Definitions *********************/
/* Read the Interrupt Group Register offset for an interrupt id.*/

#define FGicPs_SECURITY_OFFSET_CALC(InterruptID) \
    ((u32)FGicPs_SECURITY_OFFSET + (((InterruptID) / 32U) * 4U))
/****************************************************************************/
/**
 *
 * Read the Interrupt Configuration Register offset for an interrupt id.
 *
 * @param	InterruptID is the interrupt number.
 *
 * @return	The 32-bit value of the offset
 *
 * @note
 *
 *****************************************************************************/
#define FGicPs_INT_CFG_OFFSET_CALC(InterruptID) \
    ((u32)FGicPs_INT_CFG_OFFSET + (((InterruptID) / 16U) * 4U))

/****************************************************************************/
/**
 *
 * Read the Interrupt Priority Register offset for an interrupt id.
 *
 * @param	InterruptID is the interrupt number.
 *
 * @return	The 32-bit value of the offset
 *
 * @note
 *
 *****************************************************************************/
#define FGicPs_PRIORITY_OFFSET_CALC(InterruptID) \
    ((u32)FGicPs_PRIORITY_OFFSET + (((InterruptID) / 4U) * 4U))

/****************************************************************************/
/**
 *
 * Read the SPI Target Register offset for an interrupt id.
 *
 * @param	InterruptID is the interrupt number.
 *
 * @return	The 32-bit value of the offset
 *
 * @note
 *
 *****************************************************************************/
#define FGicPs_SPI_TARGET_OFFSET_CALC(InterruptID) \
    ((u32)FGicPs_SPI_TARGET_OFFSET + (((InterruptID) / 4U) * 4U))

/****************************************************************************/
/**
 *
 * Read the Interrupt Clear-Enable Register offset for an interrupt ID
 *
 * @param	Register is the register offset for the clear/enable bank.
 * @param	InterruptID is the interrupt number.
 *
 * @return	The 32-bit value of the offset
 *
 * @note
 *
 *****************************************************************************/
#define FGicPs_EN_DIS_OFFSET_CALC(Register, InterruptID) \
    ((Register) + (((InterruptID) / 32U) * 4U))

/****************************************************************************/
/**
 *
 * Read the given Intc register.
 *
 * @param	BaseAddress is the base address of the device.
 * @param	RegOffset is the register offset to be read
 *
 * @return	The 32-bit value of the register
 *
 * @note
 * C-style signature:
 *    u32 FGicPs_ReadReg(u32 BaseAddress, u32 RegOffset)
 *
 *****************************************************************************/
#define FGicPs_ReadReg(BaseAddress, RegOffset) \
    (FMSH_In32((BaseAddress) + (RegOffset)))

/****************************************************************************/
/**
 *
 * Write the given Intc register.
 *
 * @param	BaseAddress is the base address of the device.
 * @param	RegOffset is the register offset to be written
 * @param	Data is the 32-bit value to write to the register
 *
 * @return	None.
 *
 * @note
 * C-style signature:
 *    void FGicPs_WriteReg(u32 BaseAddress, u32 RegOffset, u32 Data)
 *
 *****************************************************************************/
#define FGicPs_WriteReg(BaseAddress, RegOffset, Data) \
    (FMSH_Out32(((BaseAddress) + (RegOffset)), ((u32)(Data))))

/****************************************************************************/
/**
 *
 * Enable specific interrupt(s) in the interrupt controller.
 *
 * @param	DistBaseAddress is the Distributor Register base address of the
 *		device
 * @param	Int_Id is the ID of the interrupt source and should be in the
 *		range of 0 to FGicPs_MAX_NUM_INTR_INPUTS - 1
 *
 * @return	None.
 *
 * @note		C-style signature:
 *		void FGicPs_EnableIntr(u32 DistBaseAddress, u32 Int_Id)
 *
 *****************************************************************************/
#define FGicPs_EnableIntr(DistBaseAddress, Int_Id)                      \
    FGicPs_WriteReg((DistBaseAddress),                                  \
                    FGicPs_ENABLE_SET_OFFSET + (((Int_Id) / 32U) * 4U), \
                    (0x00000001U << ((Int_Id) % 32U)))

/****************************************************************************/
/**
 *
 * Disable specific interrupt(s) in the interrupt controller.
 *
 * @param	DistBaseAddress is the Distributor Register base address of the
 *		device
 * @param	Int_Id is the ID of the interrupt source and should be in the
 *		range of 0 to FGicPs_MAX_NUM_INTR_INPUTS - 1
 *
 *
 * @return	None.
 *
 * @note		C-style signature:
 *		void FGicPs_DisableIntr(u32 DistBaseAddress, u32 Int_Id)
 *
 *****************************************************************************/
#define FGicPs_DisableIntr(DistBaseAddress, Int_Id)                  \
    FGicPs_WriteReg((DistBaseAddress),                               \
                    FGicPs_DISABLE_OFFSET + (((Int_Id) / 32U) * 4U), \
                    (0x00000001U << ((Int_Id) % 32U)))

/************************** Function Prototypes ******************************/

void FGicPs_DeviceInterruptHandler(void *DeviceId);
s32 FGicPs_DeviceInitialize(u32 DeviceId);
void FGicPs_RegisterHandler(u32 BaseAddress, s32 InterruptID,
                            FMSH_InterruptHandler Handler, void *CallBackRef);
void FGicPs_SetPriTrigTypeByDistAddr(u32 DistBaseAddress, u32 Int_Id,
                                     u8 Priority, u8 Trigger);
void FGicPs_GetPriTrigTypeByDistAddr(u32 DistBaseAddress, u32 Int_Id,
                                     u8 *Priority, u8 *Trigger);
/************************** Variable Definitions *****************************/
#ifdef __cplusplus
}
#endif

#endif /* end of protection macro */
/** @} */
