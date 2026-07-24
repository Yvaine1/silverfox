#include "gpio_api.h"
#include "fmsh_gpio_public.h"
#include "fmsh_psu_parameters.h"

HW_ID mw_get_hwid(void)
{
    FGpioPs_T gpio0;
    enum FGpioPs_state hwid0;
    enum FGpioPs_state hwid1;
    HW_ID hwid;

    FGpioPs_bank_init(FPAR_GPIOPS_0_DEVICE_ID, &gpio0);     /*gpio bank0, mio 0-25*/
    FGpioPs_setBitDirection(&gpio0, Gpio_bit_16, Gpio_input);
    FGpioPs_setBitDirection(&gpio0, Gpio_bit_17, Gpio_input);


    hwid0 = FGpioPs_readBit(&gpio0,  Gpio_bit_16);
    hwid1 = FGpioPs_readBit(&gpio0,  Gpio_bit_17);
    fmsh_print("MIO %d, input data is : %x; MIO %d, input data is : %x;\r\n", Gpio_bit_16, hwid0, Gpio_bit_17, hwid1);

    hwid = ((hwid1 << 1) | hwid0) & 0x3;

    return hwid;
}