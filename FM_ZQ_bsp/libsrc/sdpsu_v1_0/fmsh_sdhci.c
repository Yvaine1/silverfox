
#include <stdlib.h>
#include <string.h>

#include "fmsh_common.h"
#include "fmsh_sdhci.h"
#include "fmsh_sdhci_card.h"
#include "fmsh_sdhci_hw.h"

static struct sdmmc_usercfg default_cfg = {
    .flags = 0,  // SDMMC_F_UHS_SUPPORT,
                 //  SDMMC_F_USE_PRESET_VALUE | SDMMC_F_HOST_VERSION_4,
#if !NO_OS
    .dma = SDMMC_USE_ADMA64,
#else
    .dma = SDMMC_USE_ADMA,
#endif  
    .sdclk_max = 50000000,
};

/*****************************************************************************
 * This is a stub for the status callback. The stub is here in case the upper
 * layers forget to set the handler.
 *
 * @param	CallBackRef is a pointer to the upper layer callback reference
 * @param	StatusEvent is the event that just occurred.
 * @param	ByteCount is the number of bytes transferred up until the event
 *		occurred.
 *
 * @return	None.
 *
 * @note		None.
 *
 ******************************************************************************/
static void StubStatusHandler(void *callBackRef, u32 statusEvent,
                              unsigned byteCount);

/*****************************************************************************
 * This function is used set initial register value before initialization.
 *
 * @param
 *
 * @return
 *		- 0 if success.
 *		- other value if fail.
 *
 * @note
 *
 ******************************************************************************/
static int FSdPsu_Host_Initf(FSdPsu_T *sdPtr);

static void FSdPsu_Host_Response(FSdPsu_T *sdPtr, u32 *resp);

/****************************************************
 * Transfer 1 block with no-dma
 *****************************************************/
static void FSdPsu_Host_PIOTransfer(FSdPsu_T *sdPtr, struct sdmmc_data *data);

/****************************************************
 * prepare data for dma transfer
 *****************************************************/
static void FSdPsu_Host_SetupADMADescTbl(FSdPsu_T *sdPtr,
                                         struct sdmmc_data *data,
                                         struct sdhci_adma_desc *desc);

/****************************************************
 * prepare dma
 *****************************************************/
static int FSdPsu_Host_PrepareDma(FSdPsu_T *sdPtr, struct sdmmc_data *data);

static int FSdPsu_Host_TransferData(FSdPsu_T *sdPtr, struct sdmmc_data *data);

/*****************************************************************************/
static void StubStatusHandler (void *callBackRef, u32 statusEvent,
                               unsigned byteCount)
{
    (void)callBackRef;
    (void)statusEvent;
    (void)byteCount;
}

int FSdPsu_CfgInitialize (FSdPsu_T *sdPtr, FSdPsu_Config_T *configPtr)
{
    struct sdmmc_card *card;
    struct sdhci_adma_desc *desc;

    FMSH_ASSERT(sdPtr != NULL);
    FMSH_ASSERT(configPtr != NULL);

    /* set default value */
    sdPtr->config.device_id = configPtr->device_id;
    sdPtr->config.card_type = configPtr->card_type;
    sdPtr->config.base = configPtr->base;
    sdPtr->config.input_clock_hz = configPtr->input_clock_hz;
    sdPtr->config.init_freq = configPtr->init_freq;
    sdPtr->config.bus_width = configPtr->bus_width;
    sdPtr->config.has_cd = configPtr->has_cd;
    sdPtr->config.has_wp = configPtr->has_wp;
    sdPtr->config.has_buspwr = configPtr->has_buspwr;
    sdPtr->config.is_cache_coherent = configPtr->is_cache_coherent;

    sdPtr->flags = 0;
    sdPtr->is_inited = 0;

    card = malloc(sizeof(struct sdmmc_card));
    if (card == NULL)
    {
        return FMSH_ENOMEM;
    }
    sdPtr->card = card;

    desc = malloc(sizeof(struct sdhci_adma_desc));
    if (desc == NULL)
    {
        return FMSH_ENOMEM;
    }
    sdPtr->desc = desc;

    sdPtr->statusHandler = StubStatusHandler;

    return FMSH_SUCCESS;
}

void FSdPsu_SetStatusHandler (FSdPsu_T *sdPtr, void *callBackRef,
                              FSdPsu_StatusHandler funcPtr)
{
    FMSH_ASSERT(sdPtr != NULL);
    FMSH_ASSERT(funcPtr != NULL);

    sdPtr->statusHandler = funcPtr;
    sdPtr->statusRef = callBackRef;
}

int FSdPsu_SelfTest (FSdPsu_T *sdPtr)
{
    u32 value;

    value = FMSH_ReadReg(sdPtr->config.base, SDHCI_HRS31);
    if (value != SDHCI_HOST_VERSION)
    {
        return FMSH_EIO;
    }

    FMSH_WriteReg(sdPtr->config.base, SDHCI_SRS00, 0xaa995566);
    value = FMSH_ReadReg(sdPtr->config.base, SDHCI_SRS00);
    if (value != 0xaa995566)
    {
        return FMSH_EIO;
    }

    return FMSH_SUCCESS;
}

void FSdPsu_InterruptHandler (void *instancePtr)
{
    u32 status;
    FSdPsu_T *sdPtr;

    FMSH_ASSERT(instancePtr != NULL);

    sdPtr = (FSdPsu_T *)instancePtr;

    status = FMSH_ReadReg(sdPtr->config.base, SDHCI_SRS12);
    // clear interrupt status(W1C)
    FMSH_WriteReg(sdPtr->config.base, SDHCI_SRS12, status);

    sdPtr->host.int_status |= status;

    sdPtr->statusHandler(sdPtr, status, 0);
}

/************************** Host Controller Interface ************************/
static int FSdPsu_Host_Initf (FSdPsu_T *sdPtr)
{
    u32 value;
    FSdPsu_UserCfg_T *usercfg;

    usercfg = sdPtr->usercfg;

    (void)FSdPsu_Host_MuxIOs(sdPtr, sdPtr->config.card_type);
    delay_us(1);

    // dp should be chosen to obtain 20ms
    FMSH_WriteReg(sdPtr->config.base, SDHCI_HRS01,
                  SDHCI_DP_MS(sdPtr->config.input_clock_hz, 20));
    FMSH_WriteReg(sdPtr->config.base, SDHCI_HRS02, 0x0);
    FMSH_WriteReg(sdPtr->config.base, SDHCI_HRS13, 0xf);
    FMSH_WriteReg(sdPtr->config.base, SDHCI_SRS14, 0x0);

    // host ctrl2
    value = 0;
    if (usercfg->flags & SDMMC_F_USE_PRESET_VALUE)
    {
        value |= SDHCI_SRS15_PVE;
    }
    if (usercfg->flags & SDMMC_F_HOST_VERSION_4)
    {
        value |= SDHCI_SRS15_HV4E;
    }
    FMSH_WriteReg(sdPtr->config.base, SDHCI_SRS15, value);

    return FMSH_SUCCESS;
}

int FSdPsu_Host_CheckDAT (FSdPsu_T *sdPtr, int pattern, int timeout_us)
{
    u32 value, mask, target = 0;

    switch (pattern)
    {
    case SDMMC_DAT_xxx0:
        mask = SDHCI_DATSL_0;
        break;
    case SDMMC_DAT_xxx1:
        target = mask = SDHCI_DATSL_0;
        break;
    case SDMMC_DAT_0000:
        mask = SDHCI_DATSL_0 | SDHCI_DATSL_1 | SDHCI_DATSL_2 | SDHCI_DATSL_3;
        break;
    case SDMMC_DAT_1111:
        target = mask = SDHCI_DATSL_0 | SDHCI_DATSL_1 | SDHCI_DATSL_2 |
                        SDHCI_DATSL_3;
        break;
    default:
        return FMSH_EINVAL;
    }

    // poll dat line
    while (1)
    {
        value = FMSH_ReadReg(sdPtr->config.base, SDHCI_SRS09);
        if ((value & mask) == target)
        {
            break;
        }

        timeout_us--;
        delay_us(1);
        if (timeout_us <= 0)
        {
            return FMSH_ETIME;
        }
    }

    return FMSH_SUCCESS;
}

int FSdPsu_Host_MuxIOs (FSdPsu_T *sdPtr, int type)
{
    u32 sel, cd_ctrl;

    sel = FMSH_ReadReg(0xff180000, 0x0310);
    cd_ctrl = FMSH_ReadReg(0xff180000, 0x035c);
    // type0 is sd, io4~7 is used to drive level shifter
    if (type == SDMMC_TYPE_MMC)
    {
        if (sdPtr->config.device_id == 0)
        {
            sel |= 0x1;
            cd_ctrl |= 0x1;
        }
        else
        {
            sel |= 0x8000;
            cd_ctrl |= 0x10000;
        }
    }
    else
    {
        if (sdPtr->config.device_id == 0)
        {
            sel &= ~0x1;
            cd_ctrl &= ~0x1;
        }
        else
        {
            sel &= ~0x8000;
            cd_ctrl &= ~0x10000;
        }
    }
    FMSH_WriteReg(0xff180000, 0x0310, sel);
    FMSH_WriteReg(0xff180000, 0x035c, cd_ctrl);

    return 0;
}

void FSdPsu_Reset (FSdPsu_T *sdPtr)
{
    u8 value;

    // write slcr reset
    value = FMSH_ReadReg(0xff5e0000, 0x238);
    if (sdPtr->config.device_id == 0)
    {
        FMSH_WriteReg(0xff5e0000, 0x238, value | 0x20);
        delay_ms(1);
        FMSH_WriteReg(0xff5e0000, 0x238, value & ~0x20);
    }
    else
    {
        FMSH_WriteReg(0xff5e0000, 0x238, value | 0x40);
        delay_ms(1);
        FMSH_WriteReg(0xff5e0000, 0x238, value & ~0x40);
    }
}

int FSdPsu_Host_Reset (FSdPsu_T *sdPtr)
{
    u8 value;
    int timeout = 100;

    // write software reset
    FMSH_WriteReg(sdPtr->config.base, SDHCI_HRS00, SDHCI_HRS00_SOFTRST);

    // wait reset bit become 0
    while (1)
    {
        value = FMSH_ReadReg(sdPtr->config.base, SDHCI_HRS00);
        if ((value & SDHCI_HRS00_SOFTRST) == 0)
        {
            break;
        }

        timeout--;
        if (timeout <= 0)
        {
            return FMSH_ETIME;
        }
        delay_ms(1);
    };

    return FMSH_SUCCESS;
}

int FSdPsu_Host_SoftReset (FSdPsu_T *sdPtr, u32 mask)
{
    u32 value;
    int timeout = 100;

    value = FMSH_ReadReg(sdPtr->config.base, SDHCI_SRS11);
    FMSH_WriteReg(sdPtr->config.base, SDHCI_SRS11, value | mask);

    // wait reset bit become 0
    while (1)
    {
        value = FMSH_ReadReg(sdPtr->config.base, SDHCI_SRS11);
        if ((value & mask) == 0)
        {
            break;
        }

        timeout--;
        if (timeout <= 0)
        {
            return FMSH_ETIME;
        }
        delay_ms(1);
    };

    return FMSH_SUCCESS;
}

int FSdPsu_Host_CardDetect (FSdPsu_T *sdPtr)
{
    int retry = 1;
    u32 value;

    if (sdPtr->config.has_cd)
    {
        while (retry >= 0)
        {
            value = FMSH_ReadReg(sdPtr->config.base, SDHCI_SRS09);
            if ((value & SDHCI_SRS09_CSS) == 0)
            {
                retry--;
                delay_ms(20);
                continue;
            }
            // high level stands for card inserted
            if (value & SDHCI_SRS09_CI)
            {
                break;
            }

            return FMSH_ENODEV;
        }
    }

    return FMSH_SUCCESS;
}

int FSdPsu_Host_WriteProt (FSdPsu_T *sdPtr)
{
    u32 value;

    if (sdPtr->config.has_wp)
    {
        value = FMSH_ReadReg(sdPtr->config.base, SDHCI_SRS09);
        // high level stands for write protect
        if (value & SDHCI_SRS09_WPSL)
        {
            return 1;
        }
    }

    return 0;
}

int FSdPsu_Host_InitHw (FSdPsu_T *sdPtr, FSdPsu_UserCfg_T *usercfg)
{
    int ret;
    int no_card, wp;
    struct sdhci *host;
    struct sdmmc_card *card;

    FMSH_ASSERT(sdPtr != NULL);

    host = &(sdPtr->host);
    card = &(sdPtr->card[0]);

    sdPtr->is_inited = 0;
    sdPtr->flags = 0;

    (void)memset(host, 0, sizeof(struct sdhci));
    (void)memset(card, 0, sizeof(struct sdmmc_card));

    if (usercfg == NULL)
    {
        sdPtr->usercfg = &default_cfg;
        if ((sdPtr->config.card_type == SDMMC_TYPE_SD) &&
            (sdPtr->config.bus_width > 4))
        {
            sdPtr->usercfg->flags |= SDMMC_F_UHS_SUPPORT;
        }
    }
    else
    {
        sdPtr->usercfg = usercfg;
    }
    
        if (sdPtr->usercfg->flags & SDMMC_F_HOST_VERSION_4)
    {
        host->version = SDMMC_HOST_VERSION_4;
    }
    else
    {
        host->version = SDMMC_HOST_VERSION_3;
    }

    // reset controller
    FSdPsu_Reset(sdPtr);

    // early initialize host
    (void)FSdPsu_Host_Initf(sdPtr);

    // power on card
    ret = FSdPsu_Host_SetPower(sdPtr, SDMMC_POWER_ON);
    if (ret)
    {
        return ret;
    }

    // check if card present and write prot
    no_card = FSdPsu_Host_CardDetect(sdPtr);
    if (no_card)
    {
        fmsh_print_err("SDMMC: No card inserted.\r\n");
        return FMSH_ENODEV;
    }

    wp = FSdPsu_Host_WriteProt(sdPtr);
    if (wp == 1)
    {
        fmsh_print_warning("SDMMC: Card is write protected.\r\n");
    }

    return 0;
}

void FSdPsu_Host_SetWakeUpEnable (FSdPsu_T *sdPtr, int enable)
{
    u32 value;

    value = FMSH_ReadReg(sdPtr->config.base, SDHCI_SRS10);
    if (enable)
    {
        value |= 0x7000000;
    }
    else
    {
        value &= ~0x7000000;
    }
    FMSH_WriteReg(sdPtr->config.base, SDHCI_SRS10, value);
}

int FSdPsu_Host_SetPower (FSdPsu_T *sdPtr, u32 power)
{
    int ret = 0;
    u32 value;

    value = FMSH_ReadReg(sdPtr->config.base, SDHCI_SRS10);

    // regulator is not supported, always power on;
    switch (power)
    {
    case SDMMC_POWER_OFF:
        /* SD Bus Power */
        FMSH_WriteReg(sdPtr->config.base, SDHCI_SRS10, value & ~SDHCI_SRS10_BP);
        delay_ms(2);
        return FMSH_SUCCESS;
    case SDMMC_POWER_ON:
        /* SD Bus Power */
        FMSH_WriteReg(sdPtr->config.base, SDHCI_SRS10, value | SDHCI_SRS10_BP);
        delay_ms(3);
        return FMSH_SUCCESS;
    case SDMMC_POWER_CYCLE:
        if (sdPtr->host.powercycle)
        {
            ret = sdPtr->host.powercycle(sdPtr);
            return ret;
        }
        else
        {
            FMSH_WriteReg(sdPtr->config.base, SDHCI_SRS10,
                          value & ~SDHCI_SRS10_BP);
            delay_ms(2);
            FMSH_WriteReg(sdPtr->config.base, SDHCI_SRS10,
                          value | SDHCI_SRS10_BP);
            delay_ms(3);
            return FMSH_SUCCESS;
            // return FMSH_EIO;
        }
    default:
        return FMSH_EIO;
    }
}

int FSdPsu_Host_SetClock (FSdPsu_T *sdPtr, u32 freq)
{
    int i;
    u32 value, clk_div;
    u32 sdclk;
    int timeout = 10;

    FMSH_ASSERT(sdPtr != NULL);

    // get SRS11
    value = FMSH_ReadReg(sdPtr->config.base, SDHCI_SRS11);

    // disable clock
    value &= ~SDHCI_SRS11_CLOCK;
    FMSH_WriteReg(sdPtr->config.base, SDHCI_SRS11, value);

    if (freq == 0)
    {
        fmsh_print_dbg("SDMMC: Clock is disabled\r\n");
        return 0;
    }

    // change to freq (real freq should not bigger than this value)
    sdclk = sdPtr->config.input_clock_hz;
    if (sdPtr->usercfg->flags & SDMMC_F_USE_PRESET_VALUE)
    {
        clk_div = ((value & 0xc0) << 2) | ((value & 0xff00) >> 8);
    }
    else
    {
        if (sdclk <= freq)
        {
            clk_div = 0;
            goto change_freq;
        }

        for (i = 1; i < 1024; i++)
        {
            clk_div = i;
            if ((sdclk / (i * 2)) <= freq)
            {
                goto change_freq;
            }
        }
        if (i == 1024)
        {
            return FMSH_FAILURE;
        }

change_freq:
        value |= (clk_div & 0xff) << 8;
        value |= (clk_div & 0x300) >> 2;
    }

    // enable internal clock and wait for int clk stable
    value |= SDHCI_CLOCK_ICE;
    FMSH_WriteReg(sdPtr->config.base, SDHCI_SRS11, value);
    while (1)
    {
        value = FMSH_ReadReg(sdPtr->config.base, SDHCI_SRS11);
        if ((value & SDHCI_CLOCK_ICS) == SDHCI_CLOCK_ICS)
        {
            break;
        }

        timeout--;
        if (timeout <= 0)
        {
            fmsh_print_err("SDMMC: Clock is not stable\r\n");
            return FMSH_ETIME;
        }
        delay_ms(1);
    }

    // enable sd clk
    value |= SDHCI_CLOCK_SDCE;
    FMSH_WriteReg(sdPtr->config.base, SDHCI_SRS11, value);
    if (clk_div == 0)
    {
        sdPtr->host.sdclk = sdclk;
    }
    else
    {
        sdPtr->host.sdclk = sdclk / (clk_div * 2);
    };

    fmsh_print_dbg("SDMMC: sdmclk is %d Hz\r\n", sdPtr->host.sdclk);

    return FMSH_SUCCESS;
}

int FSdPsu_Host_SetBusWidth (FSdPsu_T *sdPtr, int width)
{
    u32 value;

    FMSH_ASSERT(sdPtr != NULL);

    value = FMSH_ReadReg(sdPtr->config.base, SDHCI_SRS10);
    value &= ~SDHCI_SRS10_WIDTH;
    switch (width)
    {
    case 1:
        value |= SDHCI_WIDTH_1;
        break;
    case 4:
        value |= SDHCI_WIDTH_4;
        break;
    case 8:
        value |= SDHCI_WIDTH_8;
        break;
    default:
        return FMSH_EINVAL;
    }
    FMSH_WriteReg(sdPtr->config.base, SDHCI_SRS10, value);
    sdPtr->host.bus_width = width;

    return 0;
}

int FSdPsu_Host_SetBusSpeed (FSdPsu_T *sdPtr, int mode)
{
    u32 value, ctrl = 0, ctrl2 = 0, emmc_ctrl = 0;
    struct sdmmc_card *card;

    FMSH_ASSERT(sdPtr != NULL);

    card = &(sdPtr->card[0]);

    switch (mode)
    {
    case SDMMC_DS:
        if (card->card_type == SDMMC_TYPE_MMC)
        {
            emmc_ctrl = SDHCI_HRS06_EMM_LEGACY;
        }
        break;

    case SD_HS:
        ctrl = SDHCI_SRS10_HSE;
        break;
    case UHS_SDR12:
        ctrl2 = SDHCI_UMS_SDR12 | SDHCI_SRS15_V18SE;
        break;
    case UHS_SDR25:
        ctrl2 = SDHCI_UMS_SDR25 | SDHCI_SRS15_V18SE;
        break;
    case UHS_SDR50:
        ctrl2 = SDHCI_UMS_SDR50 | SDHCI_SRS15_V18SE;
        break;
    case UHS_DDR50:
        ctrl2 = SDHCI_UMS_DDR50 | SDHCI_SRS15_V18SE;
        break;
    case UHS_SDR104:
        ctrl2 = SDHCI_UMS_SDR104 | SDHCI_SRS15_V18SE;
        break;

    case MMC_HS26:
    case MMC_HS52:
        emmc_ctrl = SDHCI_HRS06_EMM_SDR;
        break;
    case MMC_HS52_DDR:
        emmc_ctrl = SDHCI_HRS06_EMM_DDR;
        break;
    case MMC_HS200:
        emmc_ctrl = SDHCI_HRS06_EMM_HS200;
        break;
    case MMC_HS400:
        emmc_ctrl = SDHCI_HRS06_EMM_HS400;
        break;
    case MMC_HS400_ES:
        emmc_ctrl = SDHCI_HRS06_EMM_HS400ES;
        break;

    default:
        return FMSH_EINVAL;
    }

    FMSH_WriteReg(sdPtr->config.base, SDHCI_HRS06, emmc_ctrl);

    value = FMSH_ReadReg(sdPtr->config.base, SDHCI_SRS10);
    value &= ~SDHCI_SRS10_HSE;
    FMSH_WriteReg(sdPtr->config.base, SDHCI_SRS10, value | ctrl);

    value = FMSH_ReadReg(sdPtr->config.base, SDHCI_SRS15);
    value &= ~(SDHCI_SRS15_UMS | SDHCI_SRS15_V18SE);
    FMSH_WriteReg(sdPtr->config.base, SDHCI_SRS15, value | ctrl2);

    sdPtr->host.mode = mode;
    delay_ms(5);

    return FMSH_SUCCESS;
}

int FSdPsu_Host_DefaultIOs (FSdPsu_T *sdPtr)
{
    int ret;
    struct sdmmc_card *card;

    FMSH_ASSERT(sdPtr != NULL);

    card = &(sdPtr->card[0]);
    card->freq = sdPtr->config.init_freq;
    card->bus_width = 1;
    card->mode = SDMMC_DS;
    card->bus_voltage = SDMMC_BVS_330;

    ret = FSdPsu_Host_SetBusWidth(sdPtr, card->bus_width);
    if (ret)
    {
        return ret;
    }

    ret = FSdPsu_Host_SetBusSpeed(sdPtr, card->mode);
    if (ret)
    {
        return ret;
    }

    ret = FSdPsu_Host_SetClock(sdPtr, card->freq);
    if (ret)
    {
        return ret;
    }

    ret = FSdPsu_Phy_Config(sdPtr, SDMMC_DS_ID);
    if (ret)
    {
        return ret;
    }

    delay_ms(3);

    return 0;
}

static void FSdPsu_Host_Response (FSdPsu_T *sdPtr, u32 *resp)
{
    if (resp)
    {
        resp[0] = FMSH_ReadReg(sdPtr->config.base, SDHCI_SRS04);
        resp[1] = FMSH_ReadReg(sdPtr->config.base, SDHCI_SRS05);
        resp[2] = FMSH_ReadReg(sdPtr->config.base, SDHCI_SRS06);
        resp[3] = FMSH_ReadReg(sdPtr->config.base, SDHCI_SRS07);
    }
}

int FSdPsu_Host_SendCmd (FSdPsu_T *sdPtr, struct sdmmc_cmd *cmd,
                         struct sdmmc_data *data)
{
    int ret = 0;
    int no_transfer = 0;
    u32 value, mask, flag;
    u32 blocksize, tsfmode;
    __attribute__((unused)) u32 trans_bytes;
    // int trans_bytes;
    struct sdhci *host;
    struct sdmmc_usercfg *usercfg;
    int timeout;

    FMSH_ASSERT(sdPtr != NULL);
    FMSH_ASSERT(cmd != NULL);

    host = &(sdPtr->host);
    usercfg = sdPtr->usercfg;

    if (data)
    {
        if (data->flags & SDMMC_FLDATA_NOTRANSFER)
        {
            no_transfer = 1;
        }
    }

    // wait for cmd & dat inhibit
    mask = SDHCI_SRS09_CICMD | SDHCI_SRS09_CIDAT;
    if ((cmd->idx == SDMMC_CMD_STOP_TRANSMISSION) || no_transfer)
    {
        mask &= ~SDHCI_SRS09_CIDAT;
    }

    timeout = 1000000;
    while (1)
    {
        value = FMSH_ReadReg(sdPtr->config.base, SDHCI_SRS09);
        if ((value & mask) == 0)
        {
            break;
        }
        delay_us(10);
        timeout -= 10;
        if (timeout <= 0)
        {
            return FMSH_ETIME;
        }
    };

    // clear and enable interrupt
    FMSH_WriteReg(sdPtr->config.base, SDHCI_SRS12, SDHCI_INT_ALL_MASK);
    FMSH_WriteReg(sdPtr->config.base, SDHCI_SRS13, SDHCI_INT_ALL_MASK);
    if (usercfg->flags & SDMMC_F_USE_INTR)
    {
        FMSH_WriteReg(sdPtr->config.base, SDHCI_SRS14, SDHCI_INT_ALL_MASK);
    }

    mask = SDHCI_INT_CC;
    if (no_transfer)
    {
        mask |= SDHCI_INT_BRR;
    }

    // set command reg (response type)
    if ((cmd->resp_type & SDMMC_RESP_PRESENT) == 0)
    {
        flag = SDHCI_RTS_NONE;
    }
    else if (cmd->resp_type & SDMMC_RESP_136)
    {
        flag = SDHCI_RTS_LONG;
    }
    else if (cmd->resp_type & SDMMC_RESP_BUSY)
    {
        flag = SDHCI_RTS_SHORT_BUSY;
        if (data && (!no_transfer))
        {
            mask |= SDHCI_INT_TC;
        }
    }
    else
    {
        flag = SDHCI_RTS_SHORT;
    }

    // set command reg (command check crc enable)
    if (cmd->resp_type & SDMMC_RESP_CRC)
    {
        flag |= SDHCI_SRS03_CRCCE;
    }
    // set command reg (command index check enable)
    if (cmd->resp_type & SDMMC_RESP_OPCODE)
    {
        flag |= SDHCI_SRS03_CICE;
    }
    // set command reg (data present select)
    if (data)
    {
        flag |= SDHCI_SRS03_DPS;
    }

    // set command reg (command type)
    if (cmd->flags & SDMMC_FLCMD_SUSPEND)
    {
        flag |= SDHCI_CT_SUSPEND;
    }
    else if (cmd->flags & SDMMC_FLCMD_RESUME)
    {
        flag |= SDHCI_CT_RESUME;
    }
    else if (cmd->flags & SDMMC_FLCMD_ABORT)
    {
        flag |= SDHCI_CT_ABORT;
    }

    tsfmode = 0;
    if (data)
    {
        value = FMSH_ReadReg(sdPtr->config.base, SDHCI_SRS11);
        value &= ~SDHCI_SRS11_TIMEOUT;
        value |= 0xe << 16;
        FMSH_WriteReg(sdPtr->config.base, SDHCI_SRS11, value);

        // set transfer mode (block count enable)
        tsfmode |= SDHCI_SRS03_BCE;

        trans_bytes = data->blocks * data->blocksize;
        // set transfer mode (multi/single block select)
        if (data->blocks > 1)
        {
            tsfmode |= SDHCI_MSBS_MULTI;
        }

        // set transfer mode (data transfer direction)
        if (data->flags & SDMMC_FLDATA_READ)
        {
            tsfmode |= SDHCI_DTDS_READ;
        }

        // set transfer mode (dma enable)
        if ((usercfg->dma) && (!no_transfer))
        {
            tsfmode |= SDHCI_SRS03_DMAE;
            (void)FSdPsu_Host_PrepareDma(sdPtr, data);
        }

        // set transfer mode (auto cmd enable)
        if (data->flags & SDMMC_FLDATA_AUTOCMD12)
        {
            tsfmode |= SDHCI_ACE_CMD12;
        }
        if (data->flags & SDMMC_FLDATA_AUTOCMD23)
        {
            tsfmode |= SDHCI_ACE_CMD23;
        }

        // set block size
        blocksize = (0x7 << 12) | (data->blocksize & 0xfff);
        if (host->version == SDMMC_HOST_VERSION_3)
        {
            // block count is set in SRS01 if host ver is 3.0
            value = (data->blocks << 16) | blocksize;
        }
        else
        {
            FMSH_WriteReg(sdPtr->config.base, SDHCI_SRS00, data->blocks);
            value = blocksize;
        }
        FMSH_WriteReg(sdPtr->config.base, SDHCI_SRS01, value);
    }
    else if (cmd->resp_type & SDMMC_RESP_BUSY)
    {
        value = FMSH_ReadReg(sdPtr->config.base, SDHCI_SRS11);
        value &= ~SDHCI_SRS11_TIMEOUT;
        value |= 0xe << 16;
        FMSH_WriteReg(sdPtr->config.base, SDHCI_SRS11, value);
    }
    else
    {
        ; /* no deal with */
    }

    // send cmd
    FMSH_WriteReg(sdPtr->config.base, SDHCI_SRS02, cmd->arg);
    value = tsfmode | flag | (cmd->idx << 24);
    FMSH_WriteReg(sdPtr->config.base, SDHCI_SRS03, value);

    // wait for cmd done
    timeout = 2000;
    do
    {
        value = FMSH_ReadReg(sdPtr->config.base, SDHCI_SRS12);
        if (value & SDHCI_INT_EINT)
        {
            break;
        }

        delay_us(10);
        timeout -= 10;
        if (timeout <= 0)
        {
            return FMSH_ETIME;
        }
    } while ((value & mask) != mask);

    if ((value & (SDHCI_SRS12_ERROR | mask)) == mask)
    {
        FSdPsu_Host_Response(sdPtr, cmd->resp);
        // clear interrupt
        FMSH_WriteReg(sdPtr->config.base, SDHCI_SRS12, mask);
        ret = 0;
    }
    else
    {
        ret = 1;
    }

    // transfer data
    if ((!ret) && data && (!no_transfer))
    {
        ret = FSdPsu_Host_TransferData(sdPtr, data);
    }

    // clear interrupt
    value = FMSH_ReadReg(sdPtr->config.base, SDHCI_SRS12);
    FMSH_WriteReg(sdPtr->config.base, SDHCI_SRS12, SDHCI_INT_ALL_MASK);
    if (!ret)
    {
        return 0;
    }

    fmsh_print_err("FSdPsu_Host_SendCmd: SRS12 is 0x%x\r\n", value);
    (void)FSdPsu_Host_SoftReset(sdPtr, SDHCI_RESET_SRCMD | SDHCI_RESET_SRDAT);
    if (value & SDHCI_ERR_ECT)
    {
        return SDMMC_ENORESP;
    }
    else
    {
        return FMSH_EIO;
    }
}

/************************ Transfer without DMA ******************************/
static void FSdPsu_Host_PIOTransfer (FSdPsu_T *sdPtr, struct sdmmc_data *data)
{
    int i;
    uintptr_t offs;

    for (i = 0; i < data->blocksize; i += 4)
    {
        offs = (long long)(data->buf) + i;
        if (data->flags & SDMMC_FLDATA_READ)
        {
            *(u32 *)offs = FSdPsu_Host_ReadBuf(sdPtr);
        }
        else
        {
            FSdPsu_Host_WriteBuf(sdPtr, *(u32 *)offs);
        }
    }
}

/************************ Transfer without ADMA *****************************/
static void FSdPsu_Host_SetupADMADescTbl (FSdPsu_T *sdPtr,
                                          struct sdmmc_data *data,
                                          struct sdhci_adma_desc *desc)
{
    int i, tlbsize;
    u32 bytecount;
    u64 dmaaddr;

    bytecount = (data->blocks * data->blocksize);
    dmaaddr = (unsigned long long)(data->buf);

    tlbsize = bytecount / SDHCI_ADMA_MAX_LEN;
    if (bytecount % SDHCI_ADMA_MAX_LEN)
    {
        tlbsize += 1;
    }

    for (i = 0; i < tlbsize - 1; i++)
    {
        desc[i].attr = SDHCI_ADMA_ATTR_VAL | SDHCI_ADMA_ATTR_INT |
                       SDHCI_ADMA_ATTR_TRANS;
        desc[i].length = SDHCI_ADMA_MAX_LEN;
        desc[i].address = dmaaddr;

        bytecount -= SDHCI_ADMA_MAX_LEN;
        dmaaddr += SDHCI_ADMA_MAX_LEN;
    }

    desc[i].attr = SDHCI_ADMA_ATTR_VAL | SDHCI_ADMA_ATTR_END |
                   SDHCI_ADMA_ATTR_INT | SDHCI_ADMA_ATTR_TRANS;
    desc[i].length = bytecount;
    desc[i].address = dmaaddr;
#if (DCACHE_ENABLE == 1)
    if (sdPtr->config.is_cache_coherent == 0)
    {
        Fmsh_DCacheFlushRange((uintptr_t)desc,
                              sizeof(struct sdhci_adma_desc) * tlbsize);
    }
#endif
    dsb();
}

/************************ Transfer without SDMA *****************************/
static int FSdPsu_Host_PrepareDma (FSdPsu_T *sdPtr, struct sdmmc_data *data)
{
    u32 ctrl, ctrl2, val;
    struct sdhci *host = &(sdPtr->host);
    struct sdmmc_usercfg *usercfg = sdPtr->usercfg;
    struct sdhci_adma_desc *desc = sdPtr->desc;

    // set SRS10.DMASEL
    ctrl = FMSH_ReadReg(sdPtr->config.base, SDHCI_SRS10);
    ctrl &= ~SDHCI_SRS10_DMASEL;

    // set SRS15.64B
    ctrl2 = FMSH_ReadReg(sdPtr->config.base, SDHCI_SRS15);
    ctrl2 &= ~SDHCI_SRS15_A64B;

    // sdma support 32-bit or 64-bit address, 64-bit address is only used when
    // hv4e = 1
    if ((usercfg->dma == SDMMC_USE_SDMA) || (usercfg->dma == SDMMC_USE_SDMA64))
    {
        ctrl |= SDHCI_DMASEL_SDMA;
        if (host->version == SDMMC_HOST_VERSION_3)
        {
            FMSH_WriteReg(sdPtr->config.base, SDHCI_SRS00,
                          (unsigned long long)(data->buf));
        }
        else
        {
            FMSH_WriteReg(sdPtr->config.base, SDHCI_SRS22,
                          (unsigned long long)(data->buf));
            if (usercfg->dma == SDMMC_USE_SDMA64)
            {
                ctrl2 |= SDHCI_SRS15_A64B;
                val = ((unsigned long long)(data->buf)) >> 32U;
                FMSH_WriteReg(sdPtr->config.base, SDHCI_SRS23, val);
            }
        }
    }
    else
    {
        FSdPsu_Host_SetupADMADescTbl(sdPtr, data, desc);
        FMSH_WriteReg(sdPtr->config.base, SDHCI_SRS22, (u64)desc & 0xffffffff);
        if (usercfg->dma == SDMMC_USE_ADMA)
        {
            ctrl |= SDHCI_DMASEL_ADMA2;
        }
        else if (usercfg->dma == SDMMC_USE_ADMA64)
        {
            if (host->version == SDMMC_HOST_VERSION_3)
            {
                ctrl |= SDHCI_DMASEL_ADMA3;
            }
            else
            {
                ctrl |= SDHCI_DMASEL_ADMA2;
            }
            ctrl2 |= SDHCI_SRS15_A64B;
            val = ((unsigned long long)desc >> 32U) & 0xffffffff;
            FMSH_WriteReg(sdPtr->config.base, SDHCI_SRS23, val);
        }
        else
        {
            ; /* no deal with */
        }
    }

    FMSH_WriteReg(sdPtr->config.base, SDHCI_SRS10, ctrl);
    FMSH_WriteReg(sdPtr->config.base, SDHCI_SRS15, ctrl2);
#if (DCACHE_ENABLE == 1)
    if (sdPtr->config.is_cache_coherent == 0)
    {
        if (data->flags & SDMMC_FLDATA_READ)
        {
            Fmsh_DCacheInvalidateRange((uintptr_t)data->buf,
                                       data->blocks * data->blocksize);
        }
        else
        {
            Fmsh_DCacheFlushRange((uintptr_t)data->buf,
                                  data->blocks * data->blocksize);
        }
    }
#endif
    return FMSH_SUCCESS;
}

static int FSdPsu_Host_TransferData (FSdPsu_T *sdPtr, struct sdmmc_data *data)
{
    u8 *buf;
    unsigned int stat, mask, timeout, blocks = 0;
    int transfer_done = 0;
    struct sdhci *host = &(sdPtr->host);

    // makesure timeout not overflow
    timeout = 50000 * data->blocks;
    buf = data->buf;

    mask = SDHCI_SRS09_BWE | SDHCI_SRS09_BRE;
    while (1)
    {
        stat = FMSH_ReadReg(sdPtr->config.base, SDHCI_SRS12);
        // check error
        if (stat & SDHCI_INT_EINT)
        {
            return FMSH_EIO;
        }

        // check transfer end
        if (stat & SDHCI_INT_TC)
        {
            // clear status
            FMSH_WriteReg(sdPtr->config.base, SDHCI_SRS12, SDHCI_INT_TC);
            break;
        }

        // use dma mode to transfer data
        if (sdPtr->usercfg->dma)
        {
            // only transfer data if previous dma transfer done
            if (stat & SDHCI_INT_DMAINT)
            {
                // clear dma int
                FMSH_WriteReg(sdPtr->config.base, SDHCI_SRS12, SDHCI_INT_DMAINT);
                // update sdma address, address must align to boundary
                if ((sdPtr->usercfg->dma == SDMMC_USE_SDMA) || (sdPtr->usercfg->dma == SDMMC_USE_SDMA64))
                {
                    // update dma system address and start new transfer
                    buf = (u8 *)(((unsigned long long)buf + SDMMC_DEFAULT_BOUNDARY_SIZE) &
                                 (~(SDMMC_DEFAULT_BOUNDARY_SIZE - 1)));
                    if (host->version == SDMMC_HOST_VERSION_3)
                    {
                        FMSH_WriteReg(sdPtr->config.base, SDHCI_SRS00, (unsigned long long)buf);
                    }
                    else
                    {
                        FMSH_WriteReg(sdPtr->config.base, SDHCI_SRS22, (unsigned long long)buf);
                        if (sdPtr->usercfg->dma == SDMMC_USE_SDMA64)
                        {
                            u32 val = ((unsigned long long)buf) >> 32U;
                            FMSH_WriteReg(sdPtr->config.base, SDHCI_SRS23, val);
                        }
                    }
                }
            }
        }
        else
        {
            // only transfer data if data left
            if (transfer_done == 0)
            {
                // wait for buffer rdy
                if ((FMSH_ReadReg(sdPtr->config.base, SDHCI_SRS09) & mask) == 0)
                {
                    continue;
                }
                data->buf = (void *)buf;
                FSdPsu_Host_PIOTransfer(sdPtr, data);
                buf += data->blocksize;
                blocks++;
                if (blocks >= data->blocks)
                {
                    transfer_done = 1;
                    continue;
                }
            }
        }

        // check timeout
        timeout -= 1;
        if (timeout <= 0)
        {
            return FMSH_ETIME;
        }

        delay_us(10);
    }
#if (DCACHE_ENABLE == 1)
    if ((sdPtr->usercfg->dma) && (sdPtr->config.is_cache_coherent == 0U))
    {
        if (data->flags & SDMMC_FLDATA_READ)
        {
            Fmsh_DCacheInvalidateRange((uintptr_t)data->buf,
                                       data->blocks * data->blocksize);
        }
    }
#endif
    return 0;
}

#if (SDMMC_CONFIG_TUNING_SUPPORT == 1)

#define SDMMC_MAX_TUNING_RETRY 40

extern int SDMMC_Tuning(FSdPsu_T *sdPtr, int no_transfer);

static int host_mmc_execute_tuning (FSdPsu_T *sdPtr)
{
    int ret, i;
    int cur_streak, max_streak, end_of_streak;

    for (i = 0; i < SDMMC_MAX_TUNING_RETRY; i++)
    {
        FSdPsu_Phy_SetDqsDelay(sdPtr, i * 256 / 40);
        ret = SDMMC_Tuning(sdPtr, 0);
        if (ret)
        {
            cur_streak = 0;
        }
        else
        {
            cur_streak++;
            if (cur_streak > max_streak)
            {
                max_streak = cur_streak;
                end_of_streak = i;
            }
        }
    }

    if (max_streak == 0)
    {
        return FMSH_EIO;
    }

    FSdPsu_Phy_SetDqsDelay(sdPtr, (end_of_streak - max_streak / 2) * 256 / 40);

    return 0;
}

static int host_sd_execute_tuning (FSdPsu_T *sdPtr)
{
    int i;
    u32 value;

    // set execute tuning to 1
    value = FMSH_ReadReg(sdPtr->config.base, SDHCI_SRS15);
    value |= SDHCI_SRS15_EXTNG;
    value &= ~SDHCI_SRS15_SCS;
    FMSH_WriteReg(sdPtr->config.base, SDHCI_SRS15, value);

    delay_ms(1);

    for (i = 0; i < SDMMC_MAX_TUNING_RETRY; i++)
    {
        SDMMC_Tuning(sdPtr, 1);

        // check execute tuning
        value = FMSH_ReadReg(sdPtr->config.base, SDHCI_SRS15);
        if ((value & SDHCI_SRS15_EXTNG) == 0)
        {
            break;
        }
    }

    value = FMSH_ReadReg(sdPtr->config.base, SDHCI_SRS15);
    if ((value & SDHCI_SRS15_SCS) == 0)
    {
        return 1;
    }

    return 0;
}

int FSdPsu_Host_ExecuteTuning (FSdPsu_T *sdPtr)
{
    int ret;
    struct sdmmc_card *card;

    FMSH_ASSERT(sdPtr != NULL);

    card = &(sdPtr->card[0]);

    if (card->card_type == SDMMC_TYPE_SD)
    {
        ret = host_sd_execute_tuning(sdPtr);
    }
    else if (card->card_type == SDMMC_TYPE_MMC)
    {
        ret = host_mmc_execute_tuning(sdPtr);
    }
    else
    {
        ret = FMSH_FAILURE;
    }

    return ret;
}

#endif /* (SDMMC_CONFIG_TUNING_SUPPORT == 1) */

extern int SDMMC_StopTransmission(FSdPsu_T *sdPtr, u32 *status);

int FSdPsu_Host_ErrorRecovery (FSdPsu_T *sdPtr)
{
    int ret = 0;
    u32 intr_sig, value, status;
    __attribute__((unused)) u32 intr_status;

    intr_sig = FMSH_ReadReg(sdPtr->config.base, SDHCI_SRS14);

    value = FMSH_ReadReg(sdPtr->config.base, SDHCI_SRS12);
    if (value & 0xf0000)
    {
        (void)FSdPsu_Host_SoftReset(sdPtr, SDHCI_RESET_SRCMD);
    }

    value = FMSH_ReadReg(sdPtr->config.base, SDHCI_SRS12);
    if (value & 0x700000)
    {
        (void)FSdPsu_Host_SoftReset(sdPtr, SDHCI_RESET_SRDAT);
    }

    intr_status = FMSH_ReadReg(sdPtr->config.base, SDHCI_SRS12);
    FMSH_WriteReg(sdPtr->config.base, SDHCI_SRS12, SDHCI_INT_ALL_MASK);

    SDMMC_StopTransmission(sdPtr, &status);

    value = FMSH_ReadReg(sdPtr->config.base, SDHCI_SRS12);
    if (value & 0xf0000)
    {
        (void)FSdPsu_Host_SoftReset(sdPtr, SDHCI_RESET_SRCMD);
        if (status & SDMMC_STATUS_ERROR)
        {
            ret = 1;
        }
        goto end;
    }

    if (value & 0x100000)
    {
        ret = 1;
        goto end;
    }

    delay_us(40);

    value = FMSH_ReadReg(sdPtr->config.base, SDHCI_SRS09);
    if ((value & 0xf00000) != 0xf00000)
    {
        ret = 1;
        goto end;
    }

end:
    if (ret)
    {
        fmsh_print_err("Non-recoverable error!");
    }
    else
    {
        fmsh_print_info("Recoverable error!");
    }

    FMSH_WriteReg(sdPtr->config.base, SDHCI_SRS14, intr_sig);

    return 0;
}
