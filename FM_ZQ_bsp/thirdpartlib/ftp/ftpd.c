/*************************** ����LwIP raw API��FTP������*****************************
** ע������:
** 1. ʹ��FileZilla�ͻ�������FTP������ʱ, �ַ���Ӧ��ѡ��"ǿ��UTF-8"
**    ����ѡ��"�Զ����", �����ļ����Ų�������
** 2. ����Ҫ�ƶ��ļ�, �����ļ���������ʹ��"xxx/", "../"�������﷨�������ļ�
**    ������Ҫ��"abc.txt"�ƶ�����ǰĿ¼��123Ŀ¼��, ����Խ��ļ�������Ϊ"123/abc.txt"
**    ��"def.doc"�ƶ�����Ŀ¼��456�ļ�����, ��Ӧ��������Ϊ"../456/def.doc"
** 3. ������ʹ��FatFs��д�����ļ�
**    �������HardFault����, �������startup_stm32*.s�����ļ������Stack_Sizeֵ̫С
**    ����Ĵ�Ϳ��Խ������
************************************************************************************/
#include <Fatfs15/ff.h>
#include <lwip/tcp.h>
#include <string.h>
#include <time.h>
#include <ctype.h>
#include "ftpd.h"
#include "fmsh_common.h"
#include "lwip/sys.h"
#include "lwipopts.h"

static err_t ftpd_accept(void *arg, struct tcp_pcb *newpcb, err_t err);
static void ftpd_change_user(struct ftpd_state *state, const char *newuser);
static int ftpd_copy_cmd(struct ftpd_state *state);
#if FTPD_PASV
static err_t ftpd_data_accept(void *arg, struct tcp_pcb *newpcb, err_t err);
#endif
static void ftpd_data_check(struct ftpd_state *state);
static err_t ftpd_data_connected(void *arg, struct tcp_pcb *tpcb, err_t err);
static void ftpd_data_err(void *arg, err_t err);
static err_t ftpd_data_recv(void *arg, struct tcp_pcb *tpcb, struct pbuf *p, err_t err);
static err_t ftpd_data_sent(void *arg, struct tcp_pcb *tpcb, u16_t len);
static err_t ftpd_data_sent_list(void *arg, struct tcp_pcb *tpcb, u16_t len);
static err_t ftpd_data_sent_retr(void *arg, struct tcp_pcb *tpcb, u16_t len);
static void ftpd_err(void *arg, err_t err);
static void ftpd_free(struct ftpd_state *state);
static err_t ftpd_free_data(struct ftpd_state *state, int option);
static int ftpd_is_valid_user(struct ftpd_user *user, int *pid);
static int ftpd_prepare_data(struct ftpd_state *state);
static void ftpd_process_cmd(struct ftpd_state *state);
static int ftpd_process_data_cmd(struct ftpd_state *state);
static int ftpd_process_directory_cmd(struct ftpd_state *state);
static int ftpd_process_file_cmd(struct ftpd_state *state);
static int ftpd_process_opt_cmd(struct ftpd_state *state);
static int ftpd_process_user_cmd(struct ftpd_state *state);
static err_t ftpd_recv(void *arg, struct tcp_pcb *tpcb, struct pbuf *p, err_t err);
static int ftpd_send_msg(struct ftpd_state *state, const char *s);
static err_t ftpd_sent(void *arg, struct tcp_pcb *tpcb, u16_t len);

static err_t ftpd_data_recv_bufferd(void *arg, struct tcp_pcb *tpcb, struct pbuf *p, err_t err);


extern u8_t pbuf_remove_header(struct pbuf *p, size_t header_size);
extern struct pbuf *pbuf_free_header(struct pbuf *q, u16_t size);

unsigned char ftpd_data_temp[1024];
unsigned int cur_data_len = 0;
// �û��б��Լ���Ӧ�ĸ�Ŀ¼
static const struct ftpd_account ftpd_users[] = {
  {{"anonymous", NULL}, "C:/public"}, // �����û�
  {{"admin", "123456"}, "0:/"},
  {{"test", "789123"}, "C:/test"}
};
// �̷�����ffconf.h�е�FF_VOLUME_STRS��ָ��

static struct tcp_pcb *ftpd_tpcb;
u64 Pretime = 0;
u64 Curtime = 0;
double timeUsed = 0;

/* ���������յ������� */
static err_t ftpd_accept(void *arg, struct tcp_pcb *newpcb, err_t err)
{
  struct ftpd_state *state;
  
  if (err != ERR_OK)
  {
    LWIP_DEBUGF(FTPD_DEBUG | LWIP_DBG_LEVEL_SERIOUS, ("FTPD failed to accept a client! err=%d\r\n", err));
    return err;
  }
  
  state = mem_malloc(sizeof(struct ftpd_state));
  if (state == NULL)
  {
    LWIP_DEBUGF(FTPD_DEBUG | LWIP_DBG_LEVEL_SERIOUS, ("FTPD failed to accept [%s]:%d!\r\n", ipaddr_ntoa(&newpcb->remote_ip), newpcb->remote_port));
    tcp_abort(newpcb);
    return ERR_ABRT;
  }
  
  memset(state, 0, sizeof(struct ftpd_state));
  state->ctrlconn = newpcb;
  state->dataport = -1;
  state->type = 'A';
  strcpy(state->path, "/");
  state->userid = -1;
  
  LWIP_DEBUGF(FTPD_DEBUG, ("FTPD accepted [%s]:%d!\r\n", ipaddr_ntoa(&newpcb->remote_ip), newpcb->remote_port));
  ftpd_send_msg(state, "220 LwIP FTP Service\r\n");
  
  tcp_arg(newpcb, state);
  tcp_err(newpcb, ftpd_err);
  tcp_recv(newpcb, ftpd_recv);
  tcp_sent(newpcb, ftpd_sent);
  
  return ERR_OK;
}

/* �ı��û������������ */
static void ftpd_change_user(struct ftpd_state *state, const char *newuser)
{
  if (state->user.name != NULL)
  {
    mem_free(state->user.name);
    state->user.name = NULL;
  }
  if (state->user.password != NULL)
  {
    mem_free(state->user.password);
    state->user.password = NULL;
  }

  if (newuser != NULL)
    state->user.name = ftpd_strdup(newuser);
}

/* ���ļ��к��ļ���������һ���γ���·�� */
// ��buffer��filename��������, ���浽buffer��, ͬʱ��֤�ַ���ĩβ����б��(��Ŀ¼����); buffer���������Ϊbufsize
// �ɹ�ʱ�����ַ����ĳ���; ʧ��ʱ����-1��buffer�е����ݲ���
int ftpd_concat_path(char *buffer, int bufsize, const char *filename)
{
  char *p;
  int addslash, fileabs, folderlen, namelen, len;

  // �ҳ��ַ���������λ��, ��ȥ��buffer��βб�ܺ�filename����б��
  if (filename != NULL && filename[0] == '/')
  {
    // ����ļ����Ǿ���·��, ����Ҫ���ļ���·����Ϊ��Ŀ¼
    fileabs = 1;
    filename++; // ȥ����б��

    if (buffer[0] == '/')
      folderlen = 1; // �ļ���·�������̷�ʱֻ������Ŀ¼���� (��б��)
    else
    {
      p = strchr(buffer, ':');
      if (p != NULL)
        folderlen = p + 1 - buffer; // �ļ���·�����̷�ʱֻ�����̷�
      else
        folderlen = 0; // ���buffer�����·��, ����ַ���
    }
  }
  else
  {
    // ����ļ������Ǿ���·��, �����ֱ�����ļ���·��ĩβ�������ļ���
    fileabs = 0;
    folderlen = strlen(buffer);
    if (folderlen > 1 && buffer[folderlen - 1] == '/')
      folderlen--;
  }

  // ȥ��filename��βб��
  if (filename != NULL)
    namelen = strlen(filename);
  else
    namelen = 0;
  if (namelen != 0 && filename[namelen - 1] == '/')
    namelen--;

  // �����ַ���������һ�����Ҫ�Ļ�������С
  if (folderlen == 0)
    addslash = fileabs; // ·��Ϊ��ʱ�Ӳ���б��ȡ�����ļ����ǲ��Ǿ���·��
  else if (folderlen == 1 && buffer[0] == '/')
    addslash = 0; // ·��Ϊб��ʱ����б��
  else if (folderlen != 0 && buffer[folderlen - 1] == ':')
    addslash = 1; // ·�����һ���ַ�Ϊð��ʱҪ��б��
  else if (namelen == 0)
    addslash = 0; // �ļ���Ϊ��ʱ����б��
  else
    addslash = 1; // ���������Ҫ��б��
  len = folderlen + addslash + namelen; // ���Ӻ�ĳ���
  if (len >= bufsize)
    return -1; // ����������

  // �����ַ���
  if (addslash)
    buffer[folderlen] = '/';
  if (namelen != 0)
    memcpy(buffer + folderlen + addslash, filename, namelen);
  buffer[len] = '\0';
  return len;
}

/* �����ݽ��ն���queue�е�FTP�����ַ�����ȡ��state->cmd��, ���ͷ�ռ�õ�pbuf�ڴ� */
// ����ֵ: 0��ʾ��û���յ���������; 1��ʾ�յ�����������; 2��ʾ�յ�����������, �������˻�������󳤶�
// state->cmdlen��ʾ���յ��˵�ǰ������ٸ��ַ� (����\r\n)
static int ftpd_copy_cmd(struct ftpd_state *state)
{
  char *c;
  int complete = 0; // �Ƿ��յ���������
  int cnt = 0; // ���θ��Ƶ��ַ���
  int i;
  struct pbuf *p;
  
  for (p = state->queue; p != NULL && complete == 0; p = p->next)
  {
    c = p->payload;
    for (i = 0; i < p->len && complete == 0; i++)
    {
      if (state->last == '\r' && *c == '\n')
      {
        if (state->cmdlen <= sizeof(state->cmd))
        {
          state->cmd[state->cmdlen - 1] = '\0'; // ��\r�滻��\0
          complete = 1;
        }
        else
          complete = 2;
      }
      else
      {
        if (state->cmdlen < sizeof(state->cmd))
          state->cmd[state->cmdlen] = *c;
      }
      
      state->cmdlen++;
      state->last = *c;
      c++;
      cnt++;
    }
  }
  
  state->queue = pbuf_free_header(state->queue, cnt);
  return complete;
}

#if FTPD_PASV
/* �������ӱ���ģʽ���ӽ����ɹ� */
static err_t ftpd_data_accept(void *arg, struct tcp_pcb *newpcb, err_t err)
{
  struct ftpd_state *state = arg;
  
  if (err != ERR_OK)
  {
    LWIP_DEBUGF(FTPD_DEBUG | LWIP_DBG_LEVEL_SERIOUS, ("FTPD data failed to accept a client! err=%d\r\n", err));
    return err;
  }
  
  if (!ip_addr_cmp(&newpcb->remote_ip, &state->ctrlconn->remote_ip))
  {
    LWIP_DEBUGF(FTPD_DEBUG | LWIP_DBG_LEVEL_WARNING, ("%s: IP address mismatch!\r\n", __FUNCTION__));
    tcp_abort(newpcb);
    return ERR_ABRT;
  }
  
  tcp_close(state->dataconn); // �رն˿ڼ���
  state->dataconn = newpcb;
  tcp_err(newpcb, ftpd_data_err);
  return ftpd_data_connected(arg, newpcb, err);
}
#endif

/* ������������Ƿ�δ��ʼ�������� */
// �������Ӧ���ڿ������ӷ����꿪ʼ��Ϣ�����һ��
static void ftpd_data_check(struct ftpd_state *state)
{
#if FTPD_PASV
  if (state->flags & FTPD_FLAG_PASSIVE)
  {
    // ��PASVģʽ��, ���ӿ��ܻ���PASV����ִ����ϵ�ʱ��ͽ����ɹ�
    // ������Ҫ�ȵ����ݴ�������(��LIST����)����Ӧ(��150��Ӧ)������Ϻ�, ���ܿ�ʼ��������
    if ((state->flags & FTPD_FLAG_NEWDATACONN) == 0)
      ftpd_data_sent(state, state->dataconn, 0);
    // PORTģʽ�²������������, ��Ϊ���ӽ�����Ϳ���������ʼ��������
  }
#endif
}

/* ������������ģʽ�����ɹ� */
static err_t ftpd_data_connected(void *arg, struct tcp_pcb *tpcb, err_t err)
{
  struct ftpd_state *state = arg;
  
  LWIP_DEBUGF(FTPD_DEBUG, ("FTPD data connection to [%s]:%d is established!\r\n", ipaddr_ntoa(&tpcb->remote_ip), tpcb->remote_port));
  state->flags &= ~FTPD_FLAG_NEWDATACONN;
  
  tcp_recv(tpcb, ftpd_data_recv_bufferd);
  tcp_sent(tpcb, ftpd_data_sent);
  
  return ftpd_data_sent(arg, tpcb, 0);
}

/* �������ӳ��� */
static void ftpd_data_err(void *arg, err_t err)
{
  struct ftpd_state *state = arg;
  
  LWIP_DEBUGF(FTPD_DEBUG | LWIP_DBG_LEVEL_WARNING, ("FTPD data error! err=%d\r\n", err));
  if (state != NULL)
  {
    state->dataconn = NULL; // ����err�ص�����ʱ, tpcb�Ѿ�
    ftpd_free_data(state, FTPD_FREEDATA_ABORT);
    
    if (state->flags & FTPD_FLAG_NEWDATACONN)
    {
      state->flags &= ~FTPD_FLAG_NEWDATACONN;
      state->cmdstep |= FTPD_CMDSTEP_CONNFAILED;
      ftpd_send_msg(state, "425 Failed to establish connection.\r\n");
    }
    else
    {
      state->cmdstep |= FTPD_CMDSTEP_CONNABORTED;
      ftpd_process_cmd(state);
    }
  }
}
//err_t ftpd_disk_write(FIL* fp,const void* buff,UINT btw,UINT* bw)
//{
//    u16_t i,count,remain;
//    u16_t once_len = 256;
//    u16_t buff_index = 0;
// 
//    if(cur_data_len != 0)
//    {
//      buff_index = once_len - cur_data_len;
//      memcpy(ftpd_data_temp+cur_data_len,buff,buff_index);
//      f_write(fp, (void *)ftpd_data_temp, once_len, bw);
//      if (*bw != once_len)
//      {
//        return ERR_BUF;
//      }
//    }
//
//    count = (btw - buff_index) / once_len;
//    remain = (btw - buff_index) % once_len;
//
//    for (i = 0; i < count; i++)
//    {
//      f_write(fp, (void *)(buff+buff_index), once_len, bw);
//      if (*bw != once_len)
//      {
//        return ERR_BUF;
//      }
//      buff_index += once_len;
//    }
//
//    if (remain != 0)
//    {
//      memset(ftpd_data_temp,0x0,sizeof(ftpd_data_temp));
//      cur_data_len = remain;
//      memcpy(ftpd_data_temp,buff+buff_index,cur_data_len);
//    }
//
//    return ERR_OK;
//}

unsigned int data_recv_count = 0;

static err_t ftpd_data_recv(void *arg, struct tcp_pcb *tpcb, struct pbuf *p, err_t err)
{
  struct ftpd_state *state = arg;
  struct pbuf *q;
  FRESULT fr;
  UINT bw;

  if (p != NULL)
  {
    if (state != NULL)
    {
      if (strcasecmp(state->cmd, "STOR") == 0)
      {
        // LWIP_DEBUGF(FTPD_DEBUG, ("%s: %d bytes received\r\n", __FUNCTION__, p->tot_len));
        for (q = p; q != NULL; q = q->next)
        {

 //          time_st = get_current_time();
           fr = f_write(state->fp, q->payload, q->len, &bw);
 //          time_ed = get_current_time();
//           time = (u64)((time_ed - time_st) * ((float)1e9 / GTC_FREQ) / 1000);
//           fmsh_print("f_write cost %lld us.\r\n", time);

           data_recv_count++;
           if (bw != q->len)
           {
             //LWIP_DEBUGF(FTPD_DEBUG | LWIP_DBG_LEVEL_SERIOUS, ("%s: f_write() failed! fr=%d, q->len=%u, bw=%u\r\n", __FUNCTION__, fr, q->len, bw));
             fmsh_print("%s: f_write() failed! fr=%d, q->len=%u, bw=%u\r\n", __FUNCTION__, fr, q->len, bw);
             pbuf_free(p);
             err = ftpd_free_data(state, FTPD_FREEDATA_ABORT);
             state->cmdstep = FTPD_CMDSTEP_CONNABORTED;
             ftpd_process_cmd(state);
             return err;
           }
//          err = ftpd_disk_write(state->fp, q->payload, q->len, &bw);
//          if (err != ERR_OK)
//          {
//              pbuf_free(p);
//              err = ftpd_free_data(state, FTPD_FREEDATA_ABORT);
//              state->cmdstep = FTPD_CMDSTEP_CONNABORTED;
//              ftpd_process_cmd(state);
//              return err;
//          }

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
      // if (cur_data_len != 0)
      // {
      //     f_write(state->fp, ftpd_data_temp, cur_data_len, &bw);
      //     if (bw != cur_data_len)
      //     {
      //       state->cmdstep = FTPD_CMDSTEP_CONNABORTED;
      //       ftpd_process_cmd(state);
      //     }
      //     cur_data_len = 0;
      //     memset(ftpd_data_temp,0x0,sizeof(ftpd_data_temp));
      // }
      
      LWIP_DEBUGF(FTPD_DEBUG, ("FTPD data connection [%s]:%d is shutdown by the client!\r\n", ipaddr_ntoa(&tpcb->remote_ip), tpcb->remote_port));
      ftpd_free_data(state, FTPD_FREEDATA_CLOSE);
      
      // ֪ͨ���������, ���������ѱ��ͻ��˹ر�
      state->cmdstep |= FTPD_CMDSTEP_CONNSHUTDOWN;
      ftpd_process_cmd(state);
    }
    else
      LWIP_DEBUGF(FTPD_DEBUG, ("FTPD data connection [%s]:%d is closed by the client!\r\n", ipaddr_ntoa(&tpcb->remote_ip), tpcb->remote_port));
  }
  return ERR_OK;
}

//buffered ftpd recv

static u8_t ftpd_recv_buffer0[FTPD_FILE_BUFFER_LEN];
static u8_t ftpd_recv_buffer1[FTPD_FILE_BUFFER_LEN];

static u8_t *buffer0_pt = (u8_t *)ftpd_recv_buffer0;
static u8_t *buffer1_pt = (u8_t *)ftpd_recv_buffer1;

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
  data_recv_count = 0;
}


static err_t ftpd_data_recv_bufferd(void *arg, struct tcp_pcb *tpcb, struct pbuf *p, err_t err)
{
  struct ftpd_state *state = arg;
  struct pbuf *q;
  FRESULT fr;
  UINT bw = FTPD_FILE_BUFFER_LEN;

  if (p != NULL)
  {
    if (state != NULL)
    {
      if (strcasecmp(state->cmd, "STOR") == 0)
      {
        for (q = p; q != NULL; q = q->next)
        {
          if (host_buffer ==0){
            if ((buffer0_watermark + q->len) <= FTPD_FILE_BUFFER_LEN){
              memcpy(buffer0_pt, q->payload, q->len);
              buffer0_watermark += q->len;
              buffer0_pt += q->len;
              Fmsh_DCacheFlushRange((uintptr_t)ftpd_recv_buffer0, FTPD_FILE_BUFFER_LEN);
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
              Fmsh_DCacheFlushRange((uintptr_t)ftpd_recv_buffer0, FTPD_FILE_BUFFER_LEN);
              fr = f_write(state->fp, (void *)ftpd_recv_buffer0, FTPD_FILE_BUFFER_LEN, &bw);
              }
          }
          else{
            if ((buffer1_watermark + q->len) <= FTPD_FILE_BUFFER_LEN){
              memcpy(buffer1_pt, q->payload, q->len);
              buffer1_watermark += q->len;
              buffer1_pt += q->len;
              Fmsh_DCacheFlushRange((uintptr_t)ftpd_recv_buffer1, FTPD_FILE_BUFFER_LEN);
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
              Fmsh_DCacheFlushRange((uintptr_t)ftpd_recv_buffer1, FTPD_FILE_BUFFER_LEN);
              fr = f_write(state->fp, (void *)ftpd_recv_buffer1, FTPD_FILE_BUFFER_LEN, &bw);
              }
          }
           
           data_recv_count++;
           if (bw != FTPD_FILE_BUFFER_LEN){
             
             fmsh_print("%s: f_write() failed! fr=%d, =%u, bw=%u\r\n", __FUNCTION__, fr, FTPD_FILE_BUFFER_LEN, bw);
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
      if (state->fp != NULL){
        if (host_buffer ==0){
          Fmsh_DCacheFlushRange((uintptr_t)ftpd_recv_buffer0, buffer0_watermark);
          fr = f_write(state->fp, (void *)ftpd_recv_buffer0, buffer0_watermark, &bw);
          if (bw != buffer0_watermark){
            fmsh_print("%s: f_write() failed! fr=%d, len=%u, bw=%u\r\n", __FUNCTION__, fr, buffer0_watermark, bw);
            err = ftpd_free_data(state, FTPD_FREEDATA_ABORT);
            state->cmdstep = FTPD_CMDSTEP_CONNABORTED;
            ftpd_process_cmd(state);
            bw = FTPD_FILE_BUFFER_LEN;
            ftpd_data_recv_buffer_clear();
            return err;
          }
        }
        else{
          Fmsh_DCacheFlushRange((uintptr_t)ftpd_recv_buffer1, buffer1_watermark);
          fr = f_write(state->fp, (void *)ftpd_recv_buffer1, buffer1_watermark, &bw);
          if (bw != buffer1_watermark){
            fmsh_print("%s: f_write() failed! fr=%d, len=%u, bw=%u\r\n", __FUNCTION__, fr, buffer1_watermark, bw);
            err = ftpd_free_data(state, FTPD_FREEDATA_ABORT);
            state->cmdstep = FTPD_CMDSTEP_CONNABORTED;
            ftpd_process_cmd(state);
            bw = FTPD_FILE_BUFFER_LEN;
            ftpd_data_recv_buffer_clear();
            return err;
          }
        }
        ftpd_data_recv_buffer_clear();
      }
      LWIP_DEBUGF(FTPD_DEBUG, ("FTPD data connection [%s]:%d is shutdown by the client!\r\n", ipaddr_ntoa(&tpcb->remote_ip), tpcb->remote_port));
      ftpd_free_data(state, FTPD_FREEDATA_CLOSE);
      
      state->cmdstep |= FTPD_CMDSTEP_CONNSHUTDOWN;
      ftpd_process_cmd(state);
    }
    else{
      LWIP_DEBUGF(FTPD_DEBUG, ("FTPD data connection [%s]:%d is closed by the client!\r\n", ipaddr_ntoa(&tpcb->remote_ip), tpcb->remote_port));
      ftpd_data_recv_buffer_clear();
    }
  }
  return ERR_OK;
}

static err_t ftpd_data_sent(void *arg, struct tcp_pcb *tpcb, u16_t len)
{
  err_t err = ERR_OK;
  struct ftpd_state *state = arg;
  
  if (state != NULL)
  {
    
    if (strcasecmp(state->cmd, "LIST") == 0)
      err = ftpd_data_sent_list(arg, tpcb, len);
    else if (strcasecmp(state->cmd, "RETR") == 0)
      err = ftpd_data_sent_retr(arg, tpcb, len);
  }
  return err;
}

static err_t ftpd_data_sent_list(void *arg, struct tcp_pcb *tpcb, u16_t len)
{
  char buffer[MAX_PATH + 100];
  err_t err;
  int bufsize, loop, slen;
  struct ftpd_state *state = arg;
  struct tm tm;
  FRESULT fr;
  
  if (state->finfo == NULL)
  {
    loop = 2;
    state->finfo = mem_malloc(sizeof(FILINFO));
    if (state->finfo == NULL)
    {
      LWIP_DEBUGF(FTPD_DEBUG | LWIP_DBG_LEVEL_SERIOUS, ("%s: mem_malloc() failed!\r\n", __FUNCTION__));
      goto err;
    }
  }
  else
    loop = 1;
  
  while (loop)
  {
    if (loop == 2)
    {
      fr = f_readdir(state->dp, state->finfo);
      if (fr != FR_OK || state->finfo->fname[0] == '\0')
      {
        if (fr != FR_OK)
          LWIP_DEBUGF(FTPD_DEBUG | LWIP_DBG_LEVEL_SERIOUS, ("%s: f_readdir() failed! fr=%d\r\n", __FUNCTION__, fr)); // ��ȡ�ļ���Ϣʧ��
        
        ftpd_free_data(state, FTPD_FREEDATA_SHUTDOWN);
        state->cmdstep = 2;
        ftpd_process_cmd(state);
        break;
      }
    }
    
    if (strcmp(state->finfo->fname, ".") == 0 || strcmp(state->finfo->fname, "..") == 0)
    {
      LWIP_DEBUGF(FTPD_DEBUG, ("%s: jumping over \"%s\"\r\n", __FUNCTION__, state->finfo->fname));
      continue;
    }
    
    ftpd_filetime(state->finfo->fdate, state->finfo->ftime, &tm);
    slen = strftime(buffer, sizeof(buffer), "%m-%d-%Y  %I:%M%p       ", &tm);
    if (state->finfo->fattrib & AM_DIR)
      strcpy(buffer + slen, "<DIR>          ");
    else
      sprintf(buffer + slen, "%14u ", state->finfo->fsize);
    slen += 15;
    slen += sprintf(buffer + slen, "%s\r\n", state->finfo->fname);
    LWIP_ASSERT("slen < sizeof(buffer)", slen < sizeof(buffer));
    
    bufsize = tcp_sndbuf(tpcb);
    if (bufsize >= slen)
    {
      LWIP_DEBUGF(FTPD_DEBUG, ("%s", buffer));
      err = tcp_write(tpcb, buffer, slen, TCP_WRITE_FLAG_COPY);
      if (err != ERR_OK)
      {
        LWIP_DEBUGF(FTPD_DEBUG | LWIP_DBG_LEVEL_SERIOUS, ("%s: tcp_write() failed! err=%d\r\n", __FUNCTION__, err));
        goto err;
      }
      else 
      {
        tcp_output(tpcb);
      }
      loop = 2;
    }
    else
    {
      LWIP_DEBUGF(FTPD_DEBUG, ("%s: paused! sndbuf=%d, slen=%d\r\n", __FUNCTION__, bufsize, slen));
      loop = 0;
    }
  }
  return ERR_OK;

err:
  err = ftpd_free_data(state, FTPD_FREEDATA_ABORT);
  state->cmdstep = FTPD_CMDSTEP_CONNABORTED;
  ftpd_process_cmd(state);
  return err;
}

static err_t ftpd_data_sent_retr(void *arg, struct tcp_pcb *tpcb, u16_t len)
{
  char buffer[30];
  unsigned char ftpd_data_buffer[TCP_SND_BUF];
  err_t err;
  struct ftpd_state *state = arg;
  unsigned int size;
  FRESULT fr;
  UINT br;
  
  state->dataout_len -= len;
  if (state->dataout_len != 0)
    return ERR_OK;
  
  if (state->dataout != NULL)
  {
    mem_free(state->dataout);
    state->dataout = NULL;
  }
  
  size = tcp_sndbuf(tpcb);
  LWIP_ASSERT("sndbuf != 0", size != 0);
  //state->dataout = mem_malloc(size);
  state->dataout = ftpd_data_buffer;
  if (state->dataout == NULL)
  {
    // �ڴ����ʧ��ʱ����buffer������
    LWIP_DEBUGF(FTPD_DEBUG | LWIP_DBG_LEVEL_SERIOUS, ("%s: mem_malloc() failed!\r\n", __FUNCTION__));
    if (size > sizeof(buffer))
      size = sizeof(buffer);
  }
  
  if (state->dataout != NULL)
    fr = f_read(state->fp, state->dataout, size, &br);
  else
    fr = f_read(state->fp, buffer, size, &br);
  
  if (fr == FR_OK) 
  { 
    if (state->dataout != NULL)
    {
      Fmsh_DCacheFlushRange((uintptr_t)state->dataout, br);
    } 
    else 
    {
      Fmsh_DCacheFlushRange((uintptr_t)buffer, br);
    }
  }
  if (fr != FR_OK)
  {
    LWIP_DEBUGF(FTPD_DEBUG | LWIP_DBG_LEVEL_SERIOUS, ("%s: failed to read file! fr=%d\r\n", __FUNCTION__, fr));
    goto err;
  }
  if (br < size)
    size = br;
  
  if (size > 0)
  {
    state->dataout_len = size;
    if (state->dataout != NULL)
    {
        err = tcp_write(tpcb, state->dataout, size, 1);
        tcp_output(tpcb);
    }
    else
    {
        err = tcp_write(tpcb, buffer, size, TCP_WRITE_FLAG_COPY);
        tcp_output(tpcb);
    }
    
        if (err != ERR_OK)
        {
          LWIP_DEBUGF(FTPD_DEBUG | LWIP_DBG_LEVEL_SERIOUS, ("%s: tcp_write() failed! err=%d\r\n", __FUNCTION__, err));
          goto err;
        }
  }
  
  if (f_eof(state->fp))
  {

    ftpd_free_data(state, FTPD_FREEDATA_SHUTDOWN);
    state->cmdstep = 2;
    ftpd_process_cmd(state);
  }
  return ERR_OK;

err:
  err = ftpd_free_data(state, FTPD_FREEDATA_ABORT);
  state->cmdstep = FTPD_CMDSTEP_CONNABORTED;
  ftpd_process_cmd(state);
  return err;
}

static void ftpd_err(void *arg, err_t err)
{
  struct ftpd_state *state = arg;
  
  LWIP_DEBUGF(FTPD_DEBUG | LWIP_DBG_LEVEL_WARNING, ("FTPD error! err=%d\r\n", err));
  if (state != NULL)
  {
    state->ctrlconn = NULL;
    ftpd_free(state);
  }
}

/* �ж�ָ���ļ��Ƿ���� */
// ������ж��ļ����Ƿ����, ���ַ���ĩβ������б��
int ftpd_file_exists(const char *path)
{
  FRESULT fr;
  
  if (strcmp(path + 1, ":") == 0 || strcmp(path + 1, ":/") == 0)
    return 1;
  fr = f_stat(path, NULL);
  return fr == FR_OK;
}

time_t ftpd_filetime(WORD fdate, WORD ftime, struct tm *ptm)
{
  memset(ptm, 0, sizeof(struct tm));
  ptm->tm_year = ((fdate >> 9) & 0x7f) + 80;
  ptm->tm_mon = ((fdate >> 5) & 0x0f) - 1;
  ptm->tm_mday = fdate & 0x1f;
  ptm->tm_hour = (ftime >> 11) & 0x1f;
  ptm->tm_min = (ftime >> 5) & 0x3f;
  ptm->tm_sec = (ftime & 0x1f) << 1;
  
  return mktime(ptm);
}


static void ftpd_free(struct ftpd_state *state)
{
  if (state == NULL)
    return;
  ftpd_free_data(state, FTPD_FREEDATA_ABORT);
  if (state->ctrlconn != NULL)
  {
    if (state->flags & FTPD_FLAG_CLOSE)
      LWIP_DEBUGF(FTPD_DEBUG, ("FTPD connection [%s]:%d is closed by the server!\r\n", ipaddr_ntoa(&state->ctrlconn->remote_ip), state->ctrlconn->remote_port));
    else
      LWIP_DEBUGF(FTPD_DEBUG, ("FTPD connection [%s]:%d is shutdown by the server!\r\n", ipaddr_ntoa(&state->ctrlconn->remote_ip), state->ctrlconn->remote_port));
    
    tcp_arg(state->ctrlconn, NULL);
    tcp_close(state->ctrlconn);
    state->ctrlconn = NULL;
  }
  
  ftpd_change_user(state, NULL);
  mem_free(state);
}

static err_t ftpd_free_data(struct ftpd_state *state, int option)
{
  err_t err = ERR_OK;
  
  if (state == NULL)
    return err;
  
  state->dataport = -1;
  if (state->dataconn != NULL)
  {
    tcp_arg(state->dataconn, NULL); // ���ӹرպ�, �ص��������п��ܴ���, ���Ա����state���������ϵ
#if FTPD_PASV
    if ((state->flags & FTPD_FLAG_PASSIVE) && (state->flags & FTPD_FLAG_NEWDATACONN))
    {
      LWIP_DEBUGF(FTPD_DEBUG, ("FTPD data pcb is removed!\r\n"));
      tcp_close(state->dataconn);
    }
    else
    {
#endif
      if (option == FTPD_FREEDATA_ABORT)
      {
        LWIP_DEBUGF(FTPD_DEBUG, ("FTPD data connection [%s]:%d is aborted!\r\n", ipaddr_ntoa(&state->dataconn->remote_ip), state->dataconn->remote_port));
        tcp_abort(state->dataconn);
        err = ERR_ABRT;
      }
      else
      {
        if (option == FTPD_FREEDATA_CLOSE)
          LWIP_DEBUGF(FTPD_DEBUG, ("FTPD data connection [%s]:%d is closed by the server!\r\n", ipaddr_ntoa(&state->dataconn->remote_ip), state->dataconn->remote_port));
        else
          LWIP_DEBUGF(FTPD_DEBUG, ("FTPD data connection [%s]:%d is shutdown by the server!\r\n", ipaddr_ntoa(&state->dataconn->remote_ip), state->dataconn->remote_port));
        tcp_close(state->dataconn);
      }
#if FTPD_PASV
    }
#endif
    state->dataconn = NULL;
  }
  
  if (state->dataout != NULL)
  {
    mem_free(state->dataout);
    state->dataout = NULL;
    state->dataout_len = 0;
  }
  
  // ֻ���ļ��гɹ�����֮��, �ſ��Խ�ָ�븳��state->dp
  if (state->dp != NULL)
  {
    f_closedir(state->dp);
    mem_free(state->dp);
    state->dp = NULL;
  }
  
  // ֻ���ļ��ɹ�����֮��, �ſ��Խ�ָ�븳��state->fp
  if (state->fp != NULL)
  {
    f_close(state->fp);
    mem_free(state->fp);
    state->fp = NULL;
  }
  
  if (state->finfo != NULL)
  {
    mem_free(state->finfo);
    state->finfo = NULL;
  }
  return err;
}

/* ���û����ļ���·��(rootpath)����ǰ�ļ���·��(state->path)���ļ���(filename)��������, ����buffer�������� */
// buffer��ԭ�����ݻᱻ���Բ����, bufsizeΪ�������Ĵ�С
// puserpathΪ�������, �����������û��ļ���Ϊ��Ŀ¼���ļ�·��
int ftpd_fullpath(const struct ftpd_state *state, char *buffer, int bufsize, const char *filename, char **puserpath)
{
  int basepos, ret;

  // �ڻ�������׼�����û��ļ��е�·��
  if (state->userid == -1)
    return -1; // δ��¼, ����ʧ��
  basepos = strlen(ftpd_users[state->userid].rootpath);
  if (basepos + 1 > bufsize)
    return -1; // ����������
  strcpy(buffer, ftpd_users[state->userid].rootpath);

  // ��ȡ������û��ļ��е��ļ�·��
  if (buffer[basepos - 1] == '/')
    basepos--; // ʹuserpath�ĵ�һ���ַ�Ϊб��
               // ���û��б��, ��userpathָ��\0, ��������·������ܻ���б��
  if (puserpath != NULL)
    *puserpath = buffer + basepos;

  // ����state->path�ַ���
  if (filename == NULL || filename[0] != '/')
  {
    // filename�������û��ļ���Ϊ��Ŀ¼�ľ���·��, ���������state->path�����·��
    // ��Ҫ��rootpath, state->path��filename�������ַ�������һ��
    // filename == NULL���������Ϊ���ַ���, �����·��
    LWIP_ASSERT("state->path[0] == '/'", state->path[0] == '/'); // state->path�����ַ�ʼ��Ϊб��
    ret = ftpd_concat_path(buffer, bufsize, state->path + 1);
    if (ret == -1)
      return -1;
  }
  else
  {
    // filename�����û��ļ���Ϊ��Ŀ¼�ľ���·��
    // ����б���ַ�, ֻ��rootpath�Ͳ�����б�ܵ�filename������
    filename++;
  }

  // ����filename�ַ���
  ret = ftpd_concat_path(buffer, bufsize, filename);
  if (ret == -1)
    return -1;
  ret = ftpd_simplify_path(buffer, basepos);
  if (puserpath != NULL && **puserpath == '\0')
    *puserpath = "/"; // ������ս�������û���Ŀ¼, ��ôӦ������б�ܱ�ʾ, �����ǿ��ַ���
  return ret;
}

/* �ж�������û����������Ƿ���ȷ */
static int ftpd_is_valid_user(struct ftpd_user *user, int *pid)
{
  int i;
  int n = LWIP_ARRAYSIZE(ftpd_users);

  if (user->name == NULL)
    return 0; // δ�����û���

  for (i = 0; i < n; i++)
  {
    if (strcasecmp(user->name, ftpd_users[i].user.name) == 0)
    {
      if (pid != NULL)
        *pid = i;
      if (ftpd_users[i].user.password == NULL)
        return 1; // �κ����붼����
      else if (user->password != NULL && strcmp(user->password, ftpd_users[i].user.password) == 0)
        return 1; // ������ȷ
      else
        return 0; // �������
    }
  }
  return 0; // �û���������
}

/* ����ftpd������ */
int ftpd_init(void)
{
  err_t err;
  struct tcp_pcb *temp;

  memset(ftpd_data_temp,0x0,sizeof(ftpd_data_temp));
  
  if (ftpd_tpcb != NULL)
  {
    LWIP_DEBUGF(FTPD_DEBUG | LWIP_DBG_LEVEL_WARNING, ("%s: FTPD server is already started!\r\n", __FUNCTION__));
    return -1;
  }
  
  temp = tcp_new();
  if (temp == NULL)
  {
    LWIP_DEBUGF(FTPD_DEBUG | LWIP_DBG_LEVEL_SERIOUS, ("%s: tcp_new() failed!\r\n", __FUNCTION__));
    return -1;
  }
  
  err = tcp_bind(temp, IP_ANY_TYPE, FTPD_PORT);
  if (err != ERR_OK)
  {
    LWIP_DEBUGF(FTPD_DEBUG | LWIP_DBG_LEVEL_SERIOUS, ("%s: tcp_bind() failed! err=%d\r\n", __FUNCTION__, err));
    tcp_close(temp);
    return -1;
  }
  
  ftpd_tpcb = tcp_listen(temp);
  if (ftpd_tpcb == NULL)
  {
    LWIP_DEBUGF(FTPD_DEBUG | LWIP_DBG_LEVEL_SERIOUS, ("%s: tcp_listen() failed!\r\n", __FUNCTION__));
    tcp_close(temp);
    return -1;
  }
  temp = NULL;
  
  tcp_accept(ftpd_tpcb, ftpd_accept);
  return 0;
}

/* memchr�ķ���汾 */
void *ftpd_memrchr(const void *s, int c, size_t n)
{
  const char *p = s;
  int i;

  for (i = n - 1; i >= 0; i--)
  {
    if (p[i] == c)
      return (void *)&p[i];
  }
  return NULL;
}

/* ׼������������ */
// ����������-1, ���ʾ���ӽ���ʧ��, ��ʱ�ѷ�����425��Ϣ, �����ٷ�������������Ϣ
static int ftpd_prepare_data(struct ftpd_state *state)
{
  err_t err;
  int ret = -1;
  
  if (state->dataport == -1)
  {
    ftpd_send_msg(state, "425 Use PORT or PASV first.\r\n");
    return -1;
  }
  
#if FTPD_PASV
  if (state->flags & FTPD_FLAG_PASSIVE)
  {
    LWIP_ASSERT("state->dataconn != NULL", state->dataconn != NULL);
    ret = 0;
  }
  else
  {
#endif
    LWIP_ASSERT("state->dataconn == NULL", state->dataconn == NULL);
    state->dataconn = tcp_new();
    if (state->dataconn == NULL)
    {
      LWIP_DEBUGF(FTPD_DEBUG | LWIP_DBG_LEVEL_SERIOUS, ("%s: tcp_new() failed!\r\n", __FUNCTION__));
      goto end;
    }
    
    tcp_arg(state->dataconn, state);
    err = tcp_connect(state->dataconn, &state->ctrlconn->remote_ip, state->dataport, ftpd_data_connected);
    if (err == ERR_OK)
    {
      // ʹ��PORTģʽʱ, ��ý����Եķ���ǽ�ر�, ������������ϵ��Զ�����
      LWIP_DEBUGF(FTPD_DEBUG, ("FTPD is connecting to [%s]:%d...\r\n", ipaddr_ntoa(&state->ctrlconn->remote_ip), state->dataport));
      tcp_err(state->dataconn, ftpd_data_err);
      state->flags |= FTPD_FLAG_NEWDATACONN;
      ret = 0;
    }
    else
      LWIP_DEBUGF(FTPD_DEBUG | LWIP_DBG_LEVEL_SERIOUS, ("%s: tcp_connect() failed! err=%d\r\n", __FUNCTION__, err));
#if FTPD_PASV
  }
#endif

end:
  if (ret == -1)
  {
    ftpd_send_msg(state, "425 Failed to establish connection.\r\n");
    if (state->dataconn != NULL)
    {
      tcp_arg(state->dataconn, NULL);
      tcp_close(state->dataconn);
      state->dataconn = NULL;
      state->dataport = -1;
    }
  }
  return ret;
}

/* �������� */
static void ftpd_process_cmd(struct ftpd_state *state)
{
  int ret;
  
  // ֻ�е���һ����������л�Ӧ���������ʱ, �ſ�ʼ������һ������
  if (state->sent != 0)
    return;
  else if (state->flags & FTPD_FLAG_TCPERROR)
    goto end;
  
  if ((state->flags & FTPD_FLAG_AGAIN) == 0)
  {
    // ��һ��������ִ�����
    if (state->flags & (FTPD_FLAG_CLOSE | FTPD_FLAG_SHUTDOWN))
    {
      if (state->queue != NULL) 
      {
        pbuf_free(state->queue);  
        state->queue = NULL;      
      }
      ftpd_free(state);
      return;
    }
    
    // �ӽ��ն�����ȡ��һ��������
    ret = ftpd_copy_cmd(state);
    if (ret == 0)
      return; // �������
    LWIP_DEBUGF(FTPD_DEBUG, ("%s\r\n", state->cmd));
    state->cmdstep = 0; // ��ǰ������ĵ�һ������
    
    if (state->queue != NULL) 
    {
        pbuf_free(state->queue); 
        state->queue = NULL;      
    }

    if (ret == 2)
    {
      ftpd_send_msg(state, "500 Syntax error, command unrecognized.\r\n");
      goto end;
    }
    
    // ��ȡ���������
    state->cmdarg = strchr(state->cmd, ' ');
    if (state->cmdarg != NULL)
      *state->cmdarg++ = '\0';
    else
      state->cmdarg = "";
  }
  else
  {
    // ��һ������δִ�����, ��Ȼstate->sent==0, ������Ҫ�������͸�������
    // cmd��cmdarg����, ����ɸ���cmdstep��ֵ������ǰ�ǵڼ�������
    state->flags &= ~FTPD_FLAG_AGAIN;
  }
  
  // ������������
  fmsh_print("cmd %s \r\n",state->cmd);
  if (ftpd_process_user_cmd(state)) // ��������һ������
    ;
  else if (ftpd_process_data_cmd(state))
    ;
  else if (ftpd_process_directory_cmd(state))
    ;
  else if (ftpd_process_file_cmd(state))
    ;
  else if (ftpd_process_opt_cmd(state))
    ;
  else
    ftpd_send_msg(state, "500 Unknown command.\r\n");
  
end:
  // TCP�޷���������ʱ, ǿ�ƹر�����
  if (state->sent == 0 && state->flags & FTPD_FLAG_TCPERROR)
  {
    if (state->queue != NULL) 
    {
        pbuf_free(state->queue);  
        state->queue = NULL;      
    }
    state->flags = (state->flags & ~FTPD_FLAG_AGAIN) | FTPD_FLAG_SHUTDOWN;
    ftpd_free(state);
  }
}

/* ���������������йص����� */
static int ftpd_process_data_cmd(struct ftpd_state *state)
{
  char ip[IPADDR_STRLEN_MAX];
  int i, j, ret;
  int isport = 0;
  ip_addr_t ipaddr;
#if FTPD_PASV
  char buffer[100];
  err_t err;
  int ispasv = 0;
  struct tcp_pcb *newpcb;
#endif
  
#if LWIP_IPV6
  if (IP_IS_V4_VAL(state->ctrlconn->remote_ip))
  {
#endif
    if (strcasecmp(state->cmd, "PORT") == 0)
      isport = 4;
#if FTPD_PASV
    else if (strcasecmp(state->cmd, "PASV") == 0)
      ispasv = 4;
#endif
#if LWIP_IPV6
  }
  else if (IP_IS_V6_VAL(state->ctrlconn->remote_ip))
  {
    if (strcasecmp(state->cmd, "EPRT") == 0)
      isport = 6;
#if FTPD_PASV
    else if (strcasecmp(state->cmd, "EPSV") == 0)
      ispasv = 6;
#endif
  }
#endif
  
  if (isport)
  {
    // ���֮ǰ������PASVģʽ, ��رմ����ļ�������
    state->dataport = -1;
#if FTPD_PASV
    if (state->flags & FTPD_FLAG_PASSIVE)
    {
      state->flags &= ~FTPD_FLAG_PASSIVE;
      if (state->dataconn != NULL)
      {
        tcp_close(state->dataconn);
        state->dataconn = NULL;
      }
    }
#endif
    
    // ��ȡ��IP��ַ
#if LWIP_IPV6
    if (isport == 4)
    {
#endif
      for (i = j = 0; i < sizeof(ip) && j < 4; i++)
      {
        if (isdigit(state->cmdarg[i]))
          ip[i] = state->cmdarg[i];
        else if (state->cmdarg[i] == ',')
        {
          ip[i] = '.';
          j++;
        }
        else
          break;
      }
      if (j != 4)
        goto porterr;
      ip[i - 1] = '\0';
#if LWIP_IPV6
    }
    else
    {
      if (memcmp(state->cmdarg, "|2|", 3) != 0)
        goto porterr;
      
      for (i = 0; i < sizeof(ip); i++)
      {
        if (state->cmdarg[3 + i] == '|')
          break;
        ip[i] = state->cmdarg[3 + i];
      }
      if (i == sizeof(ip))
        goto porterr;
      ip[i] = '\0';
    }
#endif
    
    ret = ipaddr_aton(ip, &ipaddr);
    if (ret == 0 || !ip_addr_cmp(&ipaddr, &state->ctrlconn->remote_ip))
      goto porterr;
    
    // ��ȡ���˿ں�
#if LWIP_IPV6
    if (isport == 4)
    {
#endif
      ret = sscanf(state->cmdarg + i, "%d,%d", &i, &j);
      if (ret != 2)
        goto porterr;
      ret = i * 256 + j;
#if LWIP_IPV6
    }
    else
    {
      i = sscanf(state->cmdarg + 4 + i, "%d", &ret);
      if (i != 1)
        goto porterr;
    }
#endif
    
    if (ret != 0 && ret < 65536)
    {
      state->dataport = ret;
#if LWIP_IPV6
      if (isport == 4)
      {
#endif
#if FTPD_PASV
        ftpd_send_msg(state, "200 PORT command successful. Consider using PASV.\r\n");
#else
        ftpd_send_msg(state, "200 PORT command successful.\r\n");
#endif
#if LWIP_IPV6
      }
      else
        ftpd_send_msg(state, "200 EPRT command successful.\r\n");
#endif
      return 1;
    }
porterr:
#if LWIP_IPV6
    if (isport == 4)
#endif
      ftpd_send_msg(state, "500 Illegal PORT command.\r\n");
#if LWIP_IPV6
    else
      ftpd_send_msg(state, "500 Illegal EPRT command.\r\n");
#endif
  }
#if FTPD_PASV
  else if (ispasv)
  {
    if (state->dataconn == NULL)
    {
      state->dataconn = tcp_new();
      if (state->dataconn == NULL)
        goto pasverr;
      
#if LWIP_IPV6
      if (ispasv == 4)
#endif
        err = tcp_bind(state->dataconn, IP_ADDR_ANY, 0);
#if LWIP_IPV6
      else
        err = tcp_bind(state->dataconn, IP6_ADDR_ANY, 0);
#endif
      if (err != ERR_OK)
        goto pasverr;
      
      newpcb = tcp_listen(state->dataconn);
      if (newpcb == NULL)
        goto pasverr;
      
      state->dataconn = newpcb;
      tcp_arg(state->dataconn, state);
      tcp_accept(state->dataconn, ftpd_data_accept);
      
      state->dataport = state->dataconn->local_port;
      state->flags |= FTPD_FLAG_NEWDATACONN | FTPD_FLAG_PASSIVE;
    }
    
#if LWIP_IPV6
    if (ispasv == 4)
    {
#endif
      ipaddr_ntoa_r(&state->ctrlconn->local_ip, ip, sizeof(ip));
      for (i = 0; ip[i] != '\0'; i++)
      {
        if (ip[i] == '.')
          ip[i] = ',';
      }
      sprintf(buffer, "227 Entering Passive Mode (%s,%d,%d).\r\n", ip, (state->dataport >> 8) & 0xff, state->dataport & 0xff);
#if LWIP_IPV6
    }
    else
      sprintf(buffer, "229 Entering Extended Passive Mode (|||%d|).\r\n", state->dataport);
#endif
    ftpd_send_msg(state, buffer);
    return 1;
pasverr:
#if LWIP_IPV6
    if (ispasv == 4)
#endif
      ftpd_send_msg(state, "500 PASV command failed.\r\n");
#if LWIP_IPV6
    else
      ftpd_send_msg(state, "500 EPSV command failed.\r\n");
#endif
    if (state->dataconn != NULL)
    {
      tcp_close(state->dataconn);
      state->dataconn = NULL;
    }
  }
#endif
  else
    return 0;
  
  return 1;
}

static int ftpd_process_directory_cmd(struct ftpd_state *state)
{
  char buffer[MAX_PATH];
  char *path;
  int ret;
  DIR *dp = NULL;
  FRESULT fr;

  if (strcasecmp(state->cmd, "PWD") == 0)
  {
    ftpd_send_msg(state, "257 \"");
    ftpd_send_msg(state, state->path);
    ftpd_send_msg(state, "\" is the current directory.\r\n");
  }
  else if (strcasecmp(state->cmd, "CWD") == 0)
  {
    ret = ftpd_fullpath(state, buffer, MAX_PATH, state->cmdarg, &path);
    if (ret == -1)
      goto cwderr;
    else if (!ftpd_file_exists(buffer))
      goto cwderr;
    
    strcpy(state->path, path);
    ftpd_send_msg(state, "250 Directory successfully changed.\r\n");
    return 1;
cwderr:
    ftpd_send_msg(state, "550 Failed to change directory.\r\n");
  }
  else if (strcasecmp(state->cmd, "LIST") == 0)
  {
    if (state->cmdstep == 0)
    {
      ret = ftpd_fullpath(state, buffer, MAX_PATH, NULL, NULL);
      if (ret == -1)
        goto listerr;
      
      LWIP_ASSERT("state->dp == NULL", state->dp == NULL);
      dp = mem_malloc(sizeof(DIR));
      if (dp == NULL)
      {
        LWIP_DEBUGF(FTPD_DEBUG | LWIP_DBG_LEVEL_SERIOUS, ("%s: mem_malloc(sizeof(DIR)) failed!\r\n", __FUNCTION__));
        goto listerr;
      }
      
      fr = f_opendir(dp, buffer);
      if (fr != FR_OK)
        goto listerr;
      state->dp = dp; // �ļ��д���֮����ܸ���state->dp
      
      ret = ftpd_prepare_data(state);
      if (ret == -1)
        goto listerr2;
      
      state->cmdstep = 1;
      state->flags |= FTPD_FLAG_AGAIN;
      ftpd_send_msg(state, "150 Here comes the directory listing.\r\n");
      return 1;
listerr:
      state->cmdstep = FTPD_CMDSTEP_CONNABORTED;
listerr2:
      // �����漰��������ͬ�Ĳ���
      // һ���ǹر��ļ���, ��һ�����ͷŴ洢�ļ�����Ϣ���ڴ�
      if (state->dp != NULL)
      {
        f_closedir(state->dp);
        state->dp = NULL;
      }
      if (dp != NULL)
      {
        mem_free(dp);
        dp = NULL;
      }
    }
    else if (state->cmdstep == 1)
    {
      state->flags |= FTPD_FLAG_AGAIN;
      ftpd_data_check(state);
    }
    else if (state->cmdstep == 2)
      ftpd_send_msg(state, "226 Directory send OK.\r\n");
    
    if (state->cmdstep & (FTPD_CMDSTEP_CONNABORTED | FTPD_CMDSTEP_CONNSHUTDOWN))
      ftpd_send_msg(state, "450 Failed to list the folder.\r\n");
  }
  else if (strcasecmp(state->cmd, "MKD") == 0)
  {
    ret = ftpd_fullpath(state, buffer, MAX_PATH, state->cmdarg, &path);
    if (ret != -1)
    {
      fr = f_mkdir(buffer);
      if (fr == FR_OK)
      {
        ftpd_send_msg(state, "257 \"");
        ftpd_send_msg(state, path);
        ftpd_send_msg(state, "\" created.\r\n");
        return 1;
      }
    }
    ftpd_send_msg(state, "550 Create directory operation failed.\r\n");
  }
  else if (strcasecmp(state->cmd, "RMD") == 0)
  {
    ret = ftpd_fullpath(state, buffer, MAX_PATH, state->cmdarg, NULL);
    if (ret != -1)
    {
      fr = f_unlink(buffer);
      if (fr == FR_OK)
      {
        ftpd_send_msg(state, "250 Remove directory operation successful.\r\n");
        return 1;
      }
    }
    ftpd_send_msg(state, "550 Remove directory operation failed.\r\n");
  }
  else
    return 0;
  return 1;
}

/* �������ļ��йص����� */
static int ftpd_process_file_cmd(struct ftpd_state *state)
{
  char buffer[MAX_PATH];
  int ret;
  long size;
  FIL *fp = NULL;
  FRESULT fr;

  if (strcasecmp(state->cmd, "SIZE") == 0)
  {
    ret = ftpd_fullpath(state, buffer, MAX_PATH, state->cmdarg, NULL);
    if (ret == -1)
      goto sizeerr;
    
    fp = mem_malloc(sizeof(FIL));
    if (fp == NULL)
    {
      LWIP_DEBUGF(FTPD_DEBUG | LWIP_DBG_LEVEL_SERIOUS, ("%s: mem_malloc(sizeof(FIL)) failed!\r\n", __FUNCTION__));
      goto sizeerr;
    }
    
    fr = f_open(fp, buffer, FA_READ);
    if (fr != FR_OK)
    {
      LWIP_DEBUGF(FTPD_DEBUG | LWIP_DBG_LEVEL_SERIOUS, ("%s: hgy 1 f_open() failed! fr=%d\r\n", __FUNCTION__, fr));
      goto sizeerr;
    }
    
    size = f_size(fp);
    f_close(fp);
    mem_free(fp);
    
    sprintf(buffer, "213 %ld\r\n", size);
    ftpd_send_msg(state, buffer);
    return 1;
sizeerr:
    ftpd_send_msg(state, "550 Could not get file size.\r\n");
    if (fp != NULL)
      mem_free(fp);
  }
  else if (strcasecmp(state->cmd, "RETR") == 0)
  {
    if (state->cmdstep == 0)
    {
      ret = ftpd_fullpath(state, buffer, MAX_PATH, state->cmdarg, NULL);
      if (ret == -1)
        goto retrerr;
      
      LWIP_ASSERT("state->fp == NULL", state->fp == NULL);
      fp = mem_malloc(sizeof(FIL));
      if (fp == NULL)
      {
        LWIP_DEBUGF(FTPD_DEBUG | LWIP_DBG_LEVEL_SERIOUS, ("%s: mem_malloc(sizeof(FIL)) failed!\r\n", __FUNCTION__));
        goto retrerr;
      }
      
      fr = f_open(fp, buffer, FA_READ);
      if (fr != FR_OK)
      {
        LWIP_DEBUGF(FTPD_DEBUG | LWIP_DBG_LEVEL_SERIOUS, ("%s: f_open() failed! fr=%d\r\n", __FUNCTION__, fr));
        goto retrerr;
      }
      state->fp = fp; // �ļ�����֮����ܸ���state->fp
      
      ret = ftpd_prepare_data(state);
      if (ret == -1)
        goto retrerr2;
      
      state->cmdstep = 1;
      state->flags |= FTPD_FLAG_AGAIN;
      
      sprintf(buffer, "150 Opening %s mode data connection for ", (state->type == 'I') ? "BINARY" : "ASCII");
      ftpd_send_msg(state, buffer);
      ftpd_send_msg(state, state->cmdarg);
      
      size = f_size(state->fp);
      sprintf(buffer, " (%ld bytes).\r\n", size);
      ftpd_send_msg(state, buffer);
      return 1;
retrerr:
      ftpd_send_msg(state, "550 Failed to open file.\r\n");
retrerr2:
      if (state->fp != NULL)
      {
        f_close(state->fp);
        state->fp = NULL;
      }
      if (fp != NULL)
      {
        mem_free(fp);
        fp = NULL;
      }
    }
    else if (state->cmdstep == 1)
    {
      state->flags |= FTPD_FLAG_AGAIN;
      ftpd_data_check(state);
    }
    else if (state->cmdstep == 2)
      ftpd_send_msg(state, "226 Transfer complete.\r\n");
    else if (state->cmdstep & (FTPD_CMDSTEP_CONNSHUTDOWN | FTPD_CMDSTEP_CONNABORTED))
      ftpd_send_msg(state, "451 Requested action aborted: local error in processing.\r\n");
  }
  else if (strcasecmp(state->cmd, "STOR") == 0)
  {
    if (state->cmdstep == 0)
    {
      ret = ftpd_fullpath(state, buffer, MAX_PATH, state->cmdarg, NULL);
      if (ret == -1)
        goto storerr;
      
      LWIP_ASSERT("state->fp == NULL", state->fp == NULL);
      fp = mem_malloc(sizeof(FIL));
      if (fp == NULL)
      {
        LWIP_DEBUGF(FTPD_DEBUG | LWIP_DBG_LEVEL_SERIOUS, ("%s: mem_malloc(sizeof(FIL)) failed!\r\n", __FUNCTION__));
        goto storerr;
      }
      
      fr = f_open(fp, buffer, FA_CREATE_ALWAYS | FA_WRITE);
      if (fr != FR_OK)
      {
        LWIP_DEBUGF(FTPD_DEBUG | LWIP_DBG_LEVEL_SERIOUS, ("%s: f_open() failed! fr=%d, path=\"%s\"\r\n", __FUNCTION__, fr, buffer));
        goto storerr;
      }
      state->fp = fp; // �ļ�����֮����ܸ���state->fp
      
      ret = ftpd_prepare_data(state);
      if (ret == -1)
        goto storerr2;
      
      state->cmdstep = 1;
      state->flags |= FTPD_FLAG_AGAIN;
      ftpd_send_msg(state, "150 Ok to send data.\r\n");
      return 1;
storerr:
      ftpd_send_msg(state, "550 Failed to open file.\r\n");
storerr2:
      if (state->fp != NULL)
      {
        f_close(state->fp);
        state->fp = NULL;
      }
      if (fp != NULL)
      {
        mem_free(fp);
        fp = NULL;
      }
    }
    else if (state->cmdstep == 1)
    {
      state->flags |= FTPD_FLAG_AGAIN;
      ftpd_data_check(state);
    }
    else if (state->cmdstep & FTPD_CMDSTEP_CONNSHUTDOWN)
      ftpd_send_msg(state, "226 Transfer complete.\r\n");
    else if (state->cmdstep & FTPD_CMDSTEP_CONNABORTED)
      ftpd_send_msg(state, "451 Requested action aborted: local error in processing.\r\n");
  }
  else if (strcasecmp(state->cmd, "DELE") == 0)
  {
    ret = ftpd_fullpath(state, buffer, MAX_PATH, state->cmdarg, NULL);
    if (ret != -1)
    {
      ret = f_unlink(buffer);
      if (ret == 0)
      {
        ftpd_send_msg(state, "250 Delete operation successful.\r\n");
        return 1;
      }
    }
    ftpd_send_msg(state, "550 Delete operation failed.\r\n");
  }
  else if (strcasecmp(state->cmd, "RNFR") == 0)
  {
    ret = ftpd_fullpath(state, state->rename, sizeof(state->rename), state->cmdarg, NULL);
    if (ret != -1)
    {
      state->flags |= FTPD_FLAG_RENAME;
      ftpd_send_msg(state, "350 Ready for RNTO.\r\n");
    }
    else
    {
      state->flags &= ~FTPD_FLAG_RENAME;
      ftpd_send_msg(state, "550 RNFR command failed.\r\n");
    }
  }
  else if (strcasecmp(state->cmd, "RNTO") == 0)
  {
    if ((state->flags & FTPD_FLAG_RENAME) == 0)
    {
      ftpd_send_msg(state, "503 RNFR required first.\r\n");
      return 1;
    }

    state->flags &= ~FTPD_FLAG_RENAME;
    ret = ftpd_fullpath(state, buffer, MAX_PATH, state->cmdarg, NULL);
    if (ret != -1)
    {
      ret = f_rename(state->rename, buffer);
      if (ret == 0)
      {
        ftpd_send_msg(state, "250 Rename successful.\r\n");
        return 1;
      }
    }
    ftpd_send_msg(state, "550 Rename failed.\r\n");
  }
  else
    return 0;
  return 1;
}

/* �����������ѡ���йص����� */
static int ftpd_process_opt_cmd(struct ftpd_state *state)
{
  if (strcasecmp(state->cmd, "opts") == 0)
  {
    if (strcasecmp(state->cmdarg, "utf8 on") == 0)
      ftpd_send_msg(state, "200 Always in UTF8 mode.\r\n");
    else
      ftpd_send_msg(state, "501 Option not understood.\r\n");
  }
  else if (strcasecmp(state->cmd, "TYPE") == 0)
  {
    if (strcasecmp(state->cmdarg, "A") == 0)
      ftpd_send_msg(state, "200 Switching to ASCII mode.\r\n");
    else if (strcasecmp(state->cmdarg, "I") == 0)
      ftpd_send_msg(state, "200 Switching to Binary mode.\r\n");
    else
    {
      ftpd_send_msg(state, "500 Unrecognised TYPE command.\r\n");
      return 1;
    }
    state->type = state->cmdarg[0];
  }
  else if (strcasecmp(state->cmd, "noop") == 0)
    ftpd_send_msg(state, "200 NOOP ok.\r\n");
  else
    return 0;
  return 1;
}

/* �������û��йص����� */
static int ftpd_process_user_cmd(struct ftpd_state *state)
{
  int userid;
  
  if (strcasecmp(state->cmd, "USER") == 0)
  {
    if (state->userid != -1)
      ftpd_send_msg(state, "530 Can't change to another user.\r\n");
    else
    {
      ftpd_change_user(state, state->cmdarg);
      if (strcasecmp(state->cmdarg, "ANONYMOUS") == 0 && ftpd_is_valid_user(&state->user, NULL))
        ftpd_send_msg(state, "331 Anonymous access allowed, send identity (e-mail name) as password.\r\n");
      else
        ftpd_send_msg(state, "331 Please specify the password.\r\n");
    }
  }
  else if (strcasecmp(state->cmd, "PASS") == 0)
  {
    if (state->userid != -1)
      ftpd_send_msg(state, "230 Already logged in.\r\n");
    else if (state->user.name == NULL)
      ftpd_send_msg(state, "503 Login with USER first.\r\n");
    else
    {
      state->user.password = ftpd_strdup(state->cmdarg);
      if (ftpd_is_valid_user(&state->user, &userid))
      {
        if (ftpd_file_exists(ftpd_users[userid].rootpath))
        {
          state->userid = userid;
          ftpd_send_msg(state, "230 Login successful.\r\n");
        }
        else
        {
          ftpd_change_user(state, NULL);
          ftpd_send_msg(state, "530 Please create the home directory \"");
          ftpd_send_msg(state, ftpd_users[userid].rootpath);
          ftpd_send_msg(state, "\" before logging in.\r\n");
        }
      }
      else
      {
        ftpd_change_user(state, NULL);
        ftpd_send_msg(state, "530 Login incorrect.\r\n");
      }
    }
  }
  else if (strcasecmp(state->cmd, "QUIT") == 0)
  {
    ftpd_send_msg(state, "221 Goodbye.\r\n");
    state->flags |= FTPD_FLAG_SHUTDOWN;
  }
  else if (state->userid == -1)
    ftpd_send_msg(state, "530 Please login with USER and PASS.\r\n");
  else
    return 0;
  return 1;
}

static err_t ftpd_recv(void *arg, struct tcp_pcb *tpcb, struct pbuf *p, err_t err)
{
  struct ftpd_state *state = arg;
  
  if (p != NULL)
  {
    // LWIP_DEBUGF(FTPD_DEBUG, ("%s: received %d bytes\r\n", __FUNCTION__, p->tot_len));
    pbuf_ref(p);
    if (state->queue == NULL)
      state->queue = p;
    else
      pbuf_cat(state->queue, p);
  }
  else
  {
    if (state != NULL)
    {
      LWIP_DEBUGF(FTPD_DEBUG, ("FTPD connection [%s]:%d is shutdown by the client!\r\n", ipaddr_ntoa(&tpcb->remote_ip), tpcb->remote_port));
      state->flags |= FTPD_FLAG_CLOSE;
    }
    else
    {
      LWIP_DEBUGF(FTPD_DEBUG, ("FTPD connection [%s]:%d is closed by the client!\r\n", ipaddr_ntoa(&tpcb->remote_ip), tpcb->remote_port));
      return ERR_OK;
    }
  }
  
  ftpd_process_cmd(state);
  return ERR_OK;
}

/* ���ͻ�Ӧ */
static int ftpd_send_msg(struct ftpd_state *state, const char *s)
{
  err_t err;
  int len;
  
  if (state->flags & FTPD_FLAG_TCPERROR)
    return -1;
  
  len = strlen(s);
  LWIP_DEBUGF(FTPD_DEBUG, ("%s", s));
  LWIP_ASSERT("sndbuf >= len", tcp_sndbuf(state->ctrlconn) >= len);
  
  err = tcp_write(state->ctrlconn, s, len, TCP_WRITE_FLAG_COPY);
  if (err != ERR_OK)
  {
    state->flags |= FTPD_FLAG_TCPERROR;
    LWIP_DEBUGF(FTPD_DEBUG | LWIP_DBG_LEVEL_SERIOUS, ("%s: tcp_write() failed! err=%d\r\n", __FUNCTION__, err));
    return -1;
  }
  else 
  {
    tcp_output(state->ctrlconn);  // 立即发送控制连接数据
  }
  
  state->sent += len;
  return len;
}

static err_t ftpd_sent(void *arg, struct tcp_pcb *tpcb, u16_t len)
{
  struct ftpd_state *state = arg;
  
  // LWIP_DEBUGF(FTPD_DEBUG, ("%s: %d bytes of response sent\r\n", __FUNCTION__, len));
  if (state != NULL)
  {
    state->sent -= len;
    if (state->sent == 0)
    {
      if (state->cmdlen != 0)
      {
        LWIP_DEBUGF(FTPD_DEBUG, ("%s: processed %d bytes\r\n", __FUNCTION__, state->cmdlen));
        tcp_recved(state->ctrlconn, state->cmdlen);
        state->cmdlen = 0;
      }
      ftpd_process_cmd(state);
    }
  }
  
  return ERR_OK;
}

/* ȥ��·���е�"./"��"../"�Լ�"//" */
// path����Ϊ����·�� (���Դ��̷�Ҳ���Բ����̷�), ������Ϊ���·��
// basepos���ַ������û���Ŀ¼ĩβ��б�ܵ�λ��, ���ڱ�֤"../"�ں���ʱ�����˵��û���Ŀ¼����
// ����"C:/foo/bar", ����û���Ŀ¼��C:/foo, ��ôbaseposӦ��Ϊ6
int ftpd_simplify_path(char *path, int basepos)
{
  char *base, *p, *pp, *q;
  int len;

  // ���path�Ƿ�Ϊ����·��
  if (*path != '/')
  {
    p = strchr(path, '/');
    if (p == NULL || *(p - 1) != ':')
      return -1; // path������Ϊ���·��
  }

  // ��鲢����basepos����
  len = strlen(path);
  if (basepos < 0)
    basepos = 0;
  else if (basepos > len)
    basepos = len;
  base = path + basepos;
  if (*base != '/' && *base != '\0')
  {
    base = strchr(base, '/');
    if (base == NULL)
      base = path + len;
    basepos = base - path;
  }

  p = base; // ��ǰĿ¼
  pp = base; // ��Ŀ¼
  do
  {
    q = strchr(p + 1, '/');
    if (q != NULL)
    {
      len = q - p;
      if (len == 1 || (len == 2 && memcmp(p, "/.", 2) == 0))
        memmove(p, q, strlen(q) + 1);
      else if (len == 3 && memcmp(p, "/..", 3) == 0)
      {
        memmove(pp, q, strlen(q) + 1);
        p = pp;
        pp = ftpd_memrchr(base, '/', pp - base);
        if (pp == NULL)
          pp = p;
      }
      else
      {
        pp = p;
        p = q;
      }
    }
    else
    {
      len = strlen(p);
      if (len == 1 || (len == 2 && memcmp(p, "/.", 2) == 0))
      {
        if (p == path || *(p - 1) == ':')
          p++;
        *p = '\0';
      }
      else if (len == 3 && memcmp(p, "/..", 3) == 0)
      {
        if (pp == path || *(pp - 1) == ':')
          pp++;
        *pp = '\0';
      }
    }
  } while (q != NULL);
  return 0;
}

/* ����һ���ڴ�ռ�, ���ڳ��ڱ���ֲ������е��ַ���, ���⺯���˳�ʱ�ֲ�����ʧЧ */
char *ftpd_strdup(const char *s)
{
  char *p;
  int len;
  
  len = strlen(s) + 1;
  p = mem_malloc(len);
  if (p != NULL)
    memcpy(p, s, len);
  return p;
}
