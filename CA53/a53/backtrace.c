#include <stdint.h>
#include <string.h>
#include "backtrace.h"
#include "portmacro.h"
#include "FreeRTOS.h"
#include "task.h"
#include "symbol_table.h"
#include "fmsh_uart_lib.h" 


#pragma section=".text"
#pragma section="CSTACK"
#pragma section="CSTACK1"
#pragma section="CSTACK2"
#pragma section="CSTACK3"

//extern const uint8_t __Address_start__;
//extern const uint8_t __cstack_base__;
//extern const uint8_t __cstack_limit__;
//extern const uint8_t __text_start__;
//extern const uint8_t __text_end__;
//extern StackType_t task_sp_info(StackType_t sp_rec);
extern void task_sp_info(TaskHandle_t xTask, TASK_SP_INFO *ptaskspinfo);
extern FUartPs_T g_UART;

#define WORDS_PER_LINE 4
typedef uint32_t word_t;
typedef struct {
    uint32_t w[4];
} line_t;
static void inline print_full_line(uintptr_t addr, const word_t *w)
{
    fmsh_print("[0x%016lX]:0x%08X 0x%08X 0x%08X 0x%08X\r\n",
           addr, w[0], w[1], w[2], w[3]);
}
static void inline print_partial_line(uintptr_t addr,
                               const word_t *w,
                               int count)
{
    fmsh_print("[0x%016lX]:", addr);
    for (int i = 0; i < count; i++) 
    {
        fmsh_print(" 0x%08X", w[i]);
    }
    //fmsh_print("\r\n");
}
void dump_stack_lines(uintptr_t start,
                      uintptr_t end)
{
    line_t prev;
    int has_prev = 0;

    uintptr_t repeat_start = 0;
    int repeat_count = 0;

    uintptr_t addr = start;
    for (; addr + sizeof(line_t) <= end; addr += sizeof(line_t)) 
    {

        line_t cur;
        memcpy(&cur, (void *)addr, sizeof(cur));

        if (has_prev && memcmp(&cur, &prev, sizeof(cur)) == 0) {
            if (repeat_count == 0)
                repeat_start = addr - sizeof(line_t);
            repeat_count++;
            continue;
        }
    
        if (repeat_count > 0) {
            fmsh_print("[0x%016lX]\r\n\t..\r\n[0x%016lX]:",
                   repeat_start,
                   addr - sizeof(line_t));
            fmsh_print("0x%08X 0x%08X 0x%08X 0x%08X (x%d)\r\n",
                   prev.w[0], prev.w[1],
                   prev.w[2], prev.w[3],
                   repeat_count + 1);
            repeat_count = 0;
        } else if (has_prev) {
            print_full_line(addr - sizeof(line_t), prev.w);
        }

        prev = cur;
        has_prev = 1;
    }

    if (has_prev) {
        if (repeat_count > 0) {
            uintptr_t last = addr - sizeof(line_t);
            fmsh_print("[0x%016lX]\r\n\t..\r\n[0x%016lX]:",
                   repeat_start, last);
            fmsh_print("0x%08X 0x%08X 0x%08X 0x%08X (x%d)\n",
                   prev.w[0], prev.w[1],
                   prev.w[2], prev.w[3],
                   repeat_count + 1);
        } else {
            print_full_line(addr - sizeof(line_t), prev.w);
        }
    }

    uintptr_t remain = end - addr;
    if (remain > 0) {
        int words = remain / sizeof(word_t);
        if (words > 0) {
            print_partial_line(addr,
                               (word_t *)addr,
                               words);
        }
    }
}
static inline StackType_t get_sp(void)
{
    StackType_t sp;
    __asm volatile("mov %0, sp" : "=r"(sp));
    return sp;
}
static inline uint64_t read_esr_el3(void)
{
    uint64_t v;
    __asm volatile ("mrs %0, esr_el3" : "=r"(v));
    return v;
}

static inline uint64_t read_far_el3(void)
{
    uint64_t v;
    __asm volatile ("mrs %0, far_el3" : "=r"(v));
    return v;
}

static inline uint64_t read_elr_el3(void)
{
    uint64_t v;
    __asm volatile ("mrs %0, elr_el3" : "=r"(v));
    return v;
}
static inline uint64_t read_sp_el0(void)
{
    uint64_t v;
    __asm volatile ("mrs %0, sp_el0" : "=r"(v));
    return v;
}
static inline uint64_t read_spsr_el3(void)
{
    uint64_t v;
    __asm volatile ("mrs %0, spsr_el3" : "=r"(v));
    return v;
}
#if 0
static const char *find_symbol_name(uintptr_t addr, uintptr_t *sym_start_out, uintptr_t *sym_size_out)
{
    (void)sym_size_out;
    const char *best = NULL;
    uintptr_t best_addr = 0;
    for (int i = 0; i < g_symbol_count; ++i) {
        uintptr_t s = g_symbol_table[i].addr;
        if (s <= addr && s >= best_addr) {
            best = g_symbol_table[i].name;
            best_addr = s;
        }
    }
    if (best) {
        if (sym_start_out) *sym_start_out = best_addr;
        if (sym_size_out) {
            /* approximate size by next symbol or code end */
            uintptr_t next = (uintptr_t)__section_end(".text");;
            for (int i = 0; i < g_symbol_count; ++i) {
                uintptr_t s = g_symbol_table[i].addr;
                if (s > best_addr && s < next) next = s;
            }
            *sym_size_out = next - best_addr;
        }
        return best;
    }
    return NULL;
}
#endif
/*
const char *find_symbol_name(uintptr_t addr,
                             uintptr_t *sym_start_out,
                             uintptr_t *sym_size_out)
{
    for (int i = 0; i < g_symbol_count; ++i) {
        uintptr_t start = g_symbol_table[i].addr;
        uintptr_t size  = g_symbol_table[i].size;
        if (size == 0)
            continue;

        if (addr >= start && addr < start + size) {
            if (sym_start_out) *sym_start_out = start;
            if (sym_size_out)  *sym_size_out  = size;
            return g_symbol_table[i].name;
        }
    }
    return NULL;
}
*/
const char *find_symbol_name(uintptr_t addr,
                             uintptr_t *sym_start_out,
                             uintptr_t *sym_size_out,
                             uintptr_t *real_size_out,
                             const char **obj_out)
{
    for (int i = 0; i < g_symbol_count; ++i) {

        uintptr_t start = g_symbol_table[i].addr;
        uintptr_t size  = g_symbol_table[i].size;

        uintptr_t end;

        if (size != 0) {
            end = start + size;
        } else {

            /* size=0 -> 用下一个符号地址推算 */
            if (i + 1 < g_symbol_count)
                end = g_symbol_table[i + 1].addr;
            else
                end = start;
        }

        if (addr >= start && addr < end) {

            if (sym_start_out)
                *sym_start_out = start;

            /* 原始 size */
            if (sym_size_out)
                *sym_size_out = size;

            /* 推算后的真实 size */
            if (real_size_out)
                *real_size_out = end - start;

            if (obj_out)
                *obj_out = g_symbol_table[i].obj;

            return g_symbol_table[i].name;
        }
    }

    return NULL;
}
#if 0
static void print_backtrace( StackType_t cur_sp)
{
  StackType_t end_sp;
  uint32_t cpuid;
  uint64_t text_start_adr, text_end_adr,text_size;
//  uint64_t cstack_start_adr, cstack_end_adr;
//  uint64_t cstack1_start_adr, cstack1_end_adr;
//  uint64_t cstack2_start_adr, cstack2_end_adr;
//  uint64_t cstack3_start_adr, cstack3_end_adr;
  uint64_t cstack_start[4],cstack_end[4];
  StackType_t sp_check;
  const char *fun;
  uintptr_t fun_start_addr, fun_size;
  TASK_SP_INFO cur_task_spinfo;
  
  //taskDISABLE_INTERRUPTS();
  //cur_sp = get_sp();
  // __asm volatile("mov %0, sp" : "=r"(cur_sp));
  cpuid = portGET_CORE_ID();
  
  //fmsh_print("CPU%d, sp:%lx\r\n",cpuid,cur_sp);
  
  text_start_adr = (uint64_t)__section_begin(".text");
  //text_start_adr = (uintptr_t)&__Address_start__;
  text_end_adr = (uint64_t)__section_end(".text");
  //text_end_adr = (uintptr_t)&__text_end__;
  text_size = (uint64_t)__section_size(".text");
  
// cstack_start_adr = (uintptr_t)&__cstack_base__;
// cstack_end_adr = (uintptr_t)&__cstack_limit__;
  cstack_start[0] =  (uint64_t)__section_begin("CSTACK");
  cstack_end[0] =    (uint64_t)__section_end("CSTACK");
  
  cstack_start[1]  = (uint64_t)__section_begin("CSTACK1");
  cstack_end[1] = (uint64_t)__section_end("CSTACK1");
  
  cstack_start[2]  = (uint64_t)__section_begin("CSTACK2");
  cstack_end[2] = (uint64_t)__section_end("CSTACK2");
  
  cstack_start[3]  = (uint64_t)__section_begin("CSTACK3");
  cstack_end[3] = (uint64_t)__section_end("CSTACK3");
  
  //fmsh_print("start_adr: 0x%lx, end_adr: 0x%lx\r\n", text_start_adr, text_end_adr);
  for(uint8_t i=0;i<4;i++)
    fmsh_print("CSTACK%d [0x%lx-0x%lx]\r\n", i,cstack_start[i], cstack_end[i]);
//  fmsh_print("CSTACK start_adr: 0x%lx, end_adr: 0x%lx\r\n", cstack_start_adr, cstack_end_adr);
//  fmsh_print("CSTACK1 start_adr: 0x%lx, end_adr: 0x%lx\r\n", cstack1_start_adr, cstack1_end_adr);
//  fmsh_print("CSTACK2 start_adr: 0x%lx, end_adr: 0x%lx\r\n", cstack2_start_adr, cstack2_end_adr);
//  fmsh_print("CSTACK3 start_adr: 0x%lx, end_adr: 0x%lx\r\n", cstack3_start_adr, cstack3_end_adr);
  
  if((cur_sp >= cstack_start[cpuid]) && (cur_sp <= cstack_end[cpuid]))
  {
      fmsh_print("Current SP is in CSTACK\r\n");
  }
  else
  {
      fmsh_print("Current SP is in thread\r\n");
      task_sp_info(NULL,&cur_task_spinfo);
      end_sp =  cur_task_spinfo.pxEndOfStack;
      fmsh_print("CPU%d, Task Name   : %s\r\n",cpuid,cur_task_spinfo.pTaskName);
      fmsh_print("pxTopOfStack 0x%016X\r\n",cur_task_spinfo.pxTopOfStack);
      fmsh_print("pxStack      0x%016X\r\n",cur_task_spinfo.pxStack);
      fmsh_print("pxEndOfStack 0x%016X\r\n",cur_task_spinfo.pxEndOfStack);
      
  }
  fmsh_print("Stack Trace :");

  for(sp_check = cur_sp; sp_check < end_sp; sp_check +=4)
  {
    if((sp_check == cur_sp) || ((sp_check - cur_sp) %16 ==0))
       fmsh_print("\r\n[0x%016X]:",sp_check);
    fmsh_print("0x%08X  ",*((uint32_t *)(sp_check)));
  }
  fmsh_print("\r\nCall trace: \r\n");

  for(sp_check = cur_sp; sp_check < end_sp; sp_check +=8)
  {
    uintptr_t code_hex = *((uintptr_t *)sp_check);
      if((code_hex < text_start_adr) || (code_hex > text_end_adr)) //out range of section .text
        continue;
      else if(code_hex % 4) //not 4 byte align
        continue;
      else
      {
         fun = find_symbol_name(code_hex,&fun_start_addr, &fun_size);
         if((fun != NULL) && ((code_hex - fun_start_addr) <= text_size))
           fmsh_print("%s(0x%016X)+0x%X (0x%X)\r\n",fun,fun_start_addr, code_hex - fun_start_addr-4, fun_size);
      }
      
  }
  for(;;);
}
#endif
static void print_stack( StackType_t start_sp, StackType_t end_sp)
{
  StackType_t sp_check;
  const char *fun,*file;
  uintptr_t fun_start_addr, fun_size, fun_cal_size;
  uint64_t text_start_adr, text_end_adr,text_size;
  
  text_start_adr = (uint64_t)__section_begin(".text");
  text_end_adr = (uint64_t)__section_end(".text");
  text_size = (uint64_t)__section_size(".text");
  fmsh_print("Stack Trace :\r\n");
  for(sp_check = start_sp; sp_check < end_sp; sp_check +=4)
  {
//    if((sp_check == start_sp) || ((sp_check - start_sp) %16 ==0))
//       fmsh_print("\r\n[0x%016X]:",sp_check);
//    fmsh_print("0x%08X  ",*((uint32_t *)(sp_check)));
  }
  dump_stack_lines(start_sp, end_sp);
  fmsh_print("\r\nCall trace: \r\n");

  for(sp_check = start_sp; sp_check < end_sp; sp_check +=8)
  {
    uintptr_t code_hex = *((uintptr_t *)sp_check);
      if((code_hex < text_start_adr) || (code_hex > text_end_adr)) //out range of section .text
        continue;
      else if(code_hex % 4) //not 4 byte align
        continue;
      else
      {
         fun = find_symbol_name(code_hex,&fun_start_addr, &fun_size, &fun_cal_size, &file);
         if((fun != NULL) && ((code_hex - fun_start_addr) <= text_size))
         {
           uintptr_t code_offset = code_hex - fun_start_addr;
           //fmsh_print("%s(0x%016X)+0x%X (0x%X)\r\n",fun,fun_start_addr, code_hex - fun_start_addr-4, fun_size);
           fmsh_print("%s(0x%016X)+0x%X ",fun,fun_start_addr, (code_offset>0)?(code_offset-4):0);
           fmsh_print("(0x%X%c) %s\r\n",fun_cal_size,(fun_size==0)?'?':' ', file);
         }
      }
      
  }
}
static void print_backtrace( StackType_t cur_sp)
{
  StackType_t end_sp;
  uint32_t cpuid;
  uint64_t cstack_start[4],cstack_end[4];
  TASK_SP_INFO cur_task_spinfo;
  
  cpuid = portGET_CORE_ID();
   
  cstack_start[0] =  (uint64_t)__section_begin("CSTACK");
  cstack_end[0] =    (uint64_t)__section_end("CSTACK");
  
  cstack_start[1]  = (uint64_t)__section_begin("CSTACK1");
  cstack_end[1] = (uint64_t)__section_end("CSTACK1");
  
  cstack_start[2]  = (uint64_t)__section_begin("CSTACK2");
  cstack_end[2] = (uint64_t)__section_end("CSTACK2");
  
  cstack_start[3]  = (uint64_t)__section_begin("CSTACK3");
  cstack_end[3] = (uint64_t)__section_end("CSTACK3");
  
  for(uint8_t i=0;i<4;i++)
    fmsh_print("CSTACK%d [0x%lx-0x%lx]\r\n", i,cstack_start[i], cstack_end[i]);
  
  if((cur_sp >= cstack_start[cpuid]) && (cur_sp <= cstack_end[cpuid]))
  {
      fmsh_print("Current SP is in CSTACK\r\n");
      end_sp = cstack_end[cpuid];
  }
  else
  {
      fmsh_print("Current SP is in thread\r\n");
      task_sp_info(NULL,&cur_task_spinfo);
      end_sp =  cur_task_spinfo.pxEndOfStack;
//      fmsh_print("CPU%d, Task Name   : %s\r\n",cpuid,cur_task_spinfo.pTaskName);
//      fmsh_print("pxTopOfStack 0x%016X\r\n",cur_task_spinfo.pxTopOfStack);
//      fmsh_print("pxStack      0x%016X\r\n",cur_task_spinfo.pxStack);
//      fmsh_print("pxEndOfStack 0x%016X\r\n",cur_task_spinfo.pxEndOfStack);
      
  }
  
  print_stack(cur_sp,end_sp);
  //for(;;);
}

void backtrace(void)
{
     taskDISABLE_INTERRUPTS();
     StackType_t cur_sp = get_sp();
     
     while(FMSH_ReadReg(g_UART.base_address,0x1c) == 0x5f);//check uart is used in backtrace
     FMSH_WriteReg(g_UART.base_address,0x1c, 0x5f);
     
     print_backtrace(cur_sp);
     
     FMSH_WriteReg(g_UART.base_address,0x1c, 0x0);
     
     for(;;);
     
}

void backtrace_abort(void)
{
    taskDISABLE_INTERRUPTS();
    uint64_t cur_elr,cur_sp_el0,cur_esr,cur_far,cur_spsr;
    StackType_t cur_sp = get_sp();
    uint32_t cpuid = portGET_CORE_ID();
    cur_elr = read_elr_el3();
    cur_sp_el0 =  read_sp_el0();
    cur_esr = read_esr_el3();
    cur_far =read_far_el3();
    cur_spsr =read_spsr_el3();
    
     while(FMSH_ReadReg(g_UART.base_address,0x1c) == 0x5f);//check uart is used in backtrace
     FMSH_WriteReg(g_UART.base_address,0x1c, 0x5f);
     
    fmsh_print("Abort in CPU%d...\r\n",cpuid);
    fmsh_print("ESR_EL3:0x%016lX\r\n",cur_esr);
    fmsh_print("EC      = 0x%02lX\t ", (cur_esr >> 26) & 0x3f);
    /*
    | EC (bin) | EC (hex) | 含义                           |
    | -------- | -------- | ---------------------------- |
    | 100000   | 0x20     | Instruction Abort (Lower EL) |
    | 100001   | 0x21     | Instruction Abort (Same EL)  |
    | 100100   | 0x24     | Data Abort (Lower EL)        |
    | 100101   | 0x25     | Data Abort (Same EL)         |

    */
    switch((cur_esr >> 26) & 0x3f)
    {
        case 0x20:
          fmsh_print("Instruction Abort (Lower EL)\r\n");
          break;
        case 0x21:
          fmsh_print("Instruction Abort (Same EL)\r\n");
          break;
        case 0x24:
          fmsh_print("Data Abort (Lower EL)\r\n");
          break;
        case 0x25:
          fmsh_print("Data Abort (Same EL)\r\n");
          break;
        default:
          fmsh_print("Unknow\r\n");

    }
    fmsh_print("ISS     = 0x%06lX\t ", cur_esr & 0x1ffffff);
    /*
    | DFSC/IFSC  | 含义                         | 常见原因  |
    | ---------- | --------------------------- | --------  |
    | `0b000100` | Translation fault (level 0) | 页表没建   |
    | `0b000101` | Translation fault (level 1) | 映射缺失   |
    | `0b000110` | Translation fault (level 2) | 映射缺失   |
    | `0b000111` | Translation fault (level 3) | 映射缺失   |
    | `0b001001` | Access flag fault           | AF 位没置  |
    | `0b001101` | Permission fault            | 读写权限错 |
    | `0b100001` | Alignment fault             | 非对齐访问 |
    | `0b010000` | Synchronous external abort  | AXI/DDR 错 |
    */
    switch(cur_esr & 0x1f)
    {
        case 0b000100:
          fmsh_print("Translation fault (level 0)\r\n");
          break;
        case 0b000101:
          fmsh_print("Translation fault (level 1)\r\n");
          break;
        case 0b000110:
          fmsh_print("Translation fault (level 2)\r\n"); 
          break;
        case 0b000111:
          fmsh_print("Translation fault (level 3)\r\n"); 
          break;
        case 0b001001:
          fmsh_print("Access flag fault\r\n"); 
          break;
        case 0b001101:
          fmsh_print("Permission fault\r\n"); 
          break;
        case 0b100001:
          fmsh_print("Alignment fault\r\n"); 
          break;
        case 0b010000:
          fmsh_print("Synchronous external abort\r\n"); 
          break;
        default :
          fmsh_print("Unknow\r\n"); 
          break;      
    }
    fmsh_print("ELR_EL3:0x%016lX\r\n",cur_elr);
    uintptr_t fun_start_addr, fun_size,fun_cal_size;
    const char *file;
    //const char *fun = find_symbol_name(cur_elr,&fun_start_addr, &fun_size);
    const char *fun =  find_symbol_name(cur_elr,&fun_start_addr, &fun_size, &fun_cal_size, &file);
    if(fun != NULL)
    {
      //fmsh_print("\t%s(0x%016X)+0x%X (0x%X)\r\n",fun,fun_start_addr, cur_elr - fun_start_addr, fun_size);
      fmsh_print("%s(0x%016X)+0x%X ",fun,fun_start_addr, cur_elr - fun_start_addr);
      fmsh_print("(0x%X%c) %s\r\n",fun_cal_size,(fun_size==0)?'?':' ', file);
    }
        

    fmsh_print("FAR_EL3:0x%016lX\r\n",cur_far);
    fmsh_print("SPSR_EL3:0x%016lX\r\n",cur_spsr);
    print_backtrace(cur_sp_el0);
    print_backtrace(cur_sp);
    
    FMSH_WriteReg(g_UART.base_address,0x1c, 0x0);
    for(;;);
}

static void overflow_detail(TaskHandle_t xTask, StackType_t cur_sp, StackType_t cur_el0_sp)
{
  TASK_SP_INFO cur_task_spinfo;
  
  while(FMSH_ReadReg(g_UART.base_address,0x1c) == 0x5f);//check uart is used in backtrace
  FMSH_WriteReg(g_UART.base_address,0x1c, 0x5f);
     
  fmsh_print("SP:0x%016lX\r\n",cur_sp);
  fmsh_print("EL0_SP:0x%016lX\r\n",cur_el0_sp);
  task_sp_info(xTask,&cur_task_spinfo);
  print_stack(cur_task_spinfo.pxStack,cur_task_spinfo.pxEndOfStack);
  
  FMSH_WriteReg(g_UART.base_address,0x1c, 0x0);
  for(;;);
}
void backtrace_overflow(TaskHandle_t xTask)
{
    taskDISABLE_INTERRUPTS();
    uint64_t cur_sp_el0;
    StackType_t cur_sp = get_sp();
    uint32_t cpuid = portGET_CORE_ID();
    cur_sp_el0 =  read_sp_el0();
    overflow_detail(xTask, cur_sp,cur_sp_el0);
}
