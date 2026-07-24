#include <math.h>
#include "powermeter.h"
#include "fpga_reg.h"
void calc_powermeter()
{
    uint32_t pwr_val = FMSH_ReadReg(FPGA_BASEADDR, FPGA_TXPOWER_REG);
    if (pwr_val == 0)
    {
        pwr_val = 1;
    }
    float pwr_dbfs = 10 * log10(pwr_val) - 93.32;

    fmsh_print("Tx powr: %.2f (dbfs) (0x%x)\r\n", pwr_dbfs, pwr_val);

    pwr_val = FMSH_ReadReg(FPGA_BASEADDR, FPGA_RXPOWER_REG);
    if (pwr_val == 0)
    {
        pwr_val = 1;
    }
    pwr_dbfs = 10 * log10(pwr_val) - 93.32;
    fmsh_print("Rx powr: %.2f (dbfs) (0x%x)\r\n", pwr_dbfs, pwr_val);
}
