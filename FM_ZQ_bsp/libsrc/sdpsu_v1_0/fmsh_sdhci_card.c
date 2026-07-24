#include <string.h>
#include <stdlib.h>
#include "fmsh_common.h"
#include "fmsh_sdhci.h"
#include "fmsh_sdhci_card.h"
#include "fmsh_sdhci_pro.h"

/****************************************************
 * CMD0 is defined to reset card.
 * after reset command, card is in idle state.
 * CMD line input / default driver strength with 400kHz /
 * 3.3v signal voltage
 *****************************************************/
static int SDMMC_GoIdleState(FSdPsu_T *sdPtr);
/****************************************************
 * CMD2 is defined to get All device CID register
 *****************************************************/
static int SDMMC_AllSendCID(FSdPsu_T *sdPtr);
/****************************************************
 * CMD3 is defined to set relative address
 *****************************************************/
static int SDMMC_SendRCA(FSdPsu_T *sdPtr);
/********************************************************
 * This function check switchable function and switch function.
 *********************************************************/
static int SD_Switch(FSdPsu_T *sdPtr, unsigned int mode, int group, u32 value,
                     void *resp);
static int MMC_Switch(FSdPsu_T *sdPtr, int index, u32 value);
/****************************************************
 * CMD7 is defined to toggle a card between standby and
 * transfer states.
 *****************************************************/
static int SDMMC_SelectCard(FSdPsu_T *sdPtr);
/****************************************************
 * CMD9 is defined to get CSD register
 *****************************************************/
static int SDMMC_SendCSD(FSdPsu_T *sdPtr);
/****************************************************
 * CMD12 is defined to stop transmission
 *****************************************************/
static int SDMMC_StopTransmission(FSdPsu_T *sdPtr, u32 *status);
/****************************************************
 * CMD13 is defined to get card status
 *****************************************************/
static int SDMMC_SendStatus(FSdPsu_T *sdPtr, u32 *status);
/****************************************************
 * CMD16 is defined to set block length for rd/wr operation
 *****************************************************/
static int SDMMC_SetBlockLen(FSdPsu_T *sdPtr, int len);
/****************************************************
 * CMD23 is defined to set block length for rd/wr operation
 *****************************************************/
static int SDMMC_SetBlockCnt(FSdPsu_T *sdPtr, int cnt);
/****************************************************
 * CMD55 is defined that next cmd is acmd
 *****************************************************/
static int SDMMC_Cmd55(FSdPsu_T *sdPtr);
/****************************************************
 * ACMD41 is defined to get sd voltage support and switch voltage
 *****************************************************/
static int SD_SendOpCond(FSdPsu_T *sdPtr, unsigned int uhs_en);
/****************************************************
 * CMD1 is defined to get MMC voltage support
 *****************************************************/
static int MMC_SendOpCond(FSdPsu_T *sdPtr);
/********************************************************
 * This function switchs the bus speed mode (both card + host);
 * Default Speed, High Speed mode and UHS-I mode.
 *********************************************************/
static int SD_SwitchVoltage(FSdPsu_T *sdPtr);
static int SD_GetCapabilities(FSdPsu_T *sdPtr);
static int MMC_GetCapabilities(FSdPsu_T *sdPtr);
static int SDMMC_WaitReady(FSdPsu_T *sdPtr, int timeout);

static u32 get_resp_field (u32 *resp, u32 field, int bit_size)
{
    int i = field / 32;
    u32 mask = (bit_size < 32) ? (1U << bit_size) - 1 : 0xFFFFFFFF;
    u32 shift = field & 31;
    u32 value;

    value = resp[i] >> shift;
    if (bit_size + shift >= 32)
    {
        value |= resp[i + 1] << (32 - shift);
    }
    return (value & mask);
}

/******************** Common cmd seq ***************************/
static int SDMMC_GoIdleState (FSdPsu_T *sdPtr)
{
    int ret;
    struct sdmmc_cmd cmd;

    cmd.idx = SDMMC_CMD_GO_IDLE_STATE;  // CMD0
    cmd.arg = 0;
    cmd.resp_type = SDMMC_RESP_NONE;
    cmd.flags = 0;
    ret = FSdPsu_Host_SendCmd(sdPtr, &cmd, NULL);
    if (ret)
    {
        return ret;
    }

    delay_ms(1);

    return 0;
}

static int SDMMC_AllSendCID (FSdPsu_T *sdPtr)
{
    int ret;
    struct sdmmc_cmd cmd;
    struct sdmmc_card *card;
    u8 temp[16];

    card = &(sdPtr->card[0]);

    cmd.idx = SDMMC_CMD_ALL_SEND_CID;  // CMD2
    cmd.arg = 0x0;
    cmd.resp_type = SDMMC_RESP_R2;
    cmd.flags = 0;
    ret = FSdPsu_Host_SendCmd(sdPtr, &cmd, NULL);
    if (ret)
    {
        return ret;
    }

    // cid(bit8) -> resp(bit0), crc is skipped
    (void)memcpy(&(temp[1]), &(cmd.resp[0]), 15);
    (void)memcpy(&(card->cid[0]), &(temp[0]), 16);

    return 0;
}

static int SDMMC_SendRCA (FSdPsu_T *sdPtr)
{
    int ret;
    struct sdmmc_cmd cmd;
    struct sdmmc_card *card;

    card = &(sdPtr->card[0]);

    cmd.idx = SDMMC_CMD_SET_RELATIVE_ADDR;  // CMD3
    cmd.arg = card->rca << 16;
    cmd.resp_type = SDMMC_RESP_R6;
    cmd.flags = 0;
    ret = FSdPsu_Host_SendCmd(sdPtr, &cmd, NULL);
    if (ret)
    {
        return ret;
    }

    if (card->card_type == SDMMC_TYPE_SD)
    {
        card->rca = (cmd.resp[0] >> 16) & 0xffff;
    }

    return 0;
}

__attribute__((unused)) static int SDMMC_SetDSR (FSdPsu_T *sdPtr, u32 dsr)
{
    int ret;
    struct sdmmc_cmd cmd = {0};

    cmd.idx = SDMMC_CMD_SET_DSR;  // CMD7
    cmd.arg = dsr << 16;
    cmd.resp_type = SDMMC_RESP_NONE;
    cmd.flags = 0;
    ret = FSdPsu_Host_SendCmd(sdPtr, &cmd, NULL);

    return ret;
}

static int SDMMC_SelectCard (FSdPsu_T *sdPtr)
{
    int ret;
    struct sdmmc_cmd cmd = {0};
    struct sdmmc_card *card;

    card = &(sdPtr->card[0]);

    cmd.idx = SDMMC_CMD_SELECT_CARD;  // CMD7
    cmd.arg = card->rca << 16;
    cmd.resp_type = SDMMC_RESP_R1;
    cmd.flags = 0;
    ret = FSdPsu_Host_SendCmd(sdPtr, &cmd, NULL);

    return ret;
}

static int SDMMC_SendCSD (FSdPsu_T *sdPtr)
{
    int ret;
    struct sdmmc_cmd cmd = {0};
    struct sdmmc_card *card;
    u8 temp[16] = {0};

    card = &(sdPtr->card[0]);

    cmd.idx = SDMMC_CMD_SEND_CSD;
    cmd.arg = card->rca << 16;
    cmd.resp_type = SDMMC_RESP_R2;
    cmd.flags = 0;
    ret = FSdPsu_Host_SendCmd(sdPtr, &cmd, NULL);
    if (ret)
    {
        return ret;
    }

    (void)memcpy(&temp[1], &(cmd.resp[0]), 15);
    (void)memcpy(&(card->csd[0]), &temp[0], 16);

    return 0;
}

int SDMMC_StopTransmission (FSdPsu_T *sdPtr, u32 *status)
{
    int ret;
    struct sdmmc_cmd cmd = {0};

    cmd.idx = SDMMC_CMD_STOP_TRANSMISSION;  // CMD12
    cmd.arg = 0;
    cmd.resp_type = SDMMC_RESP_R1b;
    cmd.flags = 0;
    ret = FSdPsu_Host_SendCmd(sdPtr, &cmd, NULL);
    if (ret)
    {
        return ret;
    }

    if (status)
    {
        *status = cmd.resp[0];
    }

    return 0;
}

int SDMMC_SendStatus (FSdPsu_T *sdPtr, u32 *status)
{
    int ret;
    struct sdmmc_cmd cmd = {0};
    struct sdmmc_card *card;

    card = &(sdPtr->card[0]);

    cmd.idx = SDMMC_CMD_SEND_STATUS;  // CMD13
    cmd.arg = card->rca << 16;
    cmd.resp_type = SDMMC_RESP_R1;
    cmd.flags = 0;
    ret = FSdPsu_Host_SendCmd(sdPtr, &cmd, NULL);
    if (ret)
    {
        return ret;
    }

    if (status)
    {
        *status = cmd.resp[0];
    }

    return 0;
}

#if (SDMMC_CONFIG_TUNING_SUPPORT == 1)
static const u8 tuning_blk_pattern_4bit[64] = {
    0xff, 0x0f, 0xff, 0x00, 0xff, 0xcc, 0xc3, 0xcc, 0xc3, 0x3c, 0xcc,
    0xff, 0xfe, 0xff, 0xfe, 0xef, 0xff, 0xdf, 0xff, 0xdd, 0xff, 0xfb,
    0xff, 0xfb, 0xbf, 0xff, 0x7f, 0xff, 0x77, 0xf7, 0xbd, 0xef, 0xff,
    0xf0, 0xff, 0xf0, 0x0f, 0xfc, 0xcc, 0x3c, 0xcc, 0x33, 0xcc, 0xcf,
    0xff, 0xef, 0xff, 0xee, 0xff, 0xfd, 0xff, 0xfd, 0xdf, 0xff, 0xbf,
    0xff, 0xbb, 0xff, 0xf7, 0xff, 0xf7, 0x7f, 0x7b, 0xde,
};

static const u8 tuning_blk_pattern_8bit[128] = {
    0xff, 0xff, 0x00, 0xff, 0xff, 0xff, 0x00, 0x00, 0xff, 0xff, 0xcc,
    0xcc, 0xcc, 0x33, 0xcc, 0xcc, 0xcc, 0x33, 0x33, 0xcc, 0xcc, 0xcc,
    0xff, 0xff, 0xff, 0xee, 0xff, 0xff, 0xff, 0xee, 0xee, 0xff,

    0xff, 0xff, 0xdd, 0xff, 0xff, 0xff, 0xdd, 0xdd, 0xff, 0xff, 0xff,
    0xbb, 0xff, 0xff, 0xff, 0xbb, 0xbb, 0xff, 0xff, 0xff, 0x77, 0xff,
    0xff, 0xff, 0x77, 0x77, 0xff, 0x77, 0xbb, 0xdd, 0xee, 0xff,

    0xff, 0xff, 0xff, 0x00, 0xff, 0xff, 0xff, 0x00, 0x00, 0xff, 0xff,
    0xcc, 0xcc, 0xcc, 0x33, 0xcc, 0xcc, 0xcc, 0x33, 0x33, 0xcc, 0xcc,
    0xcc, 0xff, 0xff, 0xff, 0xee, 0xff, 0xff, 0xff, 0xee, 0xee,

    0xff, 0xff, 0xff, 0xdd, 0xff, 0xff, 0xff, 0xdd, 0xdd, 0xff, 0xff,
    0xff, 0xbb, 0xff, 0xff, 0xff, 0xbb, 0xbb, 0xff, 0xff, 0xff, 0x77,
    0xff, 0xff, 0xff, 0x77, 0x77, 0xff, 0x77, 0xbb, 0xdd, 0xee,
};

int SDMMC_Tuning (FSdPsu_T *sdPtr, int no_transfer)
{
    int ret;
    struct sdmmc_cmd cmd;
    struct sdmmc_data data;
    struct sdmmc_card *card;
    const u8 *tuning_block_pattern;
    int size;

    ALLOC_CACHE_ALIGN_BUFFER(u8, buf, 128);

    card = &(sdPtr->card[0]);

    if (card->bus_width == 1)
    {
        return 0;
    }

    // send cmd19
    cmd.idx = SDMMC_CMD_SEND_TUNING_BLOCK;
    cmd.arg = 0;
    cmd.resp_type = SDMMC_RESP_R1;
    cmd.flags = 0;

    data.flags = SDMMC_FLDATA_READ;
    if (no_transfer)
    {
        data.flags |= SDMMC_FLDATA_NOTRANSFER;
    }
    data.blocksize = 64;
    data.blocks = 1;
    data.buf = buf;

    tuning_block_pattern = tuning_blk_pattern_4bit;
    size = 64;

    if ((card->card_type & SDMMC_TYPE_MMC) && (card->mode == MMC_HS200))
    {
        cmd.idx = SDMMC_CMD_SEND_TUNING_BLOCK_HS200;
        if (card->bus_width == 8)
        {
            data.blocksize = 128;
            tuning_block_pattern = tuning_blk_pattern_8bit;
            size = 128;
        }
    }

    ret = FSdPsu_Host_SendCmd(sdPtr, &cmd, &data);
    if (ret)
    {
        return ret;
    }

    if (!no_transfer)
    {
        ret = memcmp(buf, tuning_block_pattern, size);
        return ret;
    }

    return 0;
}
#endif /* (SDMMC_CONFIG_TUNING_SUPPORT == 1)  */

static int SDMMC_SetBlockLen (FSdPsu_T *sdPtr, int len)
{
    int ret;
    struct sdmmc_cmd cmd = {0};

    if ((sdPtr->card->mode == UHS_DDR50) || (sdPtr->card->mode == MMC_HS52_DDR))
    {
        return 0;
    }

    cmd.idx = SDMMC_CMD_SET_BLOCKLEN;  // CMD16
    cmd.arg = len;
    cmd.resp_type = SDMMC_RESP_R1;
    cmd.flags = 0;
    ret = FSdPsu_Host_SendCmd(sdPtr, &cmd, NULL);

    return ret;
}

__attribute__((unused)) static int SDMMC_SetBlockCnt (FSdPsu_T *sdPtr, int cnt)
{
    int ret;
    struct sdmmc_cmd cmd = {0};

    cmd.idx = SDMMC_CMD_SET_BLOCK_COUNT;  // CMD23
    cmd.arg = cnt;
    cmd.resp_type = SDMMC_RESP_R1;
    cmd.flags = 0;
    ret = FSdPsu_Host_SendCmd(sdPtr, &cmd, NULL);

    return ret;
}

static int SDMMC_Cmd55 (FSdPsu_T *sdPtr)
{
    int ret;
    struct sdmmc_cmd cmd = {0};
    struct sdmmc_card *card;

    card = &(sdPtr->card[0]);

    cmd.idx = SDMMC_CMD_APP_CMD;  // CMD55
    cmd.arg = card->rca << 16;
    cmd.resp_type = SDMMC_RESP_R1;
    cmd.flags = 0;
    ret = FSdPsu_Host_SendCmd(sdPtr, &cmd, NULL);

    return ret;
}

static unsigned int SDMMC_ReadBlocks (FSdPsu_T *sdPtr, unsigned int start,
                                      unsigned int blkcnt, void *dst)
{
    int ret;
    struct sdmmc_cmd cmd = {0};
    struct sdmmc_data data = {0};
    struct sdmmc_card *card;

    card = &(sdPtr->card[0]);

    if (blkcnt == 0)
    {
        return 0;
    }

    if (blkcnt == 1)
    {
        cmd.idx = SDMMC_CMD_READ_SINGLE_BLOCK;
    }
    else
    {
        cmd.idx = SDMMC_CMD_READ_MULTIPLE_BLOCK;
    }

    if (card->high_capacity)
    {
        cmd.arg = start;  // block unit
    }
    else
    {
        cmd.arg = start * SDMMC_MAX_BLOCK_LEN;  // byte unit
    }

    cmd.resp_type = SDMMC_RESP_R1;
    cmd.flags = 0;

    data.buf = dst;
    data.blocksize = SDMMC_MAX_BLOCK_LEN;
    data.blocks = blkcnt;
    data.flags = SDMMC_FLDATA_READ;

    ret = FSdPsu_Host_SendCmd(sdPtr, &cmd, &data);

    if (((data.flags & SDMMC_FLDATA_AUTOCMD) == 0) && (blkcnt > 1))
    {
        // manually stop transmission
        SDMMC_StopTransmission(sdPtr, 0);
    }

    if (ret)
    {
        return 0;
    }

    return blkcnt;
}

static unsigned int FSdPsu_WriteBlocks (FSdPsu_T *sdPtr, unsigned int start,
                                        unsigned int blkcnt, void *src)
{
    int ret;
    int timeout = 1000;
    struct sdmmc_cmd cmd = {0};
    struct sdmmc_data data = {0};
    struct sdmmc_card *card;

    card = &(sdPtr->card[0]);

    if (blkcnt == 0)
    {
        return 0;
    }

    if (blkcnt == 1)
    {
        cmd.idx = SDMMC_CMD_WRITE_SINGLE_BLOCK;
    }
    else
    {
        cmd.idx = SDMMC_CMD_WRITE_MULTIPLE_BLOCK;
    }

    if (card->high_capacity)
    {
        cmd.arg = start;
    }
    else
    {
        cmd.arg = start * SDMMC_MAX_BLOCK_LEN;
    }

    cmd.resp_type = SDMMC_RESP_R1;
    cmd.flags = 0;

    data.buf = src;
    data.blocksize = SDMMC_MAX_BLOCK_LEN;
    data.blocks = blkcnt;
    data.flags = 0;

    ret = FSdPsu_Host_SendCmd(sdPtr, &cmd, &data);

    if (((data.flags & SDMMC_FLDATA_AUTOCMD) == 0) && (blkcnt > 1))
    {
        // manually stop transmission
        SDMMC_StopTransmission(sdPtr, 0);
    }


    if (ret)
    {
        return 0;
    }
    // wait for ready status
    ret = SDMMC_WaitReady(sdPtr, timeout);   //26us

    if (ret)
    {
        return 0;
    }

    return blkcnt;
}

static int SDMMC_WaitReady (FSdPsu_T *sdPtr, int timeout)
{
    int ret;
    u32 status;

    while (1)
    {
        ret = SDMMC_SendStatus(sdPtr, &status);
        if (ret)
        {
            return ret;
        }

        if ((status & SDMMC_STATUS_RDY_FOR_DATA) &&
            ((status & SDMMC_STATUS_CURR_STATE) != SDMMC_STATE_PRG))
        {
            break;
        }

        if (status & SDMMC_STATUS_MASK)
        {
            return FMSH_FAILURE;
        }

        if (timeout-- <= 0)
        {
            return FMSH_ETIME;
        }

        delay_ms(1);
    }

    return 0;
}

/******************** SD cmd seq ***************************/

static int SD_Switch (FSdPsu_T *sdPtr, unsigned int mode, int group, u32 value,
                      void *resp)
{
    int ret;
    struct sdmmc_cmd cmd;
    struct sdmmc_data data;
    struct sdmmc_card *card;

    card = &(sdPtr->card[0]);
    // SD version 1.0 does not support CMD6
    if (card->version == SD_VERSION_1_0)
    {
        return 0;
    }

    cmd.idx = SDMMC_CMD_SWITCH_FUNC;  // CMD6
    cmd.arg = (mode << 31) | 0xffffff;
    cmd.arg &= ~(0xf << (group * 4));
    cmd.arg |= value << (group * 4);
    cmd.resp_type = SDMMC_RESP_R1;
    cmd.flags = 0;

    data.flags = SDMMC_FLDATA_READ;
    data.buf = resp;
    data.blocksize = 64;
    data.blocks = 1;

    ret = FSdPsu_Host_SendCmd(sdPtr, &cmd, &data);

    return ret;
}

static int SD_SendIfCond (FSdPsu_T *sdPtr)
{
    int ret;
    struct sdmmc_cmd cmd = {0};
    struct sdmmc_card *card;

    card = &(sdPtr->card[0]);

    cmd.idx = SDMMC_CMD_SEND_IF_COND;  // CMD8
    cmd.arg = 0x1AA;                   // assume supply voltage is 2.7~3.3V
    cmd.resp_type = SDMMC_RESP_R7;
    cmd.flags = 0;
    ret = FSdPsu_Host_SendCmd(sdPtr, &cmd, NULL);
    if (ret)
    {
        return ret;
    }

    if ((cmd.resp[0] & 0xff) != 0xAA)
    {
        return FMSH_EIO;
    }
    else
    {
        card->version = SD_VERSION_2_0;
    }

    return 0;
}

static int SD_SendOpCond (FSdPsu_T *sdPtr, unsigned int uhs_en)
{
    int ret;
    int timeout = 1000;
    struct sdmmc_cmd cmd = {0};
    struct sdmmc_card *card;

    card = &(sdPtr->card[0]);

    // check card size and voltage.
    cmd.idx = SDMMC_ACMD_SEND_OP_COND;  // ACMD41
    // ask for voltage support
    cmd.arg = card->voltages & 0xff8000;
    if (card->version == SD_VERSION_2_0)
    {
        cmd.arg |= OCR_HCS;
    }
    if (uhs_en)
    {
        cmd.arg |= OCR_S18R;
    }
    cmd.resp_type = SDMMC_RESP_R3;
    cmd.flags = 0;

    while (1)
    {
        ret = SDMMC_Cmd55(sdPtr);
        if (ret)
        {
            return ret;
        }

        ret = FSdPsu_Host_SendCmd(sdPtr, &cmd, NULL);
        if (ret)
        {
            return ret;
        }

        if (cmd.resp[0] & OCR_BUSY)
        {
            break;
        }

        if (timeout-- <= 0)
        {
            return FMSH_ETIME;
        }

        delay_ms(1);
    }

    if (card->version != SD_VERSION_2_0)
    {
        card->version = SD_VERSION_1_0;
    }

    card->ocr = cmd.resp[0];
    if (uhs_en && ((card->ocr & 0x41000000) == 0x41000000))
    {
        ret = SD_SwitchVoltage(sdPtr);
        if (ret)
        {
            return ret;
        }
    }

    card->high_capacity = ((card->ocr & OCR_HCS) == OCR_HCS);
    card->rca = 1;

    return 0;
}

static int SD_SwitchVoltage (FSdPsu_T *sdPtr)
{
    int ret;
    struct sdmmc_cmd cmd = {0};
    struct sdmmc_card *card = &(sdPtr->card[0]);

    // check S18A if support switch
    if (!(card->ocr & OCR_S18R))
    {
        return 0;
    }

    // send cmd11 to switch card voltage
    cmd.idx = SDMMC_CMD_SWITCH_UHS18V;
    cmd.arg = 0;
    cmd.resp_type = SDMMC_RESP_R1;
    ret = FSdPsu_Host_SendCmd(sdPtr, &cmd, NULL);
    if (ret)
    {
        return ret;
    }

    if (cmd.resp[0] & SDMMC_STATUS_ERROR)
    {
        return FMSH_EIO;
    }

    // disable clock
    FSdPsu_Host_SetClock(sdPtr, 0);
    // check DAT=0000
    ret = FSdPsu_Host_CheckDAT(sdPtr, SDMMC_DAT_0000, 100);
    if (ret == FMSH_ENOSYS)
    {
        delay_us(100);
    }
    else if (ret)
    {
        return ret;
    }
    else
    {
        ; /* no deal with */
    }

    // switch voltage
    card->bus_voltage = SDMMC_BVS_180;
    card->mode = UHS_SDR12;
    ret = FSdPsu_Host_SetBusSpeed(sdPtr, card->mode);
    if (ret)
    {
        return ret;
    }

    // wait for 10 ms to finish switch
    delay_ms(10);

    // enaable clock
    FSdPsu_Host_SetClock(sdPtr, card->freq);

    // wait 1ms
    delay_ms(1);

    // check DAT[3:0]==1111b
    ret = FSdPsu_Host_CheckDAT(sdPtr, SDMMC_DAT_1111, 100);
    if (ret == FMSH_ENOSYS)
    {
        delay_us(100);
    }
    else if (ret)
    {
        return ret;
    }
    else
    {
        ; /* no deal with */
    }
    return 0;
}

static int SD_AvailBusWidthAndSpeed (FSdPsu_T *sdPtr, int *width, int *mode)
{
    struct sdmmc_card *card;
    int width_int, mode_int;

    card = &(sdPtr->card[0]);

    /* get the suitable mdoe according to speed */
    if (card->bus_voltage == SDMMC_BVS_180)
    {
        if ((card->caps & SDMMC_CAPS_SDR104) &&
            (sdPtr->config.input_clock_hz > 100000000))
        {
            mode_int = UHS_SDR104;
        }
        else if ((card->caps & SDMMC_CAPS_SDR50) &&
                 (sdPtr->config.input_clock_hz > 50000000))
        {
            mode_int = UHS_SDR50;
        }
        else if ((card->caps & SDMMC_CAPS_SDR25) &&
                 (sdPtr->config.input_clock_hz > 25000000))
        {
            mode_int = UHS_SDR25;
        }
        else
        {
            mode_int = UHS_SDR12;
        }
    }
    else
    {
        if ((card->caps & SDMMC_CAPS_HS) &&
            (sdPtr->config.input_clock_hz > 25000000))
        {
            mode_int = SD_HS;
        }
        else
        {
            mode_int = SDMMC_DS;
        }
    }

    if (card->caps & SDMMC_CAPS_BUS_4BIT)
    {
        width_int = 4;
    }
    else
    {
        width_int = 1;
    }

    if (mode)
    {
        *mode = mode_int;
    }
    if (width)
    {
        *width = width_int;
    }

    return 0;
}

static int SD_SetBusWidth (FSdPsu_T *sdPtr, unsigned int width)
{
    int ret;
    struct sdmmc_cmd cmd;
    struct sdmmc_card *card;

    card = &(sdPtr->card[0]);

    // change card bus width
    ret = SDMMC_Cmd55(sdPtr);
    if (ret)
    {
        return ret;
    }

    cmd.idx = SDMMC_ACMD_SET_BUS_WIDTH;  // ACMD6
    cmd.arg = 0;
    if (width == SDMMC_BUS_4BIT)
    {
        cmd.arg = 0x2;
    }
    cmd.resp_type = SDMMC_RESP_R1;
    cmd.flags = 0;
    ret = FSdPsu_Host_SendCmd(sdPtr, &cmd, NULL);
    if (ret)
    {
        return ret;
    }

    // change host bus width
    card->bus_width = width;
    ret = FSdPsu_Host_SetBusWidth(sdPtr, card->bus_width);
    if (ret)
    {
        return ret;
    }

    return 0;
}

static int SD_SetBusSpeedMode (FSdPsu_T *sdPtr, unsigned int mode)
{
    int ret = 0;
    u32 freq = 0, freq_max = 0, access_mode = 0;
    // u16 switch_status[32] = {0};
    struct sdmmc_card *card;

    ALLOC_CACHE_ALIGN_BUFFER(u16, switch_status, 32);

    card = &(sdPtr->card[0]);

    switch (mode)
    {
    case SDMMC_DS:
        access_mode = SDMMC_ACCESS_MODE_DEFAULT;
        freq = 25000000;
        fmsh_print_dbg("SDMMC: DS mode selected\r\n");
        break;
    case SD_HS:
        access_mode = SDMMC_ACCESS_MODE_HS;
        freq = 50000000;
        fmsh_print_dbg("SDMMC: HS mode selected\r\n");
        break;
    case UHS_SDR12:
        access_mode = SDMMC_ACCESS_MODE_DEFAULT;
        freq = 25000000;
        fmsh_print_dbg("SDMMC: SDR12 mode selected\r\n");
        break;
    case UHS_SDR25:
        access_mode = SDMMC_ACCESS_MODE_HS;
        freq = 50000000;
        fmsh_print_dbg("SDMMC: SDR25 mode selected\r\n");
        break;
    case UHS_SDR50:
        access_mode = SDMMC_ACCESS_MODE_SDR50;
        freq = 100000000;
        fmsh_print_dbg("SDMMC: SDR50 mode selected\r\n");
        break;
    case UHS_DDR50:
        access_mode = SDMMC_ACCESS_MODE_DDR50;
        freq = 50000000;
        fmsh_print_dbg("SDMMC: DDR50 mode selected\r\n");
        break;
    case UHS_SDR104:
        access_mode = SDMMC_ACCESS_MODE_SDR104;
        freq = 208000000;
        fmsh_print_dbg("SDMMC: SDR104 mode selected\r\n");
        break;
    default:
        return FMSH_EINVAL;
    }

    if (sdPtr->usercfg->sdclk_max != 0)
    {
        freq_max = sdPtr->usercfg->sdclk_max;
        if (freq > freq_max)
        {
            freq = freq_max;
        }
    }

    // SD version 1.0 does not support CMD6
    ret = SD_Switch(sdPtr, SDMMC_SWITCH_FUNC, SDMMC_SWITCH_GROUP1, access_mode,
                    (u8 *)switch_status);
    if (ret)
    {
        return ret;
    }

    if ((switch_status[8] & 0xf) != access_mode)
    {
        return FMSH_ENOSYS;
    }

    // change host speed mode
    // disable clock
    FSdPsu_Host_SetClock(sdPtr, 0);

    // switch speed mode
    card->mode = mode;
    ret = FSdPsu_Host_SetBusSpeed(sdPtr, card->mode);
    if (ret)
    {
        return ret;
    }

    // enaable clock
    card->freq = freq;
    ret = FSdPsu_Host_SetClock(sdPtr, card->freq);
    if (ret)
    {
        return ret;
    }

    ret = FSdPsu_Phy_Config(sdPtr, card->mode);
    if (ret)
    {
        return ret;
    }

    return 0;
}

static int SD_GetCapabilities (FSdPsu_T *sdPtr)
{
    int ret;
    int retry, i;
    u32 mode, unit;
    float tran_speed;
    u32 scr[2];
    u16 switch_status;
    struct sdmmc_cmd cmd;
    struct sdmmc_data data;
    struct sdmmc_card *card;

    ALLOC_CACHE_ALIGN_BUFFER(u32, scr_be, 2);
    ALLOC_CACHE_ALIGN_BUFFER(u16, switch_status_be, 32);

    card = &(sdPtr->card[0]);
    (void)memset(switch_status_be, 0, 64);

    card->caps = SDMMC_CAPS_DS | SDMMC_CAPS_BUS_1BIT;

    // decode cid info
    card->cid_decode.mid = get_resp_field(card->cid, 120, 8);
    card->cid_decode.oid = get_resp_field(card->cid, 104, 16);
    card->cid_decode.pnm[0] = get_resp_field(card->cid, 64, 8);
    card->cid_decode.pnm[1] = get_resp_field(card->cid, 72, 8);
    card->cid_decode.pnm[2] = get_resp_field(card->cid, 80, 8);
    card->cid_decode.pnm[3] = get_resp_field(card->cid, 88, 8);
    card->cid_decode.pnm[4] = get_resp_field(card->cid, 96, 8);
    card->cid_decode.prv = get_resp_field(card->cid, 56, 8);
    card->cid_decode.psn = get_resp_field(card->cid, 32, 32);
    card->cid_decode.year = get_resp_field(card->cid, 12, 8);
    card->cid_decode.month = get_resp_field(card->cid, 8, 4);

    // decode csd info
    card->csd_decode.csd_struct = get_resp_field(card->csd, CSD_STRUCTURE, 2);

    card->csd_decode.taac = get_resp_field(card->csd, CSD_TAAC, 8);
    card->csd_decode.nsac = get_resp_field(card->csd, CSD_NSAC, 8);

    card->csd_decode.tran_rate_unit = get_resp_field(card->csd, CSD_TRAN_SPEED,
                                                     3);
    card->csd_decode.tran_speed_value = get_resp_field(card->csd,
                                                       CSD_TRAN_SPEED + 3, 5);
    switch (card->csd_decode.tran_rate_unit)
    {
    case 0x0:
        unit = 100000;
        break;
    case 0x1:
        unit = 1000000;
        break;
    case 0x2:
        unit = 10000000;
        break;
    case 0x3:
        unit = 100000000;
        break;
    default:
        break;
    }
    switch (card->csd_decode.tran_speed_value)
    {
    case 0x0:
        tran_speed = 0;
        break;
    case 0x1:
        tran_speed = 1.0;
        break;
    case 0x2:
        tran_speed = 1.2;
        break;
    case 0x3:
        tran_speed = 1.3;
        break;
    case 0x4:
        tran_speed = 1.5;
        break;
    case 0x5:
        tran_speed = 2.0;
        break;
    case 0x6:
        tran_speed = 2.5;
        break;
    case 0x7:
        tran_speed = 3.0;
        break;
    case 0x8:
        tran_speed = 3.5;
        break;
    case 0x9:
        tran_speed = 4.0;
        break;
    case 0xa:
        tran_speed = 4.5;
        break;
    case 0xb:
        tran_speed = 5.0;
        break;
    case 0xc:
        tran_speed = 5.5;
        break;
    case 0xd:
        tran_speed = 6.0;
        break;
    case 0xe:
        tran_speed = 7.0;
        break;
    case 0xf:
        tran_speed = 8.0;
        break;
    default:
        break;
    }
    card->max_trans_rate = (unsigned int)(tran_speed * unit);

    card->csd_decode.ccc = get_resp_field(card->csd, CSD_CCC, 12);

    card->csd_decode.read_bl_len = get_resp_field(card->csd, CSD_READ_BL_LEN,
                                                  4);
    card->csd_decode.read_bl_partial = get_resp_field(card->csd,
                                                      CSD_READ_BL_PARTIAL, 1);
    card->csd_decode.read_blk_misalign = get_resp_field(
        card->csd, CSD_READ_BLK_MISALIGN, 1);
    card->read_blk_len = 0x1 << card->csd_decode.read_bl_len;

    card->csd_decode.write_bl_len = get_resp_field(card->csd, CSD_WRITE_BL_LEN,
                                                   4);
    card->csd_decode.write_bl_partial = get_resp_field(card->csd,
                                                       CSD_WRITE_BL_PARTIAL, 1);
    card->csd_decode.write_bl_misalign = get_resp_field(
        card->csd, CSD_WRITE_BLK_MISALIGN, 1);
    card->write_blk_len = 0x1 << card->csd_decode.write_bl_len;

    card->csd_decode.r2w_factor = get_resp_field(card->csd, CSD_R2W_FACTOR, 3);

    card->csd_decode.file_fomat = get_resp_field(card->csd, CSD_FILE_FORMAT, 2);

    if (card->csd_decode.csd_struct == 0x0)
    {
        u32 mult;
        // CSD Version 1.0
        card->csd_decode.c_size = get_resp_field(card->csd, 62, 12);
        card->csd_decode.c_size_mult = get_resp_field(card->csd, 47, 3);
        mult = 0x1 << (card->csd_decode.c_size_mult + 2);

        card->device_size = ((unsigned long long)card->csd_decode.c_size + 1) *
                            mult * card->read_blk_len;
    }
    else if (card->csd_decode.csd_struct == 0x1)
    {
        // CSD Version 2.0
        card->csd_decode.c_size = get_resp_field(card->csd, 48, 22);
        card->csd_decode.c_size_mult = 0;

        card->device_size = ((unsigned long long)card->csd_decode.c_size + 1) *
                            512 * 1024;
    }
    else
    {
        fmsh_print_err("SDMMC: CSD Version is not supported\r\n");
        return FMSH_FAILURE;
    }
    card->block_max = card->device_size / SDMMC_MAX_BLOCK_LEN;

    // get scr registrer
    ret = SDMMC_Cmd55(sdPtr);
    if (ret)
    {
        return ret;
    }

    cmd.idx = SDMMC_ACMD_SEND_SCR;  // ACMD51
    cmd.arg = 0;
    cmd.resp_type = SDMMC_RESP_R1;
    cmd.flags = 0;

    data.flags = SDMMC_FLDATA_READ;
    data.buf = (void *)scr_be;
    data.blocksize = 8;
    data.blocks = 1;

    ret = FSdPsu_Host_SendCmd(sdPtr, &cmd, &data);
    if (ret)
    {
        return ret;
    }

    scr[0] = be_to_cpu32(scr_be[0]);
    scr[1] = be_to_cpu32(scr_be[1]);

    fmsh_print_dbg("SDMMC: SCR register is\r\n");
    for (i = 0; i < 2; i++)
    {
        fmsh_print_dbg("- 0x%08x\r\n", scr[i]);
    }

    // get sd version by SD_SPEC
    switch ((scr[0] >> 24) & 0xf)
    {
    case 0:
        card->version = SD_VERSION_1_0;
        break;
    case 1:
        card->version = SD_VERSION_1_10;
        break;
    case 2:
        card->version = SD_VERSION_2_0;
        if (scr[0] >> 15)
        {
            card->version = SD_VERSION_3_0;
        }
        break;
    default:
        card->version = SD_VERSION_1_0;
        break;
    }

    // get sd bus width by SD_BUS_WIDTH
    if ((scr[0] >> 16) & 0x1)
    {
        card->caps |= SDMMC_CAPS_BUS_4BIT;
    }

    // SD VERSION 1.0 not support cmd6
    if (card->version == SD_VERSION_1_0)
    {
        return 0;
    }

    retry = 4;
    while (retry--)
    {
        ret = SD_Switch(sdPtr, SDMMC_SWITCH_CHECK, 0, 1, switch_status_be);
        if (ret)
        {
            return ret;
        }

        // check if hs function is busy
        switch_status = be_to_cpu16(switch_status_be[14]);
        if ((switch_status & 0x2) == 0)
        {
            break;
        }
    }

    // get sd speed cap
    switch_status = be_to_cpu16(switch_status_be[6]);
    if (switch_status & 0x2)
    {
        card->caps |= SDMMC_CAPS_HS;
    }

    if (sdPtr->usercfg->flags & SDMMC_F_UHS_SUPPORT)
    {
        // Version before 3.0 don't support UHS modes
        if (card->version < SD_VERSION_3_0)
        {
            return 0;
        }

        if ((card->ocr & OCR_S18R) == 0)
        {
            return 0;
        }

        switch_status = be_to_cpu16(switch_status_be[6]);
        mode = switch_status & 0x1f;
        if (mode & 0x1)
        {
            card->caps |= SDMMC_CAPS_SDR12;
        }
        if (mode & 0x2)
        {
            card->caps |= SDMMC_CAPS_SDR25;
        }
        if (mode & 0x4)
        {
            card->caps |= SDMMC_CAPS_SDR50;
        }
        if (mode & 0x8)
        {
            card->caps |= SDMMC_CAPS_SDR104;
        }
        if (mode & 0x10)
        {
            card->caps |= SDMMC_CAPS_DDR50;
        }
    }

    return 0;
}

static int SD_Init (FSdPsu_T *sdPtr)
{
    int ret;
    int uhs_en = 0;
    int retry;
    struct sdmmc_card *card;

    card = &(sdPtr->card[0]);
    card->card_type = SDMMC_TYPE_SD;

    if (sdPtr->config.has_buspwr)
    {
        uhs_en = (sdPtr->usercfg->flags & SDMMC_F_UHS_SUPPORT) ? 1 : 0;

        // if power cycle is not supported, it is not able to recover from an
        // error during UHS initialization
        ret = FSdPsu_Host_SetPower(sdPtr, SDMMC_POWER_CYCLE);
        if (ret)
        {
            fmsh_print_dbg(
                "SDMMC: Unable to do a full power cycle. It is recommanded to "
                "disable the UHS modes(SD) for safety\r\n");
            ret = FSdPsu_Host_SetPower(sdPtr, SDMMC_POWER_ON);
            if (ret)
            {
                return ret;
            }
        }
    }

    retry = 2;
    while (retry > 0)
    {
        retry = retry - 1;

        FSdPsu_Host_DefaultIOs(sdPtr);

        // reset card
        ret = SDMMC_GoIdleState(sdPtr);
        if (ret)
        {
            fmsh_print_err("SDMMC: Failed CMD0, ret=%d\r\n", ret);
            return ret;
        }

        // test for SD version 2.0
        ret = SD_SendIfCond(sdPtr);
        if (ret)
        {
            fmsh_print_err(
                "SDMMC: Failed CMD8, ret=%d. SD2.0 is not supported.\r\n", ret);
        }

        // confirm voltage and check whether mmc card
        ret = SD_SendOpCond(sdPtr, uhs_en);
        if (ret && uhs_en)
        {
            fmsh_print_err(
                "SDMMC: Card did not respond to voltage select! ret=%d\r\n",
                ret);
            uhs_en = 0;
            (void)FSdPsu_Host_SetPower(sdPtr, SDMMC_POWER_CYCLE);
            continue;
        }

        // send_op_cond no response, it is not sd card
        if (ret == SDMMC_ENORESP)
        {
            fmsh_print_err(
                "SDMMC: This is not a SD card. Please check card type.\r\n");
            return SDMMC_ENORESP;
        }

        break;
    }

    return 0;
}

/******************** MMC cmd seq ***************************/
static u8 golden_csd[8];

static int MMC_Switch (FSdPsu_T *sdPtr, int index, u32 value)
{
    int ret;
    int timeout_ms = 500;
    int retry = 10;
    struct sdmmc_cmd cmd;

    cmd.idx = SDMMC_CMD_SWITCH;
    cmd.arg = (SDMMC_SWITCH_MODE_WRITE_BYTE << 24) | (index << 16) |
              (value << 8);
    cmd.resp_type = SDMMC_RESP_R1b;
    cmd.flags = 0;

    do
    {
        ret = FSdPsu_Host_SendCmd(sdPtr, &cmd, NULL);
    } while (ret && (retry-- > 0));

    if (ret)
    {
        return ret;
    }

    // wait for dat0 ready
    ret = FSdPsu_Host_CheckDAT(sdPtr, SDMMC_DAT_xxx1, timeout_ms * 1000);
    if (ret == FMSH_ENOSYS)
    {
        delay_ms(timeout_ms);
    }
    else if (ret)
    {
        return ret;
    }
    else
    {
        ; /* no deal with */
    }
    return 0;
}

static int MMC_SendOpCond (FSdPsu_T *sdPtr)
{
    int ret;
    int timeout = 1000;
    struct sdmmc_cmd cmd;
    struct sdmmc_card *card;

    card = &(sdPtr->card[0]);

    cmd.idx = SDMMC_CMD_SEND_OP_COND;  // CMD1
    cmd.arg = 0;
    cmd.resp_type = SDMMC_RESP_R3;
    cmd.flags = 0;
    ret = FSdPsu_Host_SendCmd(sdPtr, &cmd, NULL);
    if (ret)
    {
        return ret;
    }

    card->ocr = cmd.resp[0];

    cmd.idx = SDMMC_CMD_SEND_OP_COND;  // CMD1
    cmd.arg = (card->voltages & (card->ocr & 0xff8080)) |
              (card->ocr & 0x60000000) | OCR_HCS;
    cmd.resp_type = SDMMC_RESP_R3;
    cmd.flags = 0;
    while (1)
    {
        ret = FSdPsu_Host_SendCmd(sdPtr, &cmd, NULL);
        if (ret)
        {
            return ret;
        }

        if (cmd.resp[0] & OCR_BUSY)
        {
            break;
        }

        timeout--;
        if (timeout <= 0)
        {
            return FMSH_ETIME;
        }

        delay_ms(1);
    }

    card->ocr = cmd.resp[0];

    card->high_capacity = ((card->ocr & OCR_HCS) == OCR_HCS);
    card->rca = 2;

    return 0;
}

__attribute__((unused)) static int MMC_SendExtCsd (FSdPsu_T *sdPtr, u8 *ext_csd)
{
    int ret;
    struct sdmmc_cmd cmd;
    struct sdmmc_data data;

    // get ext_csd
    cmd.idx = SDMMC_CMD_SEND_EXT_CSD;
    cmd.arg = 0;
    cmd.resp_type = SDMMC_RESP_R1;
    cmd.flags = 0;

    data.flags = SDMMC_FLDATA_READ;
    data.buf = ext_csd;
    data.blocksize = 512;
    data.blocks = 1;

    ret = FSdPsu_Host_SendCmd(sdPtr, &cmd, &data);
    if (ret)
    {
        return ret;
    }

    return FMSH_SUCCESS;
}

__attribute__((unused)) static int MMC_TestExtCsd (FSdPsu_T *sdPtr)
{
    int ret;
    struct sdmmc_card *card;

    card = &(sdPtr->card[0]);

    ALLOC_CACHE_ALIGN_BUFFER(u8, test_csd, 512);

    if (card->version < MMC_VERSION_4)
    {
        return 0;
    }

    ret = MMC_SendExtCsd(sdPtr, test_csd);
    if (ret)
    {
        return ret;
    }

    /* Only compare read only fields */
    if (golden_csd[0] == test_csd[EXT_CSD_PARTITIONING_SUPPORT] &&
        golden_csd[1] == test_csd[EXT_CSD_HC_WP_GRP_SIZE] &&
        golden_csd[2] == test_csd[EXT_CSD_REV] &&
        golden_csd[3] == test_csd[EXT_CSD_HC_ERASE_GRP_SIZE] &&
        memcmp(&golden_csd[4], &test_csd[EXT_CSD_SEC_CNT], 4) == 0)
    {
        return 0;
    }

    return FMSH_FAILURE;
}

static int MMC_AvailBusWidthAndSpeed (FSdPsu_T *sdPtr, int *width, int *mode)
{
    struct sdmmc_card *card;
    int width_int, mode_int;

    card = &(sdPtr->card[0]);

    if ((card->caps & SDMMC_CAPS_HS200) &&
        (sdPtr->config.input_clock_hz > 50000000))
    {
        mode_int = MMC_HS200;
    }
    else if ((card->caps & SDMMC_CAPS_HS52) &&
             (sdPtr->config.input_clock_hz > 25000000))
    {
        mode_int = MMC_HS52;
    }
    else if (card->caps & SDMMC_CAPS_HS26)
    {
        mode_int = MMC_HS26;
    }
    else
    {
        mode_int = SDMMC_DS;
    }

    if ((card->caps & SDMMC_CAPS_BUS_8BIT) && (sdPtr->config.bus_width == 8))
    {
        width_int = SDMMC_BUS_8BIT;
    }
    else if (card->caps & SDMMC_CAPS_BUS_4BIT)
    {
        width_int = SDMMC_BUS_4BIT;
    }
    else
    {
        width_int = SDMMC_BUS_1BIT;
    }

    if (mode)
    {
        *mode = mode_int;
    }
    if (width)
    {
        *width = width_int;
    }

    return 0;
}

static int MMC_SetBusWidth (FSdPsu_T *sdPtr, unsigned int width)
{
    int ret;
    u32 busmode;
    struct sdmmc_card *card;

    card = &(sdPtr->card[0]);

    switch (width)
    {
    case SDMMC_BUS_1BIT:
        busmode = 0x0;
        break;
    case SDMMC_BUS_4BIT:
        busmode = 0x1;
        break;
    case SDMMC_BUS_8BIT:
        busmode = 0x2;
        break;
    case SDMMC_BUS_4BIT_DDR:
        busmode = 0x5;
        break;
    case SDMMC_BUS_8BIT_DDR:
        busmode = 0x6;
        break;
    default:
        return FMSH_EIO;
    }

    ret = MMC_Switch(sdPtr, EXT_CSD_BUS_WIDTH, busmode);
    if (ret)
    {
        return ret;
    }

    // change host bus width
    card->bus_width = width & 0xf;
    ret = FSdPsu_Host_SetBusWidth(sdPtr, card->bus_width);
    if (ret)
    {
        return ret;
    }

    return 0;
}

static int MMC_SetBusSpeedMode (FSdPsu_T *sdPtr, unsigned int mode)
{
    int ret;
    u32 freq = 0, freq_max = 0, speed = 0;
    struct sdmmc_card *card;

    card = &(sdPtr->card[0]);

    switch (mode)
    {
    case SDMMC_DS:
        speed = 0x0;
        freq = 25000000;
        fmsh_print_dbg("SDMMC: DS mode selected\r\n");
        break;
    case MMC_HS26:
        speed = 0x1;
        freq = 25000000;
        fmsh_print_dbg("SDMMC: HS26 mode selected\r\n");
        break;
    case MMC_HS52:
        speed = 0x1;
        freq = 50000000;
        fmsh_print_dbg("SDMMC: HS52 mode selected\r\n");
        break;
    case MMC_HS52_DDR:
        speed = 0x1;
        freq = 50000000;
        fmsh_print_dbg("SDMMC: DDR52 mode selected\r\n");
        break;
    case MMC_HS200:
        speed = 0x2;
        freq = 200000000;
        fmsh_print_dbg("SDMMC: HS200 mode selected\r\n");
        break;
    case MMC_HS400:
        speed = 0x3;
        freq = 400000000;
        fmsh_print_dbg("SDMMC: HS400 mode selected\r\n");
        break;
    default:
        return FMSH_EIO;
    }

    if (sdPtr->usercfg->sdclk_max != 0)
    {
        freq_max = sdPtr->usercfg->sdclk_max;
        if (freq > freq_max)
        {
            freq = freq_max;
        }
    }

    ret = MMC_Switch(sdPtr, EXT_CSD_HS_TIMING, speed);
    if (ret)
    {
        return ret;
    }

    // change host speed mode
    // disable clock
    FSdPsu_Host_SetClock(sdPtr, 0);

    // switch speed mode
    card->mode = mode;
    ret = FSdPsu_Host_SetBusSpeed(sdPtr, card->mode);
    if (ret)
    {
        return ret;
    }

    // enaable clock
    card->freq = freq;
    ret = FSdPsu_Host_SetClock(sdPtr, card->freq);
    if (ret)
    {
        return ret;
    }

    ret = FSdPsu_Phy_Config(sdPtr, card->mode);
    if (ret)
    {
        return ret;
    }

    return 0;
}

static int MMC_GetCapabilities (FSdPsu_T *sdPtr)
{
    int ret;
    u32 unit, mult;
    ;
    float tran_speed;
    struct sdmmc_cmd cmd;
    struct sdmmc_data data;
    struct sdmmc_card *card;

    ALLOC_CACHE_ALIGN_BUFFER(u8, ext_csd, 512);

    card = &(sdPtr->card[0]);
    card->caps = SDMMC_CAPS_DS | SDMMC_CAPS_BUS_1BIT | SDMMC_CAPS_BUS_4BIT |
                 SDMMC_CAPS_BUS_8BIT;

    // decode cid info
    card->cid_decode.mid = get_resp_field(card->cid, 120, 8);
    card->cid_decode.cbx_mmc = get_resp_field(card->cid, 112, 2);
    card->cid_decode.oid = get_resp_field(card->cid, 104, 8);
    card->cid_decode.pnm[0] = get_resp_field(card->cid, 56, 8);
    card->cid_decode.pnm[1] = get_resp_field(card->cid, 64, 8);
    card->cid_decode.pnm[2] = get_resp_field(card->cid, 72, 8);
    card->cid_decode.pnm[3] = get_resp_field(card->cid, 80, 8);
    card->cid_decode.pnm[4] = get_resp_field(card->cid, 88, 8);
    card->cid_decode.pnm[5] = get_resp_field(card->cid, 96, 8);
    card->cid_decode.prv = get_resp_field(card->cid, 48, 8);
    card->cid_decode.psn = get_resp_field(card->cid, 32, 32);
    card->cid_decode.year = get_resp_field(card->cid, 8, 4);
    card->cid_decode.month = get_resp_field(card->cid, 12, 4);

    // decode csd info
    card->csd_decode.csd_struct = get_resp_field(card->csd, CSD_STRUCTURE, 2);
    // get spec_vers from csd value
    switch (get_resp_field(card->csd, 122, 4))
    {
    case 0:
        card->version = MMC_VERSION_1_2;
        break;
    case 1:
        card->version = MMC_VERSION_1_4;
        break;
    case 2:
        card->version = MMC_VERSION_2_2;
        break;
    case 3:
        card->version = MMC_VERSION_3;
        break;
    case 4:
        card->version = MMC_VERSION_4;
        break;
    default:
        card->version = MMC_VERSION_1_2;
        break;
    }

    card->csd_decode.taac = get_resp_field(card->csd, CSD_TAAC, 8);
    card->csd_decode.nsac = get_resp_field(card->csd, CSD_NSAC, 8);

    card->csd_decode.tran_rate_unit = get_resp_field(card->csd, CSD_TRAN_SPEED,
                                                     3);
    card->csd_decode.tran_speed_value = get_resp_field(card->csd,
                                                       CSD_TRAN_SPEED + 3, 5);
    switch (card->csd_decode.tran_rate_unit)
    {
    case 0x0:
        unit = 100000;
        break;
    case 0x1:
        unit = 1000000;
        break;
    case 0x2:
        unit = 10000000;
        break;
    case 0x3:
        unit = 100000000;
        break;
    default:
        break;
    }
    switch (card->csd_decode.tran_speed_value)
    {
    case 0x0:
        tran_speed = 0;
        break;
    case 0x1:
        tran_speed = 1.0;
        break;
    case 0x2:
        tran_speed = 1.2;
        break;
    case 0x3:
        tran_speed = 1.3;
        break;
    case 0x4:
        tran_speed = 1.5;
        break;
    case 0x5:
        tran_speed = 2.0;
        break;
    case 0x6:
        tran_speed = 2.5;
        break;
    case 0x7:
        tran_speed = 3.0;
        break;
    case 0x8:
        tran_speed = 3.5;
        break;
    case 0x9:
        tran_speed = 4.0;
        break;
    case 0xa:
        tran_speed = 4.5;
        break;
    case 0xb:
        tran_speed = 5.0;
        break;
    case 0xc:
        tran_speed = 5.5;
        break;
    case 0xd:
        tran_speed = 6.0;
        break;
    case 0xe:
        tran_speed = 7.0;
        break;
    case 0xf:
        tran_speed = 8.0;
        break;
    default:
        break;
    }
    card->max_trans_rate = (unsigned int)(tran_speed * unit);

    card->csd_decode.ccc = get_resp_field(card->csd, CSD_CCC, 12);

    card->csd_decode.read_bl_len = get_resp_field(card->csd, CSD_READ_BL_LEN,
                                                  4);
    card->csd_decode.read_bl_partial = get_resp_field(card->csd,
                                                      CSD_READ_BL_PARTIAL, 1);
    card->csd_decode.read_blk_misalign = get_resp_field(
        card->csd, CSD_READ_BLK_MISALIGN, 1);
    card->read_blk_len = 0x1 << card->csd_decode.read_bl_len;

    card->csd_decode.write_bl_len = get_resp_field(card->csd, CSD_WRITE_BL_LEN,
                                                   4);
    card->csd_decode.write_bl_partial = get_resp_field(card->csd,
                                                       CSD_WRITE_BL_PARTIAL, 1);
    card->csd_decode.write_bl_misalign = get_resp_field(
        card->csd, CSD_WRITE_BLK_MISALIGN, 1);
    card->write_blk_len = 0x1 << card->csd_decode.write_bl_len;

    card->csd_decode.r2w_factor = get_resp_field(card->csd, CSD_R2W_FACTOR, 3);

    card->csd_decode.file_fomat = get_resp_field(card->csd, CSD_FILE_FORMAT, 2);

    card->csd_decode.c_size = get_resp_field(card->csd, 62, 12);
    card->csd_decode.c_size_mult = get_resp_field(card->csd, 47, 3);
    mult = 0x1 << (card->csd_decode.c_size_mult + 2);
    card->device_size = ((unsigned long long)card->csd_decode.c_size + 1) *
                        mult * card->read_blk_len;
    card->block_max = card->device_size / SDMMC_MAX_BLOCK_LEN;

    if (card->version < MMC_VERSION_4)
    {
        return 0;
    }

    // get ext_csd
    cmd.idx = SDMMC_CMD_SEND_EXT_CSD;
    cmd.arg = 0;
    cmd.resp_type = SDMMC_RESP_R1;
    cmd.flags = 0;

    data.flags = SDMMC_FLDATA_READ;
    data.buf = ext_csd;
    data.blocksize = 512;
    data.blocks = 1;

    ret = FSdPsu_Host_SendCmd(sdPtr, &cmd, &data);
    if (ret)
    {
        return ret;
    }

    switch (ext_csd[192])
    {
    case 0:
        card->version = MMC_VERSION_4;
        break;
    case 1:
        card->version = MMC_VERSION_4_1;
        break;
    case 2:
        card->version = MMC_VERSION_4_2;
        break;
    case 3:
        card->version = MMC_VERSION_4_3;
        break;
    case 4:
        card->version = MMC_VERSION_4_4;
        break;
    case 5:
        card->version = MMC_VERSION_4_41;
        break;
    case 6:
        card->version = MMC_VERSION_4_5;
        break;
    case 7:
        card->version = MMC_VERSION_5_0;
        break;
    case 8:
        card->version = MMC_VERSION_5_1;
        break;
    default:
        card->version = MMC_VERSION_4;
        break;
    }

    if (card->version >= MMC_VERSION_4_2)
    {
        card->device_size = (u64)(*(u32 *)(&ext_csd[212]) )* (u64)(512);
        card->block_max = card->device_size / SDMMC_MAX_BLOCK_LEN;
    }

    if (ext_csd[192] > 4)
    {
        card->cid_decode.year += 2013;
    }
    else
    {
        card->cid_decode.year += 1997;
    }

    if ((ext_csd[196]) & 0x1)
    {
        card->caps |= SDMMC_CAPS_HS26;
    }
    if ((ext_csd[196]) & 0x2)
    {
        card->caps |= SDMMC_CAPS_HS52;
    }
    if ((ext_csd[196]) & 0xc)
    {
        card->caps |= SDMMC_CAPS_HS52_DDR;
    }
    if ((ext_csd[196]) & 0x30)
    {
        card->caps |= SDMMC_CAPS_HS200;
    }
    if ((ext_csd[196]) & 0xc0)
    {
        card->caps |= SDMMC_CAPS_HS400;
    }

    // save read only ext_csd valur for compare
    golden_csd[0] = ext_csd[EXT_CSD_PARTITIONING_SUPPORT];
    golden_csd[1] = ext_csd[EXT_CSD_HC_WP_GRP_SIZE];
    golden_csd[2] = ext_csd[EXT_CSD_REV];
    golden_csd[3] = ext_csd[EXT_CSD_HC_ERASE_GRP_SIZE];
    memcpy(&golden_csd[4], &ext_csd[EXT_CSD_SEC_CNT], 4);

    return 0;
}

static int MMC_Init (FSdPsu_T *sdPtr)
{
    int ret;
    struct sdmmc_card *card;

    card = &(sdPtr->card[0]);
    card->card_type = SDMMC_TYPE_MMC;

    ret = FSdPsu_Host_SetPower(sdPtr, SDMMC_POWER_ON);
    if (ret)
    {
        return ret;
    }

    FSdPsu_Host_DefaultIOs(sdPtr);

    // reset card
    ret = SDMMC_GoIdleState(sdPtr);
    if (ret)
    {
        fmsh_print_err("SDMMC: Failed CMD0, ret=%d\r\n", ret);
        return ret;
    }

    ret = MMC_SendOpCond(sdPtr);
    if (ret)
    {
        fmsh_print_info(
            "SDMMC: Card did not respond to operation condition! ret=%d\r\n",
            ret);
        return ret;
    }

    return FMSH_SUCCESS;
}

/******************************************************************************/
int FSdPsu_SD_ChangeBusWidthAndSpeed (FSdPsu_T *sdPtr, int width, int mode)
{
    int ret;

    ret = SD_SetBusWidth(sdPtr, width);
    if (ret)
    {
        fmsh_print_err("SDMMC: Failed to set bus width, ret=%d\r\n", ret);
        return ret;
    }
    fmsh_print_dbg("SDMMC: Bus width is %d\r\n", width);

    ret = SD_SetBusSpeedMode(sdPtr, mode);
    if (ret)
    {
        fmsh_print_err("SDMMC: Failed to set bus speed mode, ret=%d\r\n", ret);
        return ret;
    }
    fmsh_print_dbg("SDMMC: Switch the clock to %dHz for data transfer\r\n",
                   sdPtr->host.sdclk);

#if (SDMMC_CONFIG_TUNING_SUPPORT == 1)
    if (mode == UHS_SDR104)
    {
        ret = FSdPsu_Host_ExecuteTuning(sdPtr);
        if (ret)
        {
            fmsh_print_err("SDMMC: Failed to execute tunning, ret=%d. Fallback to default phy configuration\r\n", ret);
            ret = FSdPsu_Phy_Config(sdPtr, mode);
            return ret;
        }
    }
#endif

    return 0;
}

int FSdPsu_MMC_ChangeBusWidthAndSpeed (FSdPsu_T *sdPtr, int width, int mode)
{
    int ret;
    struct sdmmc_card *card;

    card = &(sdPtr->card[0]);

    // set bus wdith without ddr
    ret = MMC_SetBusWidth(sdPtr, width & 0xf);
    if (ret)
    {
        fmsh_print_err("SDMMC: Failed to set bus width, ret=%d\r\n", ret);
        return ret;
    }
    fmsh_print_dbg("SDMMC: Bus width is %d\r\n", card->bus_width);

    ret = MMC_SetBusSpeedMode(sdPtr, mode & 0x1f);
    if (ret)
    {
        fmsh_print_err("SDMMC: Failed to set bus speed mode, ret=%d\r\n", ret);
        return ret;
    }
    fmsh_print_dbg("SDMMC: Switch the clock to %dHz for data transfer\r\n",
                   sdPtr->host.sdclk);

    if (width & SDMMC_DDR_MODE_MARK)
    {
        ret = MMC_SetBusWidth(sdPtr, width);
        if (ret)
        {
            fmsh_print_err("SDMMC: Failed to set bus width, ret=%d\r\n", ret);
            return ret;
        }
        fmsh_print_dbg("SDMMC: Bus width is %d in ddr mode\r\n",
                       card->bus_width);
    }

    if (mode & SDMMC_DDR_MODE_MARK)
    {
        ret = MMC_SetBusSpeedMode(sdPtr, mode);
        if (ret)
        {
            fmsh_print_err("SDMMC: Failed to set bus speed mode, ret=%d\r\n",
                           ret);
            return ret;
        }
    }

#if (SDMMC_CONFIG_TUNING_SUPPORT == 1)
    if (card->mode == MMC_HS200)
    {
        ret = FSdPsu_Host_ExecuteTuning(sdPtr);
        if (ret)
        {
            fmsh_print_err("SDMMC: Failed to execute tunning, ret=%d\r\n", ret);
            return ret;
        }
    }
#endif

    return 0;
}

int FSdPsu_CardDetect (FSdPsu_T *sdPtr)
{
    int ret;
    int i, width, mode;
    struct sdmmc_card *card;

    card = &(sdPtr->card[0]);

    // step1
    if (sdPtr->config.card_type == SDMMC_TYPE_SD)
    {
        card->voltages = 0xff80000;  // 2.7~3.3V by default
        ret = SD_Init(sdPtr);
        if (ret)
        {
            fmsh_print_err(
                "SDMMC: Failed to identificate sd card(step 1), ret=%d\r\n",
                ret);
            return ret;
        }
    }
    else if (sdPtr->config.card_type == SDMMC_TYPE_MMC)
    {
        card->voltages = 0xff8080;  // 1.7~3.6V
        ret = MMC_Init(sdPtr);
        if (ret)
        {
            fmsh_print_err(
                "SDMMC: Failed to identificate mmc card(step 1), ret=%d\r\n",
                ret);
            return ret;
        }
    }
    else
    {
        fmsh_print_err("SDMMC: Unknown card type\r\n");
        return FMSH_EINVAL;
    }

    // step2
    // CMD2: get CID, goto Identification State
    ret = SDMMC_AllSendCID(sdPtr);
    if (ret)
    {
        fmsh_print_err("SDMMC: Failed to Send CID, ret=%d\r\n", ret);
        return ret;
    }
    fmsh_print_dbg("SDMMC: CID register is\r\n");
    for (i = 0; i < 4; i++)
    {
        fmsh_print_dbg("- 0x%08x\r\n", card->cid[i]);
    }

    // CMD3: Get relative address, goto Stand-by State
    ret = SDMMC_SendRCA(sdPtr);
    if (ret)
    {
        fmsh_print_err("SDMMC: Failed to Send RCA, ret=%d\r\n", ret);
        return ret;
    }
    fmsh_print_dbg("SDMMC: RCA register is\r\n");
    fmsh_print_dbg("- 0x%x\r\n", card->rca);

    // CMD9: Get the Card-Specific Data
    ret = SDMMC_SendCSD(sdPtr);
    if (ret)
    {
        fmsh_print_err("SDMMC: Failed to Send CSD, ret=%d\r\n", ret);
        return ret;
    }
    fmsh_print_dbg("SDMMC: CSD register is\r\n");
    for (i = 0; i < 4; i++)
    {
        fmsh_print_dbg("- 0x%08x\r\n", card->csd[i]);
    }

    /*
    u8 dsr_imp = get_resp_field(card->csd, CSD_DSR_IMP, 1);
    if(dsr_imp) {
        ret = SDMMC_SetDSR(sdPtr, 0x404);
        if(ret) {
            fmsh_print_warning("SDMMC: Failed to Set DSR, ret=%d\r\n", ret);
        }
    }*/

    // CMD7: Select the card, and put it into Transfer Mode
    ret = SDMMC_SelectCard(sdPtr);
    if (ret)
    {
        fmsh_print_err("SDMMC: Failed to Select Card, ret=%d\r\n", ret);
        return ret;
    }

    // step3
    if (card->card_type == SDMMC_TYPE_SD)
    {
        ret = SD_GetCapabilities(sdPtr);
        if (ret)
        {
            fmsh_print_err("SDMMC: Failed to get sd capabilities, ret=%d\r\n",
                           ret);
            return ret;
        }

        (void)SD_AvailBusWidthAndSpeed(sdPtr, &width, &mode);

        // set bus width
        if (sdPtr->usercfg->force_bus_width)
        {
            fmsh_print_dbg("Warning: Bus width is forced to %d\r\n",
                           sdPtr->usercfg->force_bus_width);
            width = sdPtr->usercfg->force_bus_width;
        }

        // set bus speed mode
        if (sdPtr->usercfg->force_speed_mode)
        {
            fmsh_print_dbg("Warning: Bus speed mode is forced to %d\r\n",
                           sdPtr->usercfg->force_speed_mode);
            mode = sdPtr->usercfg->force_speed_mode;
        }

        ret = FSdPsu_SD_ChangeBusWidthAndSpeed(sdPtr, width, mode);
        if (ret)
        {
            return ret;
        }
    }
    else if (card->card_type == SDMMC_TYPE_MMC)
    {
        ret = MMC_GetCapabilities(sdPtr);
        if (ret)
        {
            fmsh_print_err("SDMMC: Failed to get mmc capabilities, ret=%d\r\n",
                           ret);
            return ret;
        }

        (void)MMC_AvailBusWidthAndSpeed(sdPtr, &width, &mode);

        // set bus wdith
        if (sdPtr->usercfg->force_bus_width)
        {
            fmsh_print_dbg("Warning: Bus width is forced to %d\r\n",
                           sdPtr->usercfg->force_bus_width);
            width = sdPtr->usercfg->force_bus_width;
        }

        // set bus speed mode
        if (sdPtr->usercfg->force_speed_mode)
        {
            fmsh_print_dbg("Warning: Bus speed mode is forced to %d\r\n",
                           sdPtr->usercfg->force_speed_mode);
            mode = sdPtr->usercfg->force_speed_mode;
        }

        ret = FSdPsu_MMC_ChangeBusWidthAndSpeed(sdPtr, width, mode);
        if (ret)
        {
            return ret;
        }
        /*
        // change bus wisth to 4 bits if 8 bits could not be used
        if((!sdPtr->usercfg->force_bus_width) && (width == SDMMC_BUS_8BIT))
        {
            ret = MMC_TestExtCsd(sdPtr);
            if(ret)
            {
                ret = FSdPsu_MMC_ChangeBusWidthAndSpeed(sdPtr, SDMMC_BUS_4BIT,
        mode); if (ret)
                {
                    return ret;
                }
            }
        }*/
    }
    else
    {
        ; /* no deal with */
    }
    
//    ret = SDMMC_SetBlockLen(sdPtr, SDMMC_MAX_BLOCK_LEN);
//    if (ret)
//    {
//        return ret;
//    }

    fmsh_print_dbg("SDMMC: Card Initialization succeed\r\n");

    return FMSH_SUCCESS;
}

int FSdPsu_CardInit (FSdPsu_T *sdPtr, FSdPsu_UserCfg_T *usercfg)
{
    int ret;

    FMSH_ASSERT(sdPtr != NULL);

    // check if card has already been initialized
    if (sdPtr->is_inited)
    {
        return 0;
    }

    // do basic initialization for sdPtr and host
    ret = FSdPsu_Host_InitHw(sdPtr, usercfg);
    if (ret == 0)
    {
        // card identification
        ret = FSdPsu_CardDetect(sdPtr);
        if (ret == 0)
        {
            sdPtr->is_inited = 1;
            return FMSH_SUCCESS;
        }
        else
        {
            fmsh_print_err("SDMMC: Failed to identificate card, ret=%d\r\n",
                           ret);
        }
    }
    else
    {
        fmsh_print_err("SDMMC: Failed to  preinit host, ret=%d\r\n", ret);
    }

    // init fail
    sdPtr->is_inited = 0;
    (void)FSdPsu_Host_SetPower(sdPtr, 0);

    return ret;
}

int FSdPsu_Bread (FSdPsu_T *sdPtr, unsigned int start, unsigned int blkcnt,
                  unsigned char *dst)
{
    int ret;
    unsigned int blk_remain, blk_read;
    unsigned char *buf;
    struct sdmmc_card *card;
    char *prbuf = NULL;
    unsigned char *prbuf_AlignStart;
    static int remalloc_flag = 0;
    u32 cahcelinesize = 64;

    FMSH_ASSERT(sdPtr != NULL);
    FMSH_ASSERT(dst != NULL);
#if !NO_OS
    if (emmc_lock(100) != FMSH_SUCCESS) 
    {
        fmsh_print("%s, fail to lock eMMC\r\n", __func__);
        return FMSH_FAILURE;
    }
#endif
    card = &(sdPtr->card[0]);

    #if 1
    // fmsh_print("dst addr : %p\n", (void*)dst);
    if ( ((unsigned int)dst & 0x3) != 0 )
    {
        prbuf = (char *)malloc(blkcnt * SDMMC_MAX_BLOCK_LEN + 2 * cahcelinesize);
        if (NULL == prbuf)
        {
            fmsh_print("prbuf malloc err\r\n");
            goto unlock_and_return;
        }

        prbuf_AlignStart = (char *)(((long long)prbuf + cahcelinesize) &
                                (~((long long)cahcelinesize - 1)));
        buf = prbuf_AlignStart;

        remalloc_flag = 1;

    }
    else
    {
        buf = dst;
    }
    #endif

    //buf = dst;

    if ((start + blkcnt) > card->block_max)
    {
        fmsh_print_err("SDMMC: End block num 0x%d exceeds max 0x%d\r\n",
                       start + blkcnt, card->block_max);
        
        if(prbuf != NULL )
        {
            free(prbuf);
        }
        goto unlock_and_return;
    }

    ret = SDMMC_SetBlockLen(sdPtr, SDMMC_MAX_BLOCK_LEN);
    if (ret)
    {
        if(prbuf != NULL )
        {
            free(prbuf);
        }
        goto unlock_and_return;
    }

    blk_remain = blkcnt;
    blk_read = blkcnt > SDMMC_MAX_BLOCK_CNT ? SDMMC_MAX_BLOCK_CNT : blkcnt;
    while (blk_remain > 0)
    {
        ret = SDMMC_ReadBlocks(sdPtr, start, blk_read, buf);
        if (ret != blk_read)
        {
            if(prbuf != NULL )
            {
                free(prbuf);
            }
        }

        if (remalloc_flag)
        {
            memcpy(dst, buf, blkcnt * SDMMC_MAX_BLOCK_LEN);
        }
        blk_remain -= blk_read;
        start += blk_read;
        buf += blk_read * SDMMC_MAX_BLOCK_LEN;
        if (remalloc_flag)
        {
            dst += blk_read * SDMMC_MAX_BLOCK_LEN;
        }
    }

    if(prbuf != NULL )
    {
        free(prbuf);
    }
#if !NO_OS
    emmc_unlock();
#endif
    return blkcnt;

unlock_and_return:
#if !NO_OS
    emmc_unlock();
#endif
    return FMSH_FAILURE;
}

int FSdPsu_Bwrite (FSdPsu_T *sdPtr, unsigned int start, unsigned int blkcnt,
                   unsigned char *src)
{
    int ret;
    unsigned int blk_remain, blk_write;
    unsigned char *buf;
    struct sdmmc_card *card;
    char *pwbuf = NULL;
    char *pwbuf_AlignStart;
    u32 cahcelinesize = 64;

    FMSH_ASSERT(sdPtr != NULL);
    FMSH_ASSERT(src != NULL);
#if !NO_OS
    if (emmc_lock(100) != FMSH_SUCCESS) 
    {
        fmsh_print("%s, fail to lock eMMC\r\n", __func__);
        return FMSH_FAILURE;
    }
#endif

    card = &(sdPtr->card[0]);
    #if 1
    if ( ((unsigned int)src & 0x3) != 0 )
    {
        pwbuf = (char *)malloc(blkcnt * SDMMC_MAX_BLOCK_LEN + 2 * cahcelinesize);
        if (NULL == pwbuf)
        {
            fmsh_print("pwbuf malloc err\r\n");
            goto unlock_and_return;
        }

        pwbuf_AlignStart = (char *)(((long long)pwbuf + cahcelinesize) &
                                (~((long long)cahcelinesize - 1)));
        memcpy(pwbuf_AlignStart, src, blkcnt * SDMMC_MAX_BLOCK_LEN);
        
        buf = pwbuf_AlignStart;
    }
    else
    {
        buf = src;
        
    }
#endif 

    //buf = src;
    
    if ((start + blkcnt) > card->block_max)
    {
        fmsh_print_err("SDMMC: End block num 0x%d exceeds max 0x%d\r\n",
                       start + blkcnt, card->block_max);
        if(pwbuf != NULL )
        {
            free(pwbuf);
        }
        goto unlock_and_return;
    }

    ret = SDMMC_SetBlockLen(sdPtr, SDMMC_MAX_BLOCK_LEN);

    if (ret)
    {
        if(pwbuf != NULL )
        {
            free(pwbuf);
        }
        goto unlock_and_return;
    }

    blk_remain = blkcnt;
    blk_write = blkcnt > SDMMC_MAX_BLOCK_CNT ? SDMMC_MAX_BLOCK_CNT : blkcnt;

    while (blk_remain > 0)
    {
        ret = FSdPsu_WriteBlocks(sdPtr, start, blk_write, buf);
        if (ret != blk_write)
        {
            if(pwbuf != NULL )
            {
                free(pwbuf);
            }
            goto unlock_and_return;
        }
        blk_remain -= blk_write;
        start += blk_write;
        buf += blk_write * SDMMC_MAX_BLOCK_LEN;
    }   

    if(pwbuf != NULL )
    {
        free(pwbuf);
    }

#if !NO_OS
    emmc_unlock();
#endif
    return blkcnt;

unlock_and_return:
#if !NO_OS
    emmc_unlock();
#endif
    return FMSH_FAILURE;
}
