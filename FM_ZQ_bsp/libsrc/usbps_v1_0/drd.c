#include "drd.h"

#define USB0_BASEADDR (0xFE200000U)
int cdns_get_id ()
{
    int id;

    id = FMSH_ReadReg(USB0_BASEADDR, OTGSTS) & OTGSTS_ID_VALUE;
    printf("OTG ID: %d", id);

    return id;
}

int cdns_get_vbus ()
{
    int vbus;

    vbus = FMSH_ReadReg(USB0_BASEADDR, OTGSTS) & OTGSTS_VBUS_VALID;
    printf("OTG VBUS: %d", vbus);

    return vbus;
}

u32 cdns_drd_init ()
{
    u32 state = 0;

    state = OTGSTS_STRAP(FMSH_ReadReg(USB0_BASEADDR, OTGSTS));
    return state;
}

void cdns_otg_enable_irq ()
{
    FMSH_WriteReg(USB0_BASEADDR, OTGIEN,
                  OTGIEN_ID_CHANGE_INT | OTGIEN_VBUSVALID_RISE_INT |
                      OTGIEN_VBUSVALID_FALL_INT);
}

/**
 * cdns_drd_gadget_on - start gadget.
 * @cdns: Pointer to controller context structure.
 *
 * Returns 0 on success otherwise negative errno
 */
int cdns_drd_gadget_on (void)
{
    u32 reg = OTGCMD_OTG_DIS;

    /* switch OTG core */
    FMSH_WriteReg(USB0_BASEADDR, OTGCMD, OTGCMD_DEV_BUS_REQ | reg);
    printf("Waiting till Device mode is turned on\n");

    do
    {
        reg = FMSH_ReadReg(USB0_BASEADDR, OTGSTS) & OTGSTS_CDNSP_DEV_READY;
        if (reg != 0)
        {
            break;
        }
    } while (1);

    return 0;
}

void cdns_drd_irq (void)
{
    u32 reg = FMSH_ReadReg(USB0_BASEADDR, OTGIVECT);

    if (!reg)
    {
        return;
    }

    if (reg & OTGIEN_ID_CHANGE_INT)
    {
        printf("OTG IRQ: new ID: %d\n", cdns_get_id());
    }

    if (reg & (OTGIEN_VBUSVALID_RISE_INT | OTGIEN_VBUSVALID_FALL_INT))
    {
        printf("OTG IRQ: new VBUS: %d\n", cdns_get_vbus());
    }
    FMSH_WriteReg(USB0_BASEADDR, OTGIVECT, ~0);
    return;
}
