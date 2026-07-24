#include <LowLevelIOInterface.h>
#include <stddef.h>

#include "bspconfig.h"

#if USE_NO_SEMIHOST

#define UART_CONSOLE
#ifdef UART_CONSOLE
#include "fmsh_common.h"
#include "fmsh_psu_parameters.h"

void uart_putc_polled (int ch)
{
    while (!(FMSH_IN32_32((STDOUT_BASEADDRESS + 0x7C)) & 0x02));
    FMSH_OUT32_32(ch, STDOUT_BASEADDRESS);
}

#else
char console_buff[0x200];
int console_pos = 0;
#define uart_putc_polled(ch) (console_buff[console_pos++] = (char)ch)

#endif

long __lseek (int a, long b, int c) { return 0; }

int __close (int a) { return 0; }
/*
int remove()
{
    return 0;
}
*/

size_t __write (int handle, const unsigned char *buf, size_t bufSize)
{
    size_t nChars = 0; /* Check for the command to flush all handles */

    if (handle == -1)
    {
        return 0;
    }

    /* Check for stdout and stderr  (only necessary if FILE descriptors are
     * enabled.) */
    if (handle != 1 && handle != 2)
    {
        return -1;
    }

    while (bufSize > 0)
    {
        if (*buf == '\n')
        {
            uart_putc_polled('\r');
        }
        uart_putc_polled(*buf);

        ++buf;
        ++nChars;
        --bufSize;
    }

    return nChars;
}

size_t __dwrite (int handle, const unsigned char *buf, size_t bufSize)
{
    return __write(handle, buf, bufSize);
}

#endif  // end USE_NO_SEMIHOST
