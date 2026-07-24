
#include <string.h>
#include <stdio.h>


#include "lwip/udp.h"
#include "FreeRTOS.h"
#include "task.h"
#include "sys.h"
#include "rk_udp.h"
#include "load_img.h"
#include "eeprom_api.h"
#include "fmsh_common_types.h"
#include "fmsh_gpio_common.h"
#include "fmsh_gpio_public.h"
#include "fmsh_sdmmc_example.h"

extern ip_addr_t ipaddr;
static struct udp_pcb *upcb;
FGpioPs_T gpio_bank0;
image_header_t version_header;

typedef void (*cmd_handler_t)(CHAR *msg_buff,RK_INFO_FORMAT *rsp_buf);

typedef struct
{
    uint16_t cmd_id;
    cmd_handler_t msg_handler;
}CMD_HANDLER_MAP;

UINT8 cp_reset_flag = 0;

HW_ID mw_get_hwid()
{
    HW_ID hwid;
    INT8 hwid0 , hwid1;
    
    FGpioPs_bank_init (FPAR_GPIOPS_0_DEVICE_ID, &gpio_bank0);
    FGpioPs_setBitDirection(&gpio_bank0, Gpio_bit_12, Gpio_input);
    
    hwid0 = FGpioPs_readBit(&gpio_bank0, Gpio_bit_16);
    hwid1 = FGpioPs_readBit(&gpio_bank0, Gpio_bit_17);
    
    hwid  = (hwid0 | (hwid1 << 1))& 0x3;
    
    return hwid;
}

void rk_get_hw_id(char * cmd_info,RK_INFO_FORMAT * resp_info) 
{
    (void)cmd_info;
    HW_ID cur_hw_id= mw_get_hwid();
    resp_info->cmd_type = CMD_RESP_OK;
    resp_info->cmd_context[0] = cur_hw_id;
}

void rk_set_eeprom(char * cmd_info,RK_INFO_FORMAT * resp_info) 
{
    EEPROM_CMD_INFO set_context;

    memset(&set_context,0x0,sizeof(set_context));
    memcpy(&set_context,cmd_info,sizeof(EEPROM_CMD_INFO));

    set_context.data_size %= 17;
    resp_info->cmd_type = CMD_RESP_OK;

    if (OK != eeprom_write_bytes(set_context.eeprom_addr, 2,set_context.data, set_context.data_size))
    {
        resp_info->cmd_type = CMD_RESP_ERROR;
    }
}

void rk_get_eeprom(CHAR * cmd_info,RK_INFO_FORMAT * resp_info) 
{
    (void)cmd_info;
    EEPROM_CMD_INFO get_context;

    memset(&get_context,0x0,sizeof(get_context));
    memcpy(&get_context,cmd_info,sizeof(EEPROM_CMD_INFO));

    resp_info->cmd_type = CMD_RESP_OK;
    get_context.data_size %= 17;

    if (OK != eeprom_read_bytes(get_context.eeprom_addr, 2,get_context.data, get_context.data_size))
    {
        resp_info->cmd_type = CMD_RESP_ERROR;
        return;
    }

    memcpy(resp_info->cmd_context,get_context.data,get_context.data_size);
}

void rk_get_version_system(char * cmd_info,RK_INFO_FORMAT * resp_info) 
{
    resp_info->cmd_type = CMD_RESP_OK;
    resp_info->cmd_context[0] = 0x1;
    char version_str[10];
    
    memset(version_str,0x0,sizeof(version_str));

    if (strlen(version_header.image_version) < 5)
    {
        resp_info->cmd_type = CMD_RESP_ERROR;
    }
    else 
    {
        memcpy(version_str,version_header.image_version,8);
    }

    memcpy(&resp_info->cmd_context[1],version_str,10);
}

void rk_set_cp_mode(char * cmd_info,RK_INFO_FORMAT * resp_info) 
{
    UINT8 cur_band;
    UINT8 request_band = cmd_info[0];
    resp_info->cmd_type = CMD_RESP_OK;

    if (OK != eeprom_read_bytes(EEPROM_BAND_INFO_ADDR, 2,&cur_band, 1))
    {
        resp_info->cmd_type = CMD_RESP_ERROR;
        return;
    }
    
    if (cur_band != request_band)
    {
        if (OK != eeprom_write_bytes(EEPROM_BAND_INFO_ADDR, 2,&request_band, 1))
        {
            resp_info->cmd_type = CMD_RESP_ERROR;
            return;
        }
        cp_reset_flag = 1;
    }
}

void system_software_reset()
{
    FMSH_WriteReg(FPS_CRL_APB_BASEADDR, 0x1c, 0x0);

    u32 reg = FMSH_ReadReg(FPS_CRL_APB_BASEADDR, 0x218);
    FMSH_WriteReg(FPS_CRL_APB_BASEADDR, 0x218, reg|0x10);
}

void rk_reset_cp(CHAR *cmd_info,RK_INFO_FORMAT *resp_info)
{
    (void)cmd_info;
    resp_info->cmd_type = CMD_RESP_OK;
    cp_reset_flag = 1;
}

CMD_HANDLER_MAP cmd_map[] = 
{
    {CMD_SET_EEPROM,          rk_set_eeprom},
    {CMD_GET_EEPROM,          rk_get_eeprom},
    {CMD_GET_HWID,            rk_get_hw_id},
    {CMD_SET_RESET,           rk_reset_cp},
    {CMD_GET_VESION_SYSTEM,   rk_get_version_system},
    {CMD_SET_CP_MODE,         rk_set_cp_mode},
};

static void rk_udp_receive_callback(void *arg, struct udp_pcb *upcb,
     struct pbuf *p, const ip_addr_t *addr, u16_t port)
{
    RK_INFO_FORMAT  recv_msg;
    RK_INFO_FORMAT  send_msg;
    memcpy(&recv_msg,p->payload,sizeof(RK_INFO_FORMAT));

    if (recv_msg.cmd_type < CMD_TYPE_MAX)
    {
        cmd_map[recv_msg.cmd_type].msg_handler(recv_msg.cmd_context,&send_msg);
    }
    else 
    {
        send_msg.cmd_type = CMD_RESP_ERROR;
        fmsh_print("Recv unknow cmd: %d, ignore\n", recv_msg.cmd_type);
    }
    
    memcpy(p->payload,&send_msg,sizeof(RK_INFO_FORMAT));
    
    udp_sendto(upcb, p, addr, port);
    pbuf_free(p);
    if(cp_reset_flag == 1)
    {
       vTaskDelay(100);
       system_software_reset();
    }
}

void rk_udp_server_init(void)
{
    err_t err;

    upcb = udp_new();

    if (upcb)
    {
        err = udp_bind(upcb, &ipaddr, SERVER_PORT);

        if(err == ERR_OK)
        {
            udp_recv(upcb, rk_udp_receive_callback, NULL);
        }
        else
        {
            udp_remove(upcb);
            
            fmsh_print("can not bind pcb\r\n");
        }
    }

    memset(&version_header,0x0,sizeof(image_header_t));
    emmc_read_image_header_version(0, &version_header);
}

























