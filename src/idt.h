#ifndef IDT_H
#define IDT_H
#define IDT_ENTRIES 256

#include <stdint.h>
#include <stdbool.h>
#include "malloc.h"
#include "osafs2.h"
#include "schedule.h"

extern void LoadAndJump();
extern void default_exception_handler(void);
extern void OSASyscall(void);
extern void invalid_opcode_handler(void);
extern void timer_interrupt_handler(void);
extern void divide_by_zero_handler(void);
extern void keyboard_interrupt_handler(void);
extern void general_protection_fault_handler(void);
extern void page_fault_handler(void);

typedef struct
{
        int code;
        int a;
        int b;
        int c;
        int d;
} SysCall;

typedef struct idt_entry
{
        uint16_t base_low;
        uint16_t selector;
        uint8_t always0;
        uint8_t flags;
        uint16_t base_high;
} idt_entry;

typedef struct idt_ptr
{
        uint16_t limit;
        uint32_t base;
} idt_ptr;

/* PSEUDO-INST */
enum INST
{
        MOV_RN_EXX,
        MOV_EXX_RN,
        MOV_RD_RS,
        ADD_EXX_RN,
        ADD_RN_EXX,
        ADD_RD_RS,
        SUB_EXX_RN,
        SUB_RN_EXX,
        SUB_RD_RS,
        AND_EXX_RN,
        AND_RN_EXX,
        AND_RD_RS,
        OR_EXX_RN,
        OR_RN_EXX,
        OR_RD_RS,
        XOR_EXX_RN,
        XOR_RN_EXX,
        XOR_RD_RS,
        CMP_EXX_RN,
        CMP_RN_EXX,
        CMP_RD_RS,
        PUTS_EDI_ECX,
        ATOI_EAX_ESI,
};

idt_entry idt[256];
void SystemTick();

void Exception(unsigned int cs, unsigned int addr)
{
        PANIC("Exception Error At: %x:%x\nException in `%s`, PID=%d", addr,cs,ActiveTask->name,ActiveTask->pid);
}

void divide_by_zero()
{
        PANIC("You can't divide by zero, Silly!\n");
}

uint32_t update_flags(uint32_t result, uint32_t operand1, uint32_t operand2, int is_addition)
{
        regs[EFLAGS] = regs[EFLAGS] & ~(1 << ZF);
        regs[EFLAGS] |= (result == 0) << ZF;
        regs[EFLAGS] = regs[EFLAGS] & ~(1 << CF);
        if (is_addition)
        {
                regs[EFLAGS] |= (result < operand1 || result < operand2) << CF;
        }
        else
        {
                regs[EFLAGS] |= (result > operand1) << CF;
        }

        return result;
}

uint32_t invalid_opcode(uint8_t *op)
{
        int opcode = op[2];
        int prefix = op[0] | (op[1] << 8);
        int i8A = op[3];
        int i8B = op[4];

        if (prefix != 0xB0F)
        {
                PANIC("Invalid Opcode: %x\n", *op);
        } /* hang */

        switch (opcode)
        {
        case PUTS_EDI_ECX:
        {
                i8A &= 15;
                putsn((char *)regs[EDI], regs[ECX] + 1);
                return (uint32_t)op + 3;
        }

        case ATOI_EAX_ESI:
        {
                i8A &= 15;
                regs[EAX] = atoi((char *)regs[EDI]);
                return (uint32_t)op + 3;
        }

        case MOV_RN_EXX: /* MOV Rn,ExX */
        {
                i8A = i8A & 63;
                i8B = i8B & 15;
                ActiveTask->psuedo_regs[i8A] = regs[i8B];
                break;
        }

        case MOV_EXX_RN: /* MOV ExX,Rn */
        {
                i8A = i8A & 15;
                i8B = i8B & 63;
                regs[i8A] = ActiveTask->psuedo_regs[i8B];
                break;
        }

        case MOV_RD_RS: /* MOV Rd,Rs */
        {
                i8A = i8A & 63;
                i8B = i8B & 63;
                ActiveTask->psuedo_regs[i8A] = ActiveTask->psuedo_regs[i8B];
                break;
        }

        case AND_RN_EXX:
        {
                i8A = i8A & 63;
                i8B = i8B & 15;
                ActiveTask->psuedo_regs[i8A] &= regs[i8B];
                break;
        }

        case AND_EXX_RN:
        {
                i8A = i8A & 15;
                i8B = i8B & 63;
                regs[i8A] &= ActiveTask->psuedo_regs[i8B];
                break;
        }

        case AND_RD_RS:
        {
                i8A = i8A & 63;
                i8B = i8B & 63;
                ActiveTask->psuedo_regs[i8A] &= ActiveTask->psuedo_regs[i8B];
                break;
        }

        case XOR_RN_EXX:
        {
                i8A = i8A & 63;
                i8B = i8B & 15;
                ActiveTask->psuedo_regs[i8A] ^= regs[i8B];
                break;
        }

        case XOR_EXX_RN:
        {
                i8A = i8A & 15;
                i8B = i8B & 63;
                regs[i8A] ^= ActiveTask->psuedo_regs[i8B];
                break;
        }

        case XOR_RD_RS:
        {
                i8A = i8A & 63;
                i8B = i8B & 63;
                ActiveTask->psuedo_regs[i8A] ^= ActiveTask->psuedo_regs[i8B];
                break;
        }

        case OR_RN_EXX:
        {
                i8A = i8A & 63;
                i8B = i8B & 15;
                ActiveTask->psuedo_regs[i8A] |= regs[i8B];
                break;
        }

        case OR_EXX_RN:
        {
                i8A = i8A & 15;
                i8B = i8B & 63;
                regs[i8A] |= ActiveTask->psuedo_regs[i8B];
                break;
        }

        case OR_RD_RS:
        {
                i8A = i8A & 63;
                i8B = i8B & 63;
                ActiveTask->psuedo_regs[i8A] |= ActiveTask->psuedo_regs[i8B];
                break;
        }

        case ADD_EXX_RN: /* ADD ExX,Rn */
        {
                i8A = i8A & 15;
                i8B = i8B & 63;
                regs[i8A] = update_flags(
                    regs[i8A] + ActiveTask->psuedo_regs[i8B],
                    regs[i8A],
                    ActiveTask->psuedo_regs[i8B],
                    true);
                break;
        }

        case ADD_RN_EXX: /* ADD Rn,ExX */
        {
                i8A = i8A & 63;
                i8B = i8B & 15;
                ActiveTask->psuedo_regs[i8A] = update_flags(
                    regs[i8B] + ActiveTask->psuedo_regs[i8A],
                    regs[i8A],
                    ActiveTask->psuedo_regs[i8B],
                    true);
                break;
        }

        case ADD_RD_RS: /* ADD Rd,Rs */
        {
                i8A = i8A & 63;
                i8B = i8B & 63;
                ActiveTask->psuedo_regs[i8A] = update_flags(
                    ActiveTask->psuedo_regs[i8A] + ActiveTask->psuedo_regs[i8B],
                    ActiveTask->psuedo_regs[i8A],
                    ActiveTask->psuedo_regs[i8B],
                    true);
                break;
        }

        case SUB_EXX_RN: /* SUB ExX,Rn */
        {
                i8A = i8A & 15;
                i8B = i8B & 63;
                regs[i8A] = update_flags(
                    regs[i8A] - ActiveTask->psuedo_regs[i8B],
                    regs[i8A],
                    ActiveTask->psuedo_regs[i8B],
                    false);
                break;
        }

        case SUB_RN_EXX: /* SUB Rn,ExX */
        {
                i8A = i8A & 63;
                i8B = i8B & 15;
                ActiveTask->psuedo_regs[i8A] = update_flags(
                    ActiveTask->psuedo_regs[i8A] - regs[i8B],
                    ActiveTask->psuedo_regs[i8B],
                    regs[i8A],
                    false);
                break;
        }

        case SUB_RD_RS:
        {
                i8A = i8A & 63;
                i8B = i8B & 63;
                ActiveTask->psuedo_regs[i8A] = update_flags(
                    ActiveTask->psuedo_regs[i8A] - ActiveTask->psuedo_regs[i8B],
                    ActiveTask->psuedo_regs[i8A],
                    ActiveTask->psuedo_regs[i8B],
                    false);
                break;
        }

        case CMP_EXX_RN: /* SUB ExX,Rn */
        {
                i8A = i8A & 15;
                i8B = i8B & 63;
                update_flags(
                    regs[i8A] - ActiveTask->psuedo_regs[i8B],
                    regs[i8A],
                    ActiveTask->psuedo_regs[i8B],
                    false);
                break;
        }

        case CMP_RN_EXX: /* SUB Rn,ExX */
        {
                i8A = i8A & 63;
                i8B = i8B & 15;
                update_flags(
                    ActiveTask->psuedo_regs[i8A] - regs[i8B],
                    ActiveTask->psuedo_regs[i8B],
                    regs[i8A],
                    false);
                break;
        }

        case CMP_RD_RS:
        {
                i8A = i8A & 63;
                i8B = i8B & 63;
                update_flags(
                    ActiveTask->psuedo_regs[i8A] - ActiveTask->psuedo_regs[i8B],
                    ActiveTask->psuedo_regs[i8A],
                    ActiveTask->psuedo_regs[i8B],
                    false);
                break;
        }

        default:
        {
                PANIC("Invalid Opcode: %x\n", *op);
        }
        }

        return (uint32_t)op + 5;
}

void keyboard_handler()
{
        send_eoi(1);
}

void page_fault()
{
        uint32_t faulting_address;
        __asm__ __volatile__(
            "movl %%cr2, %0;"
            : "=r"(faulting_address));
        send_eoi(0xE);
        PANIC("Page Fault @%x\n", faulting_address);
}

void general_protection_fault(int ds, int ss, int es, int fs, int gs, int cs, int eip)
{
        send_eoi(0xD);
        PANIC("Protection Fault @%X:%x\n\n", cs, eip)
        {
                printf("DS=%d\nSS=%d\nES=%d\nFS=%d\nGS=%d\n", ds, ss, es, fs, gs);
                flush();
                while (1)
                        ;
        }
}

int OSASyscallHandler(int eip, int cs, int flags, int op, int b)
{
        (void)eip;
        (void)cs;
        (void)flags;
        switch (op)
        {
        case 0:
                MarkDead();
                r = b;
                sti();
                while (1)
                        ;
                break;
        case 1:
                putc(b);
                break;
        case 2:
        {
                return getch();
        }
        break;
        }

        return 0;
}

void timer_interrupt()
{
        static int tick = 0;
        ++tick;
        if (tick == 2048)
        {
                LookForDead();
                tick = 0;
        }
        Scheduler();
        send_eoi(0x0);
        LoadAndJump();
        while (1)
                ;
}

void set_idt_entry(int n, uint32_t handler, struct idt_entry *idt)
{
        idt[n].base_low = handler & 0xFFFF;
        idt[n].selector = 0x08;
        idt[n].always0 = 0;
        idt[n].flags = 0x8E;
        idt[n].base_high = (handler >> 16) & 0xFFFF;
}

void init_idt()
{
        struct idt_ptr idtp;
        cli();

        for (int i = 0; i < IDT_ENTRIES; i++)
        {
                set_idt_entry(i, (uint32_t)default_exception_handler, idt);
        }

        set_idt_entry(0x80, (uint32_t)OSASyscall, idt);
        set_idt_entry(0x20, (uint32_t)timer_interrupt_handler, idt);
        set_idt_entry(0x81, (uint32_t)timer_interrupt_handler, idt);
        set_idt_entry(0x21, (uint32_t)keyboard_interrupt_handler, idt);
        set_idt_entry(0x00, (uint32_t)divide_by_zero_handler, idt);
        set_idt_entry(0x0E, (uint32_t)page_fault_handler, idt);
        set_idt_entry(0x0D, (uint32_t)general_protection_fault_handler, idt);
        set_idt_entry(0x06, (uint32_t)invalid_opcode_handler, idt);
        idtp.limit = (sizeof(struct idt_entry) * IDT_ENTRIES) - 1;
        idtp.base = (uint32_t)idt;
        asm volatile("lidt (%0)" : : "r"(&idtp));
#ifdef VERBOSE
        puts("IDT Initialized\n");
#endif
}

#endif