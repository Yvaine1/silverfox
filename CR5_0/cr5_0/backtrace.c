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
#pragma section="IRQ_STACK"
#pragma section="SVC_STACK"
#pragma section="FIQ_STACK"

extern uint32_t CSTACK;
extern uint32_t __Size_cstack__;
extern uint32_t IRQ_STACK;
extern uint32_t __Size_irqstack__;
extern uint32_t SVC_STACK;
extern uint32_t __Size_svcstack__;
extern uint32_t FIQ_STACK;
extern uint32_t __Size_fiqstack__;

extern void task_sp_info(TaskHandle_t xTask, TASK_SP_INFO *ptaskspinfo);
extern FUartPs_T g_UART;
RegContext_t g_RegCtx;

#define WORDS_PER_LINE 4
typedef uint32_t word_t;
typedef struct {
    uint32_t w[4];
} line_t;

static void inline print_full_line(uintptr_t addr, const word_t *w) {
    fmsh_print("[0x%08X]:0x%08X 0x%08X 0x%08X 0x%08X\r\n",
               (uint32_t)addr, w[0], w[1], w[2], w[3]);
}

static void inline print_partial_line(uintptr_t addr, const word_t *w, int count) {
    fmsh_print("[0x%08X]:", (uint32_t)addr);
    for (int i = 0; i < count; i++) {
        fmsh_print(" 0x%08X", w[i]);
    }
}

void dump_stack_lines(uintptr_t start, uintptr_t end) {
    line_t prev;
    int has_prev = 0;
    uintptr_t repeat_start = 0;
    int repeat_count = 0;
    uintptr_t addr = start;

    if (start >= end)
        return;

    for (; addr + sizeof(line_t) <= end; addr += sizeof(line_t)) {
        line_t cur;
        memcpy(&cur, (void *)addr, sizeof(cur));

        if (has_prev && memcmp(&cur, &prev, sizeof(cur)) == 0) {
            if (repeat_count == 0)
                repeat_start = addr - sizeof(line_t);
            repeat_count++;
            continue;
        }

        if (repeat_count > 0) {
            fmsh_print("[0x%08X]\r\n\t..\r\n[0x%08X]:",
                       (uint32_t)repeat_start, (uint32_t)(addr - sizeof(line_t)));
            fmsh_print("0x%08X 0x%08X 0x%08X 0x%08X (x%d)\r\n",
                       prev.w[0], prev.w[1], prev.w[2], prev.w[3],
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
            fmsh_print("[0x%08X]\r\n\t..\r\n[0x%08X]:",
                       (uint32_t)repeat_start, (uint32_t)last);
            fmsh_print("0x%08X 0x%08X 0x%08X 0x%08X (x%d)\r\n",
                       prev.w[0], prev.w[1], prev.w[2], prev.w[3],
                       repeat_count + 1);
        } else {
            print_full_line(addr - sizeof(line_t), prev.w);
        }
    }

    uintptr_t remain = end - addr;
    if (remain > 0) {
        int words = remain / sizeof(word_t);
        if (words > 0) {
            print_partial_line(addr, (word_t *)addr, words);
        }
    }
}

static inline StackType_t get_sp(void) {
    StackType_t sp;
    __asm volatile("mov %0, sp" : "=r"(sp));
    return sp;
}

const char *find_symbol_name(uintptr_t addr,
                             uintptr_t *sym_start_out,
                             uintptr_t *sym_size_out,
                             uintptr_t *real_size_out,
                             const char **obj_out) {
    for (int i = 0; i < g_symbol_count; ++i) {
        uintptr_t start = g_symbol_table[i].addr;
        uintptr_t size  = g_symbol_table[i].size;
        uintptr_t end;

        if (size != 0) {
            end = start + size;
        } else {
            if (i + 1 < g_symbol_count)
                end = g_symbol_table[i + 1].addr;
            else
                end = start;
        }

        if (addr >= start && addr < end) {
            if (sym_start_out) *sym_start_out = start;
            if (sym_size_out)  *sym_size_out  = size;
            if (real_size_out) *real_size_out = end - start;
            if (obj_out)       *obj_out       = g_symbol_table[i].obj;
            return g_symbol_table[i].name;
        }
    }
    return NULL;
}

static const char* print_symbol_for_addr(uintptr_t addr, const char *label)
{
    static char buf[64];
    const char *fun, *file;
    uintptr_t fun_start_addr, fun_size, fun_cal_size;
    fun = find_symbol_name(addr, &fun_start_addr, &fun_size, &fun_cal_size, &file);

    if (fun) {
        uintptr_t offset = addr - fun_start_addr;
        snprintf(buf, sizeof(buf), "%s: %s 0x%08X+0x%X (0x%X%c) %s",
                    label, fun,
                    (uint32_t)fun_start_addr,
                    (uint32_t)addr - fun_start_addr,
                    (uint32_t)fun_cal_size,
                    (fun_size == 0) ? '?' : ' ',
                    file ? file : "unknown");
    } else {
        snprintf(buf, sizeof(buf), "no symbol");
    }
    return buf;
}

static void print_stack(StackType_t start_sp, StackType_t end_sp) {
    StackType_t sp_check;
    const char *fun, *file;
    uintptr_t fun_start_addr, fun_size, fun_cal_size;
    uintptr_t text_start_adr, text_end_adr;

    text_start_adr = (uintptr_t)__section_begin(".text");
    text_end_adr   = (uintptr_t)__section_end(".text");

    fmsh_print("Stack Memory Dump:\r\n");
    dump_stack_lines(start_sp, end_sp);

    fmsh_print("\r\nCall trace: \r\n");
    for (sp_check = start_sp; sp_check < end_sp; sp_check += 4) {
        uintptr_t code_hex = *((uintptr_t *)sp_check);
        uintptr_t real_pc = code_hex & ~1UL;

        if ((real_pc & 3U) != 0)
            continue;
        if ((real_pc < text_start_adr) || (real_pc > text_end_adr))
            continue;

        fun = find_symbol_name(real_pc, &fun_start_addr, &fun_size,
                               &fun_cal_size, &file);
        if (fun != NULL) {
            uintptr_t code_offset = real_pc - fun_start_addr;
            fmsh_print("%-30s 0x%08X+0x%08X (0x%08X%c) %-20s\r\n",
                    fun,
                    (uint32_t)fun_start_addr,
                    (uint32_t)code_offset,
                    (uint32_t)fun_cal_size,
                    (fun_size == 0) ? '?' : ' ',
                    file ? file : "unknown");
        }
    }
}

static void print_backtrace(StackType_t cur_sp) {
    StackType_t end_sp;
    uintptr_t cstack_start = (uintptr_t)__section_begin("CSTACK");
    uintptr_t cstack_end   = (uintptr_t)__section_end("CSTACK");
    uintptr_t irqstack_start = (uintptr_t)__section_begin("IRQ_STACK");
    uintptr_t irqstack_end   = (uintptr_t)__section_end("IRQ_STACK");
    uintptr_t svcstack_start = (uintptr_t)__section_begin("SVC_STACK");
    uintptr_t svcstack_end   = (uintptr_t)__section_end("SVC_STACK");
    uintptr_t fiqstack_start = (uintptr_t)__section_begin("FIQ_STACK");
    uintptr_t fiqstack_end   = (uintptr_t)__section_end("FIQ_STACK");

    TASK_SP_INFO cur_task_spinfo;

    fmsh_print("System STACK [0x%08X - 0x%08X]\r\n",
               (uint32_t)cstack_start, (uint32_t)cstack_end);
    fmsh_print("IRQ STACK    [0x%08X - 0x%08X]\r\n",
               (uint32_t)irqstack_start, (uint32_t)irqstack_end);
    fmsh_print("SVC STACK    [0x%08X - 0x%08X]\r\n",
               (uint32_t)svcstack_start, (uint32_t)svcstack_end);
    fmsh_print("FIQ STACK    [0x%08X - 0x%08X]\r\n",
               (uint32_t)fiqstack_start, (uint32_t)fiqstack_end);
    fmsh_print("Current SP 0x%08X located in ", (uint32_t)cur_sp);
    if (cur_sp >= cstack_start && cur_sp <= cstack_end) {
        fmsh_print("system stack\r\n");
        end_sp = cstack_end;
    } else if (cur_sp >= irqstack_start && cur_sp <= irqstack_end) {
        fmsh_print("irq stack\r\n");
        end_sp = irqstack_end;
    } else if (cur_sp >= svcstack_start && cur_sp <= svcstack_end) {
        fmsh_print("svc stack\r\n");
        end_sp = svcstack_end;
    } else if (cur_sp >= fiqstack_start && cur_sp <= fiqstack_end) {
        fmsh_print("fiq stack\r\n");
        end_sp = fiqstack_end;
    } else {
        fmsh_print("task stack\r\n");
        task_sp_info(NULL, &cur_task_spinfo);
        end_sp = cur_task_spinfo.pxEndOfStack;
    }

    print_stack(cur_sp, end_sp);
}

void backtrace(void) {
    taskDISABLE_INTERRUPTS();
    StackType_t cur_sp = get_sp();

    while (FMSH_ReadReg(g_UART.base_address, 0x1c) == 0x5f);
    FMSH_WriteReg(g_UART.base_address, 0x1c, 0x5f);

    print_backtrace(cur_sp);

    FMSH_WriteReg(g_UART.base_address, 0x1c, 0x0);
    for (;;);
}

static const char * const mode_names[] = {
    [0x10] = "User",
    [0x11] = "FIQ",
    [0x12] = "IRQ",
    [0x13] = "Supervisor",
    [0x17] = "Abort",
    [0x1B] = "Undefined",
    [0x1F] = "System"
};

static const char *get_mode_name(uint32_t mode)
{
    if (mode >= 0x10 && mode <= 0x1F && mode_names[mode] != NULL)
        return mode_names[mode];
    return "Unknown";
}

void backtrace_exc(void)
{
    RegContext_t *ctx = &g_RegCtx;
    uint32_t spsr   = ctx->spsr;
    uint32_t cpsr   = ctx->cpsr;
    uint32_t pc     = ctx->pc;
    uint32_t lr     = ctx->lr;
    uint32_t sp     = ctx->sp;
    uint32_t mode   = spsr & 0x1F;
    uintptr_t pc_clean = pc & ~1UL;
    uintptr_t lr_clean = lr & ~1UL;
    uintptr_t lr_minus4 = lr_clean - 4;
    int thumb  = (spsr & (1U << 5)) ? 1 : 0;
    
    while (FMSH_ReadReg(g_UART.base_address, 0x1c) == 0x5f);
    FMSH_WriteReg(g_UART.base_address, 0x1c, 0x5f);

    fmsh_print("\r\n========== EXCEPTION CONTEXT ==========\r\n");
    fmsh_print("Mode: %s, %s state\r\n", get_mode_name(mode), thumb ? "Thumb" : "ARM");

    fmsh_print("\r\n---ARM Registers ---\r\n");
    for (int i = 0; i < 13; i++) {
        fmsh_print("R%-2d  = 0x%08X\r\n", i, ctx->r[i]);
    }

    fmsh_print("LR   = 0x%08X  (%s)\r\n", lr, print_symbol_for_addr(lr_minus4, "LR-4"));
    fmsh_print("SP   = 0x%08X  (original stack)\r\n", sp);
    fmsh_print("PC   = 0x%08X  (%s)\r\n", pc, print_symbol_for_addr(pc_clean, "PC  "));
    fmsh_print("CPSR = 0x%08X  (current exception mode)\r\n", cpsr);
    fmsh_print("SPSR = 0x%08X  (saved CPSR)\r\n", spsr);

    fmsh_print("\r\n--- Stack Backtrace ---\r\n");
    print_backtrace(sp);
    fmsh_print("\r\n========== End of Exception Report ==========\r\n");

    FMSH_WriteReg(g_UART.base_address, 0x1c, 0x0);
    for(;;);
}

static void overflow_detail(TaskHandle_t xTask, StackType_t cur_sp)
{
    TASK_SP_INFO cur_task_spinfo;

    while (FMSH_ReadReg(g_UART.base_address, 0x1c) == 0x5f);
    FMSH_WriteReg(g_UART.base_address, 0x1c, 0x5f);

    fmsh_print("cur SP:0x%016lX\r\n",cur_sp);
    task_sp_info(xTask, &cur_task_spinfo);
    print_stack(cur_task_spinfo.pxStack,cur_task_spinfo.pxEndOfStack);

    FMSH_WriteReg(g_UART.base_address, 0x1c, 0x0);
    for(;;);
}

void backtrace_overflow(TaskHandle_t xTask)
{
    taskDISABLE_INTERRUPTS();
    StackType_t cur_sp = get_sp();
    overflow_detail(xTask, cur_sp);
}
