/***************************** Include Files *********************************/
#include <stdlib.h>
#include <string.h>

#include "fmsh_bbram_lib.h"
#include "fmsh_common.h"
#include "fmsh_psu_parameters.h"

/************************** Constant Definitions *****************************/
u32 Fmsh_AssertStatus;
#define FMSH_ASSERT_NONE     0U
#define FMSH_ASSERT_OCCURRED 1U
#define XSK_POLL_TIMEOUT     0xFFFFFFFFU
#define REVERSE_POLYNOMIAL   (0x82F63B78U)
#define Fmsh_AssertNonvoid(Expression)                \
    {                                                 \
        if (Expression)                               \
        {                                             \
            Fmsh_AssertStatus = FMSH_ASSERT_NONE;     \
        }                                             \
        else                                          \
        {                                             \
            Fmsh_AssertStatus = FMSH_ASSERT_OCCURRED; \
            return 0;                                 \
        }                                             \
    }
/************************** Function Prototypes ******************************/

/****************************************************************************/
/**
 * Checks whether the passed character is a valid hash key character
 *
 *
 * @param        c - Character to check proper value
 *
 * @return
 *			XST_SUCCESS	- In case of Success
 *			XST_FAILURE - In case of Failure
 ****************************************************************************/
u32 FmshSKey_Efuse_IsValidChar (const char *c)
{
    const char ValidChars[] = "0123456789abcdefABCDEF";
    const char *RetVal;
    u32 Status = (u32)FMSH_FAILURE;

    if (c == NULL)
    {
        Status = (u32)FMSH_FAILURE;
        goto END;
    }

    RetVal = strchr(ValidChars, (int)*c);
    if (RetVal != NULL)
    {
        Status = (u32)FMSH_SUCCESS;
    }

END:
    return Status;
}

/****************************************************************************/
/**
 * Validate the key for proper characters & proper length
 *
 *
 * @param        Key - Hash Key
 * @param        Len - Valid length of key
 *
 * @return
 *			XST_SUCCESS	- In case of Success
 *			XST_FAILURE - In case of Failure
 ****************************************************************************/
u32 FmshSKey_Efuse_ValidateKey (const char *Key, u32 Len)
{
    u32 i;
    u32 Status = (u32)FMSH_FAILURE;

    Fmsh_AssertNonvoid(Key != NULL);
    Fmsh_AssertNonvoid(
        (Len == XSK_STRING_SIZE_2) || (Len == XSK_STRING_SIZE_6) ||
        (Len == XSK_STRING_SIZE_8) || (Len == XSK_STRING_SIZE_64) ||
        (Len == XSK_STRING_SIZE_96));

    /**
     * Make sure the key has valid length
     */
    if (strlen(Key) != Len)
    {
        Status = ((u32)XSK_EFUSEPL_ERROR_KEY_VALIDATION |
                  (u32)XSK_EFUSEPL_ERROR_NOT_VALID_KEY_LENGTH);
        goto END;
    }

    /**
     * Make sure the key has valid characters
     */
    for (i = 0U; i < strlen(Key); i++)
    {
        if (FmshSKey_Efuse_IsValidChar(&Key[i]) != (u32)FMSH_SUCCESS)
        {
            Status = ((u32)XSK_EFUSEPL_ERROR_KEY_VALIDATION |
                      (u32)XSK_EFUSEPL_ERROR_NOT_VALID_KEY_CHAR);
            goto END;
        }
    }
    Status = (u32)XSK_EFUSEPL_ERROR_NONE;
END:
    return Status;
}

/****************************************************************************/
/**
 * Converts the char into the equivalent nibble.
 *	Ex: 'a' -> 0xa, 'A' -> 0xa, '9'->0x9
 *
 * @param InChar is input character. It has to be between 0-9,a-f,A-F
 * @param Num is the output nibble.
 * @return
 * 		- XST_SUCCESS no errors occurred.
 *		- XST_FAILURE an error when input parameters are not valid
 ****************************************************************************/
static u32 FmshSKey_EfusePs_ConvertCharToNibble (char InChar, u8 *Num)
{
    u32 Status = (u32)FMSH_FAILURE;
    /**
     * Convert the char to nibble
     */
    if ((InChar >= '0') && (InChar <= '9'))
    {
        *Num = (u8)InChar - (u8)'0';
    }
    else if ((InChar >= 'a') && (InChar <= 'f'))
    {
        *Num = (u8)InChar - (u8)'a' + 10U;
    }
    else if ((InChar >= 'A') && (InChar <= 'F'))
    {
        *Num = (u8)InChar - (u8)'A' + 10U;
    }
    else
    {
        Status = (u32)XSK_EFUSEPS_ERROR_STRING_INVALID;
        goto END;
    }
    Status = (u32)FMSH_SUCCESS;
END:
    return Status;
}

/****************************************************************************/
/**
 * Converts the string into the equivalent Hex buffer.
 *	Ex: "abc123" -> {0x23, 0xc1, 0xab}
 *
 * @param	Str is a Input String. Will support the lower and upper case values.
 * 		Value should be between 0-9, a-f and A-F
 *
 * @param	Buf is Output buffer.
 * @param	Len of the input string. Should have even values
 * @return
 * 		- FMSH_SUCCESS no errors occurred.
 *		- FMSH_FAILURE an error when input parameters are not valid
 *		- an error when input buffer has invalid values
 *
 *	TDD Test Cases:
    ---Initialization---
    Len is odd
    Len is zero
    Str is NULL
    Buf is NULL
    ---Functionality---
    Str input with only numbers
    Str input with All values in A-F
    Str input with All values in a-f
    Str input with values in a-f, 0-9, A-F
    Str input with values in a-z, 0-9, A-Z
    Boundary Cases
    Memory Bounds of buffer checking
  ****************************************************************************/
u32 FmshSKey_Efuse_ConvertStringToHexLE (const char *Str, u8 *Buf, u32 Len)
{
    u32 Status = (u32)FMSH_FAILURE;
    u32 ConvertedLen;
    u8 LowerNibble = 0U, UpperNibble = 0U;
    u32 StrIndex;

    /**
     * Check the parameters
     */
    if (Str == NULL)
    {
        Status = (u32)XSK_EFUSEPS_ERROR_PARAMETER_NULL;
        goto END;
    }

    if (Buf == NULL)
    {
        Status = (u32)XSK_EFUSEPS_ERROR_PARAMETER_NULL;
        goto END;
    }

    /**
     * Len has to be multiple of 2
     */
    if ((Len == 0U) || ((Len % 2U) == 1U))
    {
        Status = (u32)XSK_EFUSEPS_ERROR_PARAMETER_NULL;
        goto END;
    }

    if (Len != (strlen(Str) * 4U))
    {
        Status = (u32)XSK_EFUSEPS_ERROR_PARAMETER_NULL;
        goto END;
    }

    StrIndex = (Len / 8U) - 1U;
    // StrIndex = 0;
    ConvertedLen = 0U;
    while (ConvertedLen < (Len / 4U))
    {
        /**
         * Convert char to nibble
         */
        if (FmshSKey_EfusePs_ConvertCharToNibble(
                Str[ConvertedLen], &UpperNibble) == (u32)FMSH_SUCCESS)
        {
            /**
             * Convert char to nibble
             */
            if (FmshSKey_EfusePs_ConvertCharToNibble(
                    Str[ConvertedLen + 1U], &LowerNibble) == (u32)FMSH_SUCCESS)
            {
                /**
                 * Merge upper and lower nibble to Hex
                 */
                Buf[StrIndex] = (UpperNibble << 4U) | LowerNibble;
                StrIndex = StrIndex - 1U;
                // StrIndex = StrIndex + 1U;
            }
            else
            {
                /**
                 * Error converting Lower nibble
                 */
                Status = (u32)XSK_EFUSEPS_ERROR_STRING_INVALID;
                goto END;
            }
        }
        else
        {
            /**
             * Error converting Upper nibble
             */
            Status = (u32)XSK_EFUSEPS_ERROR_STRING_INVALID;
            goto END;
        }
        /**
         * Converted upper and lower nibbles
         */
        ConvertedLen += 2U;
    }
    Status = (u32)FMSH_SUCCESS;

END:
    return Status;
}

u32 FmshSKey_Efuse_ConvertStringToHex (const char *Str, u8 *Buf, u32 Len)
{
    u32 Status = (u32)FMSH_FAILURE;
    u32 ConvertedLen;
    u8 LowerNibble = 0U, UpperNibble = 0U;
    u32 StrIndex;

    /**
     * Check the parameters
     */
    if (Str == NULL)
    {
        Status = (u32)XSK_EFUSEPS_ERROR_PARAMETER_NULL;
        goto END;
    }

    if (Buf == NULL)
    {
        Status = (u32)XSK_EFUSEPS_ERROR_PARAMETER_NULL;
        goto END;
    }

    /**
     * Len has to be multiple of 2
     */
    if ((Len == 0U) || ((Len % 2U) == 1U))
    {
        Status = (u32)XSK_EFUSEPS_ERROR_PARAMETER_NULL;
        goto END;
    }

    if (Len != (strlen(Str) * 4U))
    {
        Status = (u32)XSK_EFUSEPS_ERROR_PARAMETER_NULL;
        goto END;
    }

    StrIndex = 0;
    ConvertedLen = 0U;
    while (ConvertedLen < (Len / 4U))
    {
        /**
         * Convert char to nibble
         */
        if (FmshSKey_EfusePs_ConvertCharToNibble(
                Str[ConvertedLen], &UpperNibble) == (u32)FMSH_SUCCESS)
        {
            /**
             * Convert char to nibble
             */
            if (FmshSKey_EfusePs_ConvertCharToNibble(
                    Str[ConvertedLen + 1U], &LowerNibble) == (u32)FMSH_SUCCESS)
            {
                /**
                 * Merge upper and lower nibble to Hex
                 */
                Buf[StrIndex] = (UpperNibble << 4U) | LowerNibble;
                StrIndex = StrIndex + 1U;
            }
            else
            {
                /**
                 * Error converting Lower nibble
                 */
                Status = (u32)XSK_EFUSEPS_ERROR_STRING_INVALID;
                goto END;
            }
        }
        else
        {
            /**
             * Error converting Upper nibble
             */
            Status = (u32)XSK_EFUSEPS_ERROR_STRING_INVALID;
            goto END;
        }
        /**
         * Converted upper and lower nibbles
         */
        ConvertedLen += 2U;
    }
    Status = (u32)FMSH_SUCCESS;

END:
    return Status;
}

/*****************************************************************************/
/**
 *
 * This function zeroize's Bbram Key.
 *
 * @note		BBRAM key will be zeroized.
 *
 ******************************************************************************/
u32 FMSHSKey_FMZQ_Bbram_Zeroise (void)
{
    u32 Status = (u32)FMSH_FAILURE;
    u32 Offset;
    u32 Timeout = 0U;

    /*
     * If we are not in programming mode for zeroizing immediately
     * without latency
     */
    FMSH_WriteReg(FPS_BBRAM_BASEADDR, XSK_BBRAM_CTRL_OFFSET,
                  XSK_BBRAM_CTRL_ZEROIZE_MASK);

    /*
     * Write all zeros to the data regs
     * before issuing a zeroize command. Otherwise, we
     * may hang waiting for zeroize complete bit if
     * we were already in programming mode
     */
    Offset = XSK_BBRAM_0_OFFSET;
    while (Offset <= XSK_BBRAM_7_OFFSET)
    {
        FMSH_WriteReg(FPS_BBRAM_BASEADDR, Offset, 0x0U);
        Offset = Offset + 4U;
    }

    /* Issue the zeroize command */
    FMSH_WriteReg(FPS_BBRAM_BASEADDR, XSK_BBRAM_CTRL_OFFSET,
                  XSK_BBRAM_CTRL_ZEROIZE_MASK);

    /* Wait for zeroize complete bit to get set */
    while (Timeout < XSK_POLL_TIMEOUT)
    {
        /* Read the status register */
        Status = FMSH_ReadReg(FPS_BBRAM_BASEADDR, XSK_BBRAM_STS_OFFSET);

        if ((Status & (u32)XSK_BBRAM_STS_ZEROIZED_MASK) != 0x00U)
        {
            Status = (u32)FMSH_SUCCESS;
            goto END;
        }
        Timeout = Timeout + 1U;
    }
END:
    return Status;
}

/*****************************************************************************/
/**
 *
 * This function enables programming and zeroizes Bbram.
 *
 * @return
 *		- Error code from XskFMZQ_Ps_Bbram_ErrorCodes enum if it fails
 *		- XST_SUCCESS if programming is done.
 *
 ******************************************************************************/
static INLINE u32 FMSHSKey_FMZQ_Bbram_PrgrmEn (void)
{
    u32 StatusRead = 0U;
    u32 Status = (u32)FMSH_FAILURE;
    u32 TimeOut = 0U;

    /*
     * Always issue a zeroize command (since we may
     * already be in programming mode and it may
     * hang waiting for zeroize complete bit)
     */
    Status = FMSHSKey_FMZQ_Bbram_Zeroise();
    if (Status != (u32)FMSH_SUCCESS)
    {
        Status = (u32)XSK_FMZQ_BBRAMPS_ERROR_IN_ZEROISE;
        goto END;
    }

    /* Enter programming mode */
    FMSH_WriteReg(FPS_BBRAM_BASEADDR, XSK_BBRAM_PGM_MODE_OFFSET,
                  XSK_BBRAM_PGM_MODE_SET_VAL);

    while (TimeOut < XSK_POLL_TIMEOUT)
    {
        /* check for zeroized */
        StatusRead = FMSH_ReadReg(FPS_BBRAM_BASEADDR, XSK_BBRAM_STS_OFFSET);

        if ((StatusRead & (u32)XSK_BBRAM_STS_ZEROIZED_MASK) != 0x00U)
        {
            break;
        }
        TimeOut = TimeOut + 1U;
    }

    if ((StatusRead & (u32)XSK_BBRAM_STS_ZEROIZED_MASK) == 0x00U)
    {
        Status = (u32)XSK_FMZQ_BBRAMPS_ERROR_IN_ZEROISE;
        goto END;
    }

    StatusRead = FMSH_ReadReg(FPS_BBRAM_BASEADDR, XSK_BBRAM_STS_OFFSET);

    if ((StatusRead & XSK_BBRAM_STS_PGM_MODE_MASK) !=
        XSK_BBRAM_STS_PGM_MODE_MASK)
    {
        Status = (u32)XSK_FMZQ_BBRAMPS_ERROR_IN_PRGRMG_ENABLE;
        goto END;
    }
END:
    return Status;
}
/****************************************************************************/
/**
 * Calculates CRC value for each row of AES key.
 *
 * @param	PrevCRC holds the prev row's CRC.
 * @param	Data holds the present row's key.
 * @param	Addr stores the current row number.
 *
 * @return	Crc of current row.
 *
 * @note	None.
 *
 ****************************************************************************/
u32 FmshSKey_RowCrcCalculation (u32 PrevCRC, u32 Data, u32 Addr)
{
    u32 Crc = PrevCRC;
    u32 Value = Data;
    u32 Row = Addr;
    u32 Index;

    for (Index = 0U; Index < 32U; Index++)
    {
        if ((((Value & 0x1U) ^ Crc) & 0x1U) != 0U)
        {
            Crc = ((Crc >> 1U) ^ REVERSE_POLYNOMIAL);
        }
        else
        {
            Crc = Crc >> 1U;
        }
        Value = Value >> 1U;
    }

    for (Index = 0U; Index < 5U; Index++)
    {
        if ((((Row & 0x1U) ^ Crc) & 0x1U) != 0U)
        {
            Crc = ((Crc >> 1U) ^ REVERSE_POLYNOMIAL);
        }
        else
        {
            Crc = Crc >> 1U;
        }
        Row = Row >> 1U;
    }

    return Crc;
}
/*****************************************************************************/
/**
 *
 * This function calculates CRC of AES key.
 *
 * @param	AesKey is a pointer to the key for which CRC has to be
 *		calculated.
 *
 * @return	CRC of AES key
 *
 ******************************************************************************/




u32 crc32 (const u8 *data, u32 length)
{
    u32 i, crc, j = 0;
    crc = 0xFFFFFFFF;

    while ((length--) != 0)
    {
        crc ^= (u32)data[j] << 24;
        j++;
        for (i = 0; i < 8; ++i)
        {
            if ((crc & 0x80000000) != 0)
            {
                crc = (crc << 1) ^ 0x04C11DB7;
            }
            else
            {
                crc <<= 1;
            }
        }
    }
    return crc;
}
/*****************************************************************************/
/**
 *
 * This function disables bbram programming.
 *
 ******************************************************************************/
static INLINE void FmshSKey_Bbram_PrgrmDisable (void)
{
    FMSH_WriteReg(FPS_BBRAM_BASEADDR, XSK_BBRAM_PGM_MODE_OFFSET,
                  XSK_BBRAM_PGM_MODE_RSTVAL);
}

/*****************************************************************************/
/**
 *
 * This function implements the BBRAM programming and verifying the key written.
 * Program and verification of AES will work only together.
 * CRC of the provided key will be calculated internally and verified after
 * programming.
 *
 * @param	AesKey	Pointer to the key which has to be programmed.
 *
 * @return
 * 		- Error code from XskFMZQ_Ps_Bbram_ErrorCodes enum if it fails
 * 		- XST_SUCCESS if programming is done.
 *
 ******************************************************************************/
u32 FmshSKey_Bbram_Program (u32 *AesKey)
{
    u32 Status = (u32)FMSH_FAILURE;
    u32 AesCrc;
    u32 *KeyPtr = AesKey;
    u32 StatusRead = 0U;
    u32 Offset;
    u32 TimeOut = 0U;

    /* Assert validates the input arguments */
    Fmsh_AssertNonvoid(AesKey != NULL);

    /* Set in programming mode */
    Status = FMSHSKey_FMZQ_Bbram_PrgrmEn();
    if (Status != (u32)FMSH_SUCCESS)
    {
        Status = (Status | (u32)XSK_FMZQ_BBRAMPS_ERROR_IN_PRGRMG);
        goto END;
    }

    /* Program with provided key and check key written */
    Offset = XSK_BBRAM_0_OFFSET;
    while (Offset <= XSK_BBRAM_7_OFFSET)
    {
        FMSH_WriteReg(FPS_BBRAM_BASEADDR, Offset, *KeyPtr);
        KeyPtr++;
        Offset = Offset + 4U;
    }

    /* Calculate CRC of AES */
    // AesCrc = FmshSKey_Bbram_CrcCalc(AesKey);
    AesCrc = crc32((u8 *)AesKey, 32);

    /*
    FMSH_WriteReg(FPS_BBRAM_BASEADDR,
        XSK_BBRAM_8_OFFSET, AesCrc);
    */

    FMSH_WriteReg(FPS_BBRAM_BASEADDR, XSK_BBRAM_AES_CRC_OFFSET, AesCrc);

    while (TimeOut < XSK_POLL_TIMEOUT)
    {
        /* Check for CRC done */
        StatusRead = FMSH_ReadReg(FPS_BBRAM_BASEADDR, XSK_BBRAM_STS_OFFSET);
        if ((StatusRead & (u32)XSK_BBRAM_STS_AES_CRC_DONE_MASK) != 0x00U)
        {
            break;
        }
        TimeOut = TimeOut + 1U;
    }

    if ((StatusRead & (u32)XSK_BBRAM_STS_AES_CRC_DONE_MASK) == 0x00U)
    {
        Status = (u32)XSK_FMZQ_BBRAMPS_ERROR_IN_WRITE_CRC;
        goto END;
    }

    if ((StatusRead & XSK_BBRAM_STS_AES_CRC_PASS_MASK) !=
        XSK_BBRAM_STS_AES_CRC_PASS_MASK)
    {
        Status = (u32)XSK_FMZQ_BBRAMPS_ERROR_IN_CRC_CHECK;
        goto END;
    }
END:
    FmshSKey_Bbram_PrgrmDisable();

    return Status;
}

u32 FmshSKey_Bbram_Program_v2 (
    u32 *AesKeyLE, u32 *AesKey)  // AesKey is used to calculate crc32, AesKeyLE
                                 // is written in registers
{
    u32 Status = (u32)FMSH_FAILURE;
    u32 AesCrc;
    u32 *KeyPtr = AesKeyLE;
    u32 StatusRead = 0U;
    u32 Offset;
    u32 TimeOut = 0U;

    /* Assert validates the input arguments */
    Fmsh_AssertNonvoid(AesKey != NULL);
    Fmsh_AssertNonvoid(AesKeyLE != NULL);

    /* Set in programming mode */
    Status = FMSHSKey_FMZQ_Bbram_PrgrmEn();
    if (Status != (u32)FMSH_SUCCESS)
    {
        Status = (Status | (u32)XSK_FMZQ_BBRAMPS_ERROR_IN_PRGRMG);
        goto END;
    }

    /* Program with provided key and check key written */
    Offset = XSK_BBRAM_0_OFFSET;
    while (Offset <= XSK_BBRAM_7_OFFSET)
    {
        FMSH_WriteReg(FPS_BBRAM_BASEADDR, Offset, *KeyPtr);
        KeyPtr++;
        Offset = Offset + 4U;
    }

    /* Calculate CRC of AES */
    AesCrc = crc32((u8 *)AesKey, 32);
    /*
    FMSH_WriteReg(FPS_BBRAM_BASEADDR,
        XSK_BBRAM_8_OFFSET, AesCrc);
    */
    // AesCrc=0;
    FMSH_WriteReg(FPS_BBRAM_BASEADDR, XSK_BBRAM_AES_CRC_OFFSET, AesCrc);

    while (TimeOut < XSK_POLL_TIMEOUT)
    {
        /* Check for CRC done */
        StatusRead = FMSH_ReadReg(FPS_BBRAM_BASEADDR, XSK_BBRAM_STS_OFFSET);
        if ((StatusRead & (u32)XSK_BBRAM_STS_AES_CRC_DONE_MASK) != 0x00U)
        {
            break;
        }
        TimeOut = TimeOut + 1U;
    }

    if ((StatusRead & (u32)XSK_BBRAM_STS_AES_CRC_DONE_MASK) == 0x00U)
    {
        Status = (u32)XSK_FMZQ_BBRAMPS_ERROR_IN_WRITE_CRC;
        goto END;
    }

    if ((StatusRead & XSK_BBRAM_STS_AES_CRC_PASS_MASK) !=
        XSK_BBRAM_STS_AES_CRC_PASS_MASK)
    {
        Status = (u32)XSK_FMZQ_BBRAMPS_ERROR_IN_CRC_CHECK;
        goto END;
    }
END:
    FmshSKey_Bbram_PrgrmDisable();

    return Status;
}

u32 FmshSKey_Bbram_CheckCRC (u32 *AesKey)
{
    u32 Status = (u32)FMSH_FAILURE;
    u32 StatusRead = 0U;
    u32 TimeOut = 0U;
    u32 AesCrc;
    AesCrc = crc32((u8 *)AesKey, 32);
    FMSH_WriteReg(FPS_BBRAM_BASEADDR, XSK_BBRAM_AES_CRC_OFFSET, AesCrc);

    while (TimeOut < XSK_POLL_TIMEOUT)
    {
        /* Check for CRC done */
        StatusRead = FMSH_ReadReg(FPS_BBRAM_BASEADDR, XSK_BBRAM_STS_OFFSET);
        if ((StatusRead & (u32)XSK_BBRAM_STS_AES_CRC_DONE_MASK) != 0x00U)
        {
            break;
        }
        TimeOut = TimeOut + 1U;
    }

    if ((StatusRead & (u32)XSK_BBRAM_STS_AES_CRC_DONE_MASK) == 0x00U)
    {
        Status = (u32)XSK_FMZQ_BBRAMPS_ERROR_IN_WRITE_CRC;
        goto END;
    }

    if ((StatusRead & XSK_BBRAM_STS_AES_CRC_PASS_MASK) !=
        XSK_BBRAM_STS_AES_CRC_PASS_MASK)
    {
        Status = (u32)XSK_FMZQ_BBRAMPS_ERROR_IN_CRC_CHECK;
        goto END;
    }
END:
    FmshSKey_Bbram_PrgrmDisable();
    return Status;
}
