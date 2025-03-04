#ifndef IO_H
#define IO_H
#include <stdint.h>
#include <stdbool.h>

#define	peekb(S,O)		*(unsigned char *)(16uL * (S) + (O))
#define	pokeb(S,O,V)		*(unsigned char *)(16uL * (S) + (O)) = (V)
#define	pokew(S,O,V)		*(unsigned short *)(16uL * (S) + (O)) = (V)
#define	_vmemwr(DS,DO,S,N)	memcpy((char *)((DS) * 16 + (DO)), S, N)

uint16_t inw(uint16_t port)
{
        uint16_t ret;
        asm volatile("inw %1, %0" : "=a"(ret) : "Nd"(port));
        return ret;
}

void outw(uint16_t port, uint16_t value)
{
        asm volatile ("outw %0, %1" : : "a"(value), "Nd"(port));
}

unsigned char inb(unsigned short port)
{
        unsigned char ret;
        asm("inb %1, %0" : "=a"(ret) : "Nd"(port));
        return ret;
}

void outb(unsigned short port, unsigned char value)
{
        asm("outb %0, %1" : : "a"(value), "Nd"(port));
}

/* ATA Shite */

extern void __ata_lba_read();
extern void __ata_lba_write();

void ata_lba_read(uint32_t sector, uint8_t sector_count, void* buffer) 
{
    asm volatile (
        "movl %0, %%eax\n"          // Move sector into EAX
        "movb %1, %%cl\n"           // Move sector_count into CL
        "movl %2, %%edi\n"          // Move buffer into EDI
        "call __ata_lba_read\n"     // Call the __ata_lba_read function
        :
        : "r"(sector), "r"(sector_count), "r"(buffer)
        : "%eax", "%cl", "%edi", "memory"
    );
}

void ata_lba_write(uint32_t sector, uint8_t sector_count, void* buffer)
{
        asm volatile (
                "movl %0, %%eax\n"
                "movb %1, %%cl\n"
                "movl %2, %%edi\n"
                // Your assembly instructions here
                :
                : "r"(sector), "r"(sector_count), "r"(buffer)
                : "%eax", "%cl", "%edi"
        );
        __ata_lba_write();
}

#include <stdint.h>

#define ATA_PRIMARY_BASE 0x1F0
#define ATA_SECONDARY_BASE 0x170
#define ATA_PRIMARY_CTRL 0x3F6
#define ATA_SECONDARY_CTRL 0x376

#define ATA_REG_DATA           0x00
#define ATA_REG_ERROR          0x01
#define ATA_REG_SECTOR_CNT 0x02
#define ATA_REG_LBA_LOW        0x03
#define ATA_REG_LBA_MID        0x04
#define ATA_REG_LBA_HIGH   0x05
#define ATA_REG_DRIVE          0x06
#define ATA_REG_CMD                0x07
#define ATA_REG_STATUS         0x07
#define ATA_REG_ALT_STATUS 0x0C

#define ATA_CMD_READ_PIO  0x20
#define ATA_CMD_WRITE_PIO 0x30

#define ATA_STATUS_ERR        0x01
#define ATA_STATUS_DRQ        0x08
#define ATA_STATUS_SRV        0x10
#define ATA_STATUS_DF         0x20
#define ATA_STATUS_RDY        0x40
#define ATA_STATUS_BSY        0x80

#define ATA_MASTER 0xA0
#define ATA_SLAVE  0xB0

static void ata_io_wait(uint16_t base)
{
        // Introduce a small delay (400ns) by reading the status port
        for (int i = 0; i < 4; i++)
                inb(base + ATA_REG_ALT_STATUS);
}

static int ata_polling(uint16_t base)
{
        ata_io_wait(base);

        while (inb(base + ATA_REG_STATUS) & ATA_STATUS_BSY);

        uint8_t status = inb(base + ATA_REG_STATUS);
        if (status & ATA_STATUS_ERR) return -1;
        if (status & ATA_STATUS_DF) return -1;
        if (!(status & ATA_STATUS_DRQ)) return -1;

        return 0;
}

void ata_select_drive(uint16_t base, uint8_t drive)
{
        outb(base + ATA_REG_DRIVE, drive);
        ata_io_wait(base);
}

#endif
