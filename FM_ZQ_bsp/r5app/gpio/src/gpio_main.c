#include "gpio_main.h"
#include "gpio_api.h"


void get_hw_id(void)
{
    HW_ID hw_id;
    hw_id = mw_get_hwid();
    switch (hw_id)
    {
        case HW_ID_00:
                fmsh_print("hw_id:00 yinHu V2\n");
                break;
        case HW_ID_01:
                fmsh_print("hw_id:01 LRM board\n");
                break;
        default:
                fmsh_print("Unknown Hardware Version=0x%x\n", hw_id);
                break;
    }
    return;
}