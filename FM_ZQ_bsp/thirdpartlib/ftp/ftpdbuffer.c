#include <Fatfs15/ff.h>
#include <lwip/tcp.h>
#include <string.h>
#include <time.h>
#include <ctype.h>
#include "ftpd.h"
#include "fmsh_common.h"
#include "lwip/sys.h"

#include "ftpdbuffer.h"

unsigned int data_recv_count = 0;

u8_t ftpd_recv_buffer0[FTPD_FILE_BUFFER_LEN];
u8_t ftpd_recv_buffer1[FTPD_FILE_BUFFER_LEN];

u8_t *buffer0_pt = (u8_t *)ftpd_recv_buffer0;
u8_t *buffer1_pt = (u8_t *)ftpd_recv_buffer1;

u32_t buffer0_watermark = 0;
u32_t buffer1_watermark = 0;

u8 host_buffer = 0;

static void ftpd_data_recv_buffer_clear()
{
  buffer0_pt = (void *)ftpd_recv_buffer0;
  buffer1_pt = (void *)ftpd_recv_buffer1;
  buffer0_watermark = 0;
  buffer1_watermark = 0;
  host_buffer = 0;
}


static err_t ftpd_data_recv_bufferd(void *arg, struct tcp_pcb *tpcb, struct pbuf *p, err_t err)
{
  struct ftpd_state *state = arg;
  struct pbuf *q;
  FRESULT fr;
  UINT bw = FTPD_FILE_BUFFER_LEN;
  u64 time_st, time_ed, time;

  // Pretime1 = get_current_time();
  if (p != NULL)
  {
    if (state != NULL)
    {
      if (strcasecmp(state->cmd, "STOR") == 0)
      {
        // LWIP_DEBUGF(FTPD_DEBUG, ("%s: %d bytes received\n", __FUNCTION__, p->tot_len));
        for (q = p; q != NULL; q = q->next)
        {
          if (host_buffer ==0){
            if ((buffer0_watermark + q->len) <= FTPD_FILE_BUFFER_LEN){
              memcpy(buffer0_pt, q->payload, q->len);
              buffer0_watermark += q->len;
              buffer0_pt += q->len;
            }
            else{
              u32_t temp_len = FTPD_FILE_BUFFER_LEN - buffer0_watermark;
              memcpy(buffer0_pt, q->payload, temp_len);
              memcpy(buffer1_pt, ((u8_t *)(q->payload) + temp_len), (q->len - temp_len));
              
              host_buffer = 1;
              buffer0_pt = (void *)ftpd_recv_buffer0;
              buffer0_watermark = 0;
              buffer1_pt += (q->len - temp_len);
              buffer1_watermark += (q->len - temp_len);
              
              fr = f_write(state->fp, (void *)ftpd_recv_buffer0, FTPD_FILE_BUFFER_LEN, &bw);
              }
          }
          else{
            if ((buffer1_watermark + q->len) <= FTPD_FILE_BUFFER_LEN){
              memcpy(buffer1_pt, q->payload, q->len);
              buffer1_watermark += q->len;
              buffer1_pt += q->len;
            }
            else{
              u32_t temp_len = FTPD_FILE_BUFFER_LEN - buffer1_watermark;
              memcpy(buffer1_pt, q->payload, temp_len);
              memcpy(buffer0_pt, ((u8_t *)(q->payload) + temp_len), (q->len - temp_len));
              
              host_buffer = 0;
              buffer1_pt = (void *)ftpd_recv_buffer1;
              buffer1_watermark = 0;
              buffer0_pt += (q->len - temp_len);
              buffer0_watermark += (q->len - temp_len);
              
              fr = f_write(state->fp, (void *)ftpd_recv_buffer1, FTPD_FILE_BUFFER_LEN, &bw);
              }
          }
           
           data_recv_count++;
           if (bw != FTPD_FILE_BUFFER_LEN){
             
             //LWIP_DEBUGF(FTPD_DEBUG | LWIP_DBG_LEVEL_SERIOUS, ("%s: f_write() failed! fr=%d, q->len=%u, bw=%u\n", __FUNCTION__, fr, q->len, bw));
             printf("%s: f_write() failed! fr=%d, q->len=%u, bw=%u\n", __FUNCTION__, fr, q->len, bw);
             pbuf_free(p);
             err = ftpd_free_data(state, FTPD_FREEDATA_ABORT);
             state->cmdstep = FTPD_CMDSTEP_CONNABORTED;
             ftpd_process_cmd(state);
             bw = FTPD_FILE_BUFFER_LEN;
             ftpd_data_recv_buffer_clear();
             return err;
           }

        }
      }
    }
    tcp_recved(tpcb, p->tot_len);
    pbuf_free(p);
  }
  else
  {
    if (state != NULL)
    {
      if (host_buffer ==0){
        fr = f_write(state->fp, (void *)ftpd_recv_buffer0, buffer0_watermark, &bw);
        if (bw != FTPD_FILE_BUFFER_LEN){
          printf("%s: f_write() failed! fr=%d, q->len=%u, bw=%u\n", __FUNCTION__, fr, q->len, bw);
          err = ftpd_free_data(state, FTPD_FREEDATA_ABORT);
          state->cmdstep = FTPD_CMDSTEP_CONNABORTED;
          ftpd_process_cmd(state);
          bw = FTPD_FILE_BUFFER_LEN;
          ftpd_data_recv_buffer_clear();
          return err;
        }
      }
      else{
        fr = f_write(state->fp, (void *)ftpd_recv_buffer1, buffer1_watermark, &bw);
        if (bw != FTPD_FILE_BUFFER_LEN){
          printf("%s: f_write() failed! fr=%d, q->len=%u, bw=%u\n", __FUNCTION__, fr, q->len, bw);
          err = ftpd_free_data(state, FTPD_FREEDATA_ABORT);
          state->cmdstep = FTPD_CMDSTEP_CONNABORTED;
          ftpd_process_cmd(state);
          bw = FTPD_FILE_BUFFER_LEN;
          ftpd_data_recv_buffer_clear();
          return err;
        }
      }
      ftpd_data_recv_buffer_clear();

      LWIP_DEBUGF(FTPD_DEBUG, ("FTPD data connection [%s]:%d is shutdown by the client!\n", ipaddr_ntoa(&tpcb->remote_ip), tpcb->remote_port));
      ftpd_free_data(state, FTPD_FREEDATA_CLOSE);
      
      // ????????????, ??????????????????
      state->cmdstep |= FTPD_CMDSTEP_CONNSHUTDOWN;
      ftpd_process_cmd(state);
    }
    else{
      LWIP_DEBUGF(FTPD_DEBUG, ("FTPD data connection [%s]:%d is closed by the client!\n", ipaddr_ntoa(&tpcb->remote_ip), tpcb->remote_port));
      ftpd_data_recv_buffer_clear;
    }
  }
  return ERR_OK;
}
