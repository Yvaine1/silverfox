/*****************************************************************************/
/**
 *
 * @file main.c
 *
 * This is the main file which contains code for the FSBL.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date        Changes
 * ----- ---- -------- -------------------------------------------------------
 * 6.00   lq   23/12/22 The first release project.
 * 6.01   lq   24/12/12 The fix DUMMY_CLOCK_COUNT.
 * </pre>
 *
 * @note
 *
 ******************************************************************************/

/***************************** Include Files *********************************/
#include "boot_main.h"

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/
int EL3_main (void) { return 0; }

int EL2_main (void) { return 0; }

int EL1_main (void) { return 0; }

int EL0_main (void) { return 0; }
/************************** Variable Definitions *****************************/
extern u32 FlashReadBaseAddress;
u64 gtc_count0, gtc_count1 = 0U;
double gtc_time = 0.0;
BootPs BootInstance = {6.01, 0x100U};

extern FDevcPs_T g_DEVC;


/*****************************************************************************/
/** This is the FSBL main function and is implemented stage wise.
 *
 * @param	None
 *
 * @return	None
 *
 *****************************************************************************/
int main (void)
{
#if DEBUG_PERF
    global_timer_enable();
    gtc_count0 = *(u64 *)(FPS_GTC_BASEADDR + 0x8);
#endif

    /**
     * Local variables
     */
    u32 BootStatus = FMSH_SUCCESS;
    u32 BootStage = BOOT_STAGE1;
    u32 PartitionNum = 0U;
    u32 EarlyHandoff = FALSE;

    while (BootStage <= BOOT_STAGE_DEFAULT)
    {
        switch (BootStage)
        {
        case BOOT_STAGE1:
        {
            /* Initialize the system*/
            BootStatus = FmshFsbl_BootInitialize(&BootInstance);
            if (BootStatus != FMSH_SUCCESS)
            {
                BootInstance.ErrorCode=BootStatus;
                BootStage = BOOT_STAGE_ERR;
            }
            else
            {
                BootStage = BOOT_STAGE2;
            }
        }
        break;

        case BOOT_STAGE2:
        {
            UART_LOG_OUT(DEBUG_INFO, "======= In BootStage 2 ======= \r\n");
            /**
             *  boot device
             *  DeviceOps
             *  image header
             */
            BootStatus = FmshFsbl_BootDeviceInitAndValidate(&BootInstance);
            if ((FMSH_SUCCESS != BootStatus) &&
                (BOOT_STATUS_JTAG != BootStatus))
            {
                UART_LOG_OUT(
                    DEBUG_INFO,
                    "Boot Device Initialization and validate failed!!!\r\n");
                BootStage = BOOT_STAGE_ERR;
            }
            else if (BOOT_STATUS_JTAG == BootStatus)
            {
                /**
                 * This is JTAG boot mode, go to the handoff BootStage
                 */
                BootStage = BOOT_STAGE4;
            }
            else
            {
                /**
                 * Start the partition loading from 1
                 * 0th partition will be FSBL
                 */
                PartitionNum = 0x1U;

                if (BootInstance.ImageHeader.ImageHeaderTable.NoOfPartitions ==
                    1)
                {
                    UART_LOG_OUT(DEBUG_INFO,
                                 "None valid Partition in the BOOT.bin!! \n\r");
                    BootStage = BOOT_STAGE4;
                }
                else
                {
                    BootStage = BOOT_STAGE3;
                }
            }
        }
        break;

        case BOOT_STAGE3:
        {
            UART_LOG_OUT(DEBUG_INFO, "======= In BootStage 3 ======= \r\n");

            /**
             * Load the partitions
             *  image header
             *  partition header
             *  partition parameters
             */
            BootStatus = FmshFsbl_PartitionLoad(&BootInstance, PartitionNum);
            if( (FMSH_SUCCESS != BootStatus) && (PARTITION_SKIP_LOAD != BootStatus) )
            {
                /* Error*/
                UART_LOG_OUT(DEBUG_INFO,
                             "Partition  Load Failed,PartitionNum=0x%d!!\r\n",
                             PartitionNum);
                BootStage = BOOT_STAGE_ERR;
            }
            else
            {
                UART_LOG_OUT(DEBUG_INFO, "Partition Load Success \r\n");

                FmshFsbl_MarkUsedRPUCores(&BootInstance, PartitionNum);
                /**
                 * Check loading all partitions is completed
                 */
                BootStatus = FmshFsbl_CheckEarlyHandoff(&BootInstance,
                                                        PartitionNum);
                if (PartitionNum <
                    (BootInstance.ImageHeader.ImageHeaderTable.NoOfPartitions -
                     1U))
                {
                    if (TRUE == BootStatus)
                    {
                        EarlyHandoff = TRUE;
                        BootStatus = BOOT_STAGE4;
                    }
                    else
                    {
                        /**
                         * No need to change the Fsbl Stage
                         * Load the next partition
                         */
                        PartitionNum++;
                    }
                }
                else
                {
                    /**
                     * No more partitions present, go to handoff stage
                     */
                    UART_LOG_OUT(DEBUG_INFO, "All Partitions Loaded \n\r");
                    BootStage = BOOT_STAGE4;
                }
            } /* End of else loop for Load Success */
        }
        break;

        case BOOT_STAGE4:
        {
            UART_LOG_OUT(DEBUG_INFO,
                         "================= In BootStage 4 ============ \r\n");
            /**
             * Handoff to the applications
             * Handoff address
             * xip
             * ps7 post config
             */
            UART_LOG_OUT(DEBUG_INFO,
                         "Handoff control to application...... \r\n");
            BootStatus = BootHandoff(&BootInstance, PartitionNum, EarlyHandoff);

            if (FSBL_STATUS_CONTINUE_PARTITION_LOAD == BootStatus)
            {
                UART_LOG_OUT(DEBUG_INFO,
                             "Early handoff to a application complete \n\r");
                UART_LOG_OUT(DEBUG_INFO,
                             "Continuing to load remaining partitions \n\r");

                PartitionNum++;
                BootStage = BOOT_STAGE3;
            }
            else if (FSBL_STATUS_CONTINUE_OTHER_HANDOFF == BootStatus)
            {
                UART_LOG_OUT(DEBUG_INFO,
                             "Early handoff to a application complete \n\r");
                UART_LOG_OUT(DEBUG_INFO,
                             "Continuing handoff to other applications, if "
                             "present \n\r");
                EarlyHandoff = FALSE;
            }
            else if (FMSH_SUCCESS != BootStatus)
            {
                /**
                 * Error
                 */
                UART_LOG_OUT(DEBUG_INFO, "Handoff Failed 0x%0lx\n\r",
                             BootStatus);
                BootStage = BOOT_STAGE_ERR;
            }
            else
            {
                /**
                 * we should never be here
                 */
                BootStage = BOOT_STAGE_DEFAULT;
            }
        }
        break;

        case BOOT_STAGE_ERR:
        {
            UART_LOG_OUT(
                DEBUG_INFO,
                "================= In BootStage Error ============ \r\n");
            ErrorLockDown();
            BootStage = BOOT_STAGE_DEFAULT;
        }
        break;

        case BOOT_STAGE_DEFAULT:
        default:
        {
            /**
             * we should never be here
             */
            UART_LOG_OUT(DEBUG_INFO,
                         "In default BootStage: We should never be here \r\n");
            /* Exit FSBL*/
            DefaultHandoffExit();
        }
        break;

        } /* End of switch(FsblBootStage) */
        if (BootStage == BOOT_STAGE_DEFAULT)
        {
            break;
        }
    } /* End of while() */

    /**
     * We should never be here
     */
    UART_LOG_OUT(DEBUG_INFO,
                 "In default BootStage: We should never be here \r\n");
    /**
     * Exit FSBL
     */
    DefaultHandoffExit();

    return 0;
}
