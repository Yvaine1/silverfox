#include <string.h>
#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <errno.h>
#include "ant_link_mgr.h"

struct udp_pcb *host_udp;
ip_addr_t host_udp_ip, ant_udp_ip;
uint16_t host_udp_port;
uint16_t ant_udp_port;

static struct udp_pcb *web_udp;
static ip_addr_t web_udp_ip;
static uint16_t web_udp_port;
bool web_addr_valid;

// 初始化函数
void ant_link_init(void)
{
    web_addr_valid = false;
    IP4_ADDR(&host_udp_ip, 10, 255, 0, 2);
    IP4_ADDR(&ant_udp_ip, 10, 255, 0, 66);
    host_udp_port = 61111;
    ant_udp_port = 62222;
}

void ant_link_read_ant_udp_info(void)
{
    FILE *fp;
    char line[128];
    char key[64];
    char value[64];
    char *p;

    fp = fopen("log/ip_port_info.ini", "r");
    if (fp == NULL)
    {
        fmsh_print("log/ip_port_info.ini not found, create with default config\r\n");
        fp = fopen("log/ip_port_info.ini", "w");
        if (fp != NULL)
        {
            fprintf(fp, "HOST_UDP_IP=%s\n", ipaddr_ntoa(&host_udp_ip));
            fprintf(fp, "HOST_UDP_PORT=%d\n", host_udp_port);
            fprintf(fp, "ANT_UDP_IP=%s\n", ipaddr_ntoa(&ant_udp_ip));
            fprintf(fp, "ANT_UDP_PORT=%d\n", ant_udp_port);
            fclose(fp);
        }
        else
        {
            fmsh_print("Failed to create log/ip_port_info.ini\r\n");
        }
        return;
    }

    while (fgets(line, sizeof(line), fp) != NULL)
    {
        p = strchr(line, '\n');
        if (p) *p = '\0';
        p = strchr(line, '\r');
        if (p) *p = '\0';

        if (line[0] == '\0' || line[0] == '#' || line[0] == ';')
            continue;

        if (sscanf(line, "%63[^=]=%63s", key, value) == 2)
        {
            p = key + strlen(key) - 1;
            while (p > key && (*p == ' ' || *p == '\t'))
            {
                *p = '\0';
                p--;
            }
            p = key;
            while (*p == ' ' || *p == '\t') p++;

            if (strcmp(p, "HOST_UDP_IP") == 0)
            {
                ipaddr_aton(value, &host_udp_ip);
            }
            else if (strcmp(p, "HOST_UDP_PORT") == 0)
            {
                host_udp_port = (uint16_t)atoi(value);
            }
            else if (strcmp(p, "ANT_UDP_IP") == 0)
            {
                ipaddr_aton(value, &ant_udp_ip);
            }
            else if (strcmp(p, "ANT_UDP_PORT") == 0)
            {
                ant_udp_port = (uint16_t)atoi(value);
            }
        }
    }

    fclose(fp);
}

struct udp_pcb* get_ant_host_udp(void)
{
  return host_udp;
}

// 启动阶段
void ant_link_startup_phase(void)
{
    //ant_link_read_ant_udp_info();
    ant_link_mgr_create_ant_udp_socket();
    ant_link_mgr_create_web_udp_socket();
}

static void ant_link_mgr_handle_ant_udp_connection(void *arg, struct udp_pcb *upcb, struct pbuf *p, const ip_addr_t *addr, uint16_t port)
{
    if (p->len > 0)
    {
        fmsh_print("received ant msg Len: %d\r\n", p->len);
        ant_ctrl_parse_udp_ant_resp_msg((uint8_t *)p->payload, p->len);
    }
    pbuf_free(p);
}

// 创建天线UDP socket
int32_t ant_link_mgr_create_ant_udp_socket(void)
{
    err_t err;

    host_udp = udp_new();

    if (host_udp)
    {
        err = udp_bind(host_udp, &host_udp_ip, host_udp_port);

        if(err == ERR_OK)
        {
            udp_recv(host_udp, ant_link_mgr_handle_ant_udp_connection, NULL);
            return OK;
        }
        else
        {
            udp_remove(host_udp);
            host_udp = NULL;
            fmsh_print("can not bind pcb\r\n");
            return -1;
        }
    }
    fmsh_print("ant udp new failed\r\n");
    return -1;
}

int32_t ant_link_mgr_send_udp_msg_to_ant(char *pBuffer, int32_t bytesRead)
{
    if(NULL == pBuffer || bytesRead <= 0)
    {
        fmsh_print("p_msg is null or invalid length\r\n");
        return -1;
    }

    struct pbuf *p;
    // 使用 bytesRead 而不是 strlen(pBuffer)
    p = pbuf_alloc(PBUF_TRANSPORT, bytesRead, PBUF_POOL);

    if(p != NULL)
    {
        memcpy(p->payload, pBuffer, bytesRead);
 
        err_t err = udp_sendto(host_udp, p, &ant_udp_ip, ant_udp_port);
        if(err == ERR_OK) {
            fmsh_print("UDP send succeeded\r\n");
        } else {
            fmsh_print("UDP send failed: err=%d\r\n", err);
        }

        pbuf_free(p);
    }
    else
    {
        fmsh_print("pbuf_alloc failed\r\n");
        return -2;
    }
    return 0;
}

// Web UDP receive callback: store web address and forward message to antenna
static void ant_link_mgr_handle_web_udp_connection(void *arg, struct udp_pcb *upcb, struct pbuf *p, const ip_addr_t *addr, uint16_t port)
{
    if (p->len > 0) {
        fmsh_print("received web msg Len: %d\r\n", p->len);
        // Store web source address for response forwarding
        web_udp_ip = *addr;
        web_udp_port = port;
        web_addr_valid = true;
        // Parse web message (internal handles all forwarding)
        ant_ctrl_parse_udp_web_resp_msg((uint8_t *)p->payload, p->len);
    }
    pbuf_free(p);
}

// Create web UDP socket
int32_t ant_link_mgr_create_web_udp_socket(void)
{
    err_t err;
    ip_addr_t web_local_ip;

    IP4_ADDR(&web_local_ip, 10, 255, 0, 2);

    web_udp = udp_new();
    if (web_udp) {
        err = udp_bind(web_udp, &web_local_ip, UDP_WEB_PORT);
        if (err == ERR_OK) {
            udp_recv(web_udp, ant_link_mgr_handle_web_udp_connection, NULL);
            fmsh_print("web udp listening on port %d\r\n", UDP_WEB_PORT);
            return OK;
        }
        udp_remove(web_udp);
        fmsh_print("bind web udp failed\r\n");
        return -2;
    }
    fmsh_print("web udp new failed\r\n");
    return -1;
}

// Send message to web (using web_udp as source)
int32_t ant_link_mgr_send_udp_msg_to_web(char *pBuffer, int32_t bytesRead)
{
    if(NULL == pBuffer || bytesRead <= 0 || !web_addr_valid)
    {
        return -1;
    }

    struct pbuf *p = pbuf_alloc(PBUF_TRANSPORT, bytesRead, PBUF_POOL);

    if(p != NULL)
    {
        memcpy(p->payload, pBuffer, bytesRead);

        err_t err = udp_sendto(web_udp, p, &web_udp_ip, web_udp_port);
        pbuf_free(p);
        return (err == ERR_OK) ? 0 : -2;
    }
    return -2;
}

int32_t ant_link_close_ant_udp_socket(void)
{
    return OK;
}