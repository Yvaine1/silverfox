/*****************************************************************************/
/**
 *
 * @file cortexa53.h
 *
 * This header file contains definitions for using inline assembler code. It is
 * written specifically for the GNU compiler.
 *
 * All of the ARM Cortex A53 GPRs, SPRs, and Debug Registers are defined along
 * with the positions of the bits within the registers.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who      Date     Changes
 * ----- -------- -------- -----------------------------------------------
 *
 * </pre>
 *
 ******************************************************************************/
#ifndef _CORTEXA53_H_
#define _CORTEXA53_H_

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

/* Current Processor Status Register (CPSR) Bits */
#define ARM_MODE_BITS                  0x1F
#define ARM_EL3h_MODE                  0xD
#define ARM_EL3t_MODE                  0xC
#define ARM_EL2h_MODE                  0x9
#define ARM_EL2t_MODE                  0x8
#define ARM_EL1h_MODE                  0x5
#define ARM_EL1t_MODE                  0x4
#define ARM_EL0t_MODE                  0x0

#define I_BIT                          0x80
#define F_BIT                          0x40

#define N_BIT                          0x80000000U
#define Z_BIT                          0x40000000U
#define C_BIT                          0x20000000U
#define V_BIT                          0x10000000U

/* FPSID bits */
#define FPSID_IMPLEMENTER_BIT          (24U)
#define FPSID_IMPLEMENTER_MASK         (0x000000FFU << FPSID_IMPLEMENTER_BIT)
#define FPSID_SOFTWARE                 (0X00000001U << 23U)
#define FPSID_ARCH_BIT                 (16U)
#define FPSID_ARCH_MASK                (0x0000000FU << FPSID_ARCH_BIT)
#define FPSID_PART_BIT                 (8U)
#define FPSID_PART_MASK                (0x000000FFU << FPSID_PART_BIT)
#define FPSID_VARIANT_BIT              (4U)
#define FPSID_VARIANT_MASK             (0x0000000FU << FPSID_VARIANT_BIT)
#define FPSID_REV_BIT                  (0U)
#define FPSID_REV_MASK                 (0x0000000FU << FPSID_REV_BIT)

/* FPSCR bits */
#define FPSCR_N_BIT                    (0X00000001U << 31U)
#define FPSCR_Z_BIT                    (0X00000001U << 30U)
#define FPSCR_C_BIT                    (0X00000001U << 29U)
#define FPSCR_V_BIT                    (0X00000001U << 28U)
#define FPSCR_QC                       (0X00000001U << 27U)
#define FPSCR_AHP                      (0X00000001U << 26U)
#define FPSCR_DEFAULT_NAN              (0X00000001U << 25U)
#define FPSCR_FLUSHTOZERO              (0X00000001U << 24U)
#define FPSCR_ROUND_NEAREST            (0X00000000U << 22U)
#define FPSCR_ROUND_PLUSINF            (0X00000001U << 22U)
#define FPSCR_ROUND_MINUSINF           (0X00000002U << 22U)
#define FPSCR_ROUND_TOZERO             (0X00000003U << 22U)
#define FPSCR_RMODE_BIT                (22U)
#define FPSCR_RMODE_MASK               (0X00000003U << FPSCR_RMODE_BIT)
#define FPSCR_STRIDE_BIT               (20U)
#define FPSCR_STRIDE_MASK              (0X00000003U << FPSCR_STRIDE_BIT)
#define FPSCR_LENGTH_BIT               (16U)
#define FPSCR_LENGTH_MASK              (0X00000007U << FPSCR_LENGTH_BIT)
#define FPSCR_IDC                      (0X00000001U << 7U)
#define FPSCR_IXC                      (0X00000001U << 4U)
#define FPSCR_UFC                      (0X00000001U << 3U)
#define FPSCR_OFC                      (0X00000001U << 2U)
#define FPSCR_DZC                      (0X00000001U << 1U)
#define FPSCR_IOC                      (0X00000001U << 0U)

/* MVFR0 bits */
#define MVFR0_RMODE_BIT                (28U)
#define MVFR0_RMODE_MASK               (0x0000000FU << XREG_MVFR0_RMODE_BIT)
#define MVFR0_SHORT_VEC_BIT            (24U)
#define MVFR0_SHORT_VEC_MASK           (0x0000000FU << XREG_MVFR0_SHORT_VEC_BIT)
#define MVFR0_SQRT_BIT                 (20U)
#define MVFR0_SQRT_MASK                (0x0000000FU << XREG_MVFR0_SQRT_BIT)
#define MVFR0_DIVIDE_BIT               (16U)
#define MVFR0_DIVIDE_MASK              (0x0000000FU << XREG_MVFR0_DIVIDE_BIT)
#define MVFR0_EXEC_TRAP_BIT            (0X00000012U)
#define MVFR0_EXEC_TRAP_MASK           (0X0000000FU << XREG_MVFR0_EXEC_TRAP_BIT)
#define MVFR0_DP_BIT                   (8U)
#define MVFR0_DP_MASK                  (0x0000000FU << XREG_MVFR0_DP_BIT)
#define MVFR0_SP_BIT                   (4U)
#define MVFR0_SP_MASK                  (0x0000000FU << XREG_MVFR0_SP_BIT)
#define MVFR0_A_SIMD_BIT               (0U)
#define MVFR0_A_SIMD_MASK              (0x0000000FU << MVFR0_A_SIMD_BIT)

/* FPEXC bits */
#define FPEXC_EX                       (0X00000001U << 31U)
#define FPEXC_EN                       (0X00000001U << 30U)
#define FPEXC_DEX                      (0X00000001U << 29U)

/* SCTLR */
#define CONTROL_MMU_BIT                (0X00000001U << 1U)
#define CONTROL_DCACHE_BIT             (0X00000001U << 2U)
#define CONTROL_ICACHE_BIT             (0X00000001U << 12U)

#define L1_DATA_PREFETCH_CONTROL_MASK  0xE000
#define L1_DATA_PREFETCH_CONTROL_SHIFT 13

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* XREG_CORTEXA53_H */
