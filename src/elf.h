#ifndef ELF_H
#define ELF_H
/*
#include <stdint.h>
#include "string.h"
#include "osafs2.h"

#define EI_NIDENT 16
static FILE *file=NULL;

typedef struct
{
        unsigned char e_ident[EI_NIDENT];
        uint16_t e_type;
        uint16_t e_machine;
        uint32_t e_version;
        uint64_t e_entry;
        uint64_t e_phoff;
        uint64_t e_shoff;
        uint32_t e_flags;
        uint16_t e_ehsize;
        uint16_t e_phentsize;
        uint16_t e_phnum;
        uint16_t e_shentsize;
        uint16_t e_shnum;
        uint16_t e_shstrndx;
} Elf64_Ehdr;

typedef struct
{
        uint32_t p_type;
        uint32_t p_flags;
        uint64_t p_offset;
        uint64_t p_vaddr;
        uint64_t p_paddr;
        uint64_t p_filesz;
        uint64_t p_memsz;
        uint64_t p_align;
} Elf64_Phdr;

typedef struct
{
        uint32_t sh_name;
        uint32_t sh_type;
        uint64_t sh_flags;
        uint64_t sh_addr;
        uint64_t sh_offset;
        uint64_t sh_size;
        uint32_t sh_link;
        uint32_t sh_info;
        uint64_t sh_addralign;
        uint64_t sh_entsize;
} Elf64_Shdr;

typedef struct
{
        uint32_t st_name;
        uint8_t  st_info;
        uint8_t  st_other;
        uint16_t st_shndx;
        uint64_t st_value;
        uint64_t st_size;
} Elf64_Sym;

typedef struct
{
        uint64_t r_offset;
        uint64_t r_info;
        int64_t  r_addend;
} Elf64_Rela;

int read_elf_header(FILE *file, Elf64_Ehdr *header)
{
        fseek(file, 0, SEEK_SET);
        return fread(header, sizeof(Elf64_Ehdr), 1, file) != -1;
}

Elf64_Shdr *read_section_headers(FILE *file, Elf64_Ehdr *header)
{
        Elf64_Shdr *shdrs = malloc(header->e_shnum * header->e_shentsize);
        if (!shdrs) return NULL;

        fseek(file, header->e_shoff, SEEK_SET);
        if (fread(shdrs, header->e_shentsize, header->e_shnum, file) != header->e_shnum)
        {
                free(shdrs);
                return NULL;
        }

        return shdrs;
}

Elf64_Shdr *find_section(Elf64_Ehdr *header, Elf64_Shdr *shdrs, const char *name)
{
        Elf64_Shdr *shstrtab = &shdrs[header->e_shstrndx];
        char *shstr = malloc(shstrtab->sh_size);
        if (!shstr) return NULL;

        fseek(file, shstrtab->sh_offset, SEEK_SET);
        fread(shstr, shstrtab->sh_size, 1, file);

        for (int i = 0; i < header->e_shnum; i++)
        {
                if (strcmp(&shstr[shdrs[i].sh_name], name) == 0)
                {
                        free(shstr);
                        return &shdrs[i];
                }
        }

        free(shstr);
        return NULL;
}

void apply_relocations(uint8_t *base_addr, Elf64_Rela *rela, int num_entries, Elf64_Sym *symtab, const char *strtab)
{
        for (int i = 0; i < num_entries; i++)
        {
                uint64_t offset = rela[i].r_offset;
                uint64_t type = rela[i].r_info & 0xFFFFFFFF;
                uint64_t sym_idx = rela[i].r_info >> 32;
                int64_t addend = rela[i].r_addend;

                Elf64_Sym *symbol = &symtab[sym_idx];
                uint64_t sym_value = symbol->st_value;
                const char *sym_name = &strtab[symbol->st_name];

                uint64_t *addr_to_patch = (uint64_t *)(base_addr + offset);
                uint64_t new_value = sym_value + addend;

                switch (type)
                {
                        case 1:
                                *addr_to_patch = new_value;
                                break;
                        case 2:
                                *((uint32_t *)addr_to_patch) = (uint32_t)(new_value - (uint64_t)addr_to_patch);
                                break;
                        default:
                                printf("Unsupported relocation type: %lu\n", type);
                                break;
                }

                printf("(REL) %s at 0x%lx (type %lu) to %x\n", sym_name, offset, type, addr_to_patch);
        }
}

int elf(FILE *f, uint8_t *base_addr)
{
        file=f;
        if (!file)
        {
                perror("Failed to open file");
                return 1;
        }

        Elf64_Ehdr header;
        if (!read_elf_header(file, &header))
        {
                printf("Failed to read ELF header\n");
                fclose(file);
                return 1;
        }

        if (header.e_ident[0] != 0x7F || header.e_ident[1] != 'E' ||
                header.e_ident[2] != 'L' || header.e_ident[3] != 'F')
        {
                printf("Not a valid ELF file\n");
                fclose(file);
                return 1;
        }

        Elf64_Shdr *shdrs = read_section_headers(file, &header);
        if (!shdrs)
        {
                printf("Failed to read section headers\n");
                fclose(file);
                return 1;
        }

        Elf64_Shdr *rela_plt = find_section(&header, shdrs, ".rela.plt");
        if (!rela_plt)
        {
                printf("Failed to find .rela.plt section\n");
                free(shdrs);
                fclose(file);
                return 1;
        }

        Elf64_Shdr *symtab_shdr = find_section(&header, shdrs, ".symtab");
        if (!symtab_shdr)
        {
                printf("Failed to find .symtab section\n");
                free(shdrs);
                fclose(file);
                return 1;
        }

        Elf64_Shdr *strtab_shdr = find_section(&header, shdrs, ".strtab");
        if (!strtab_shdr)
        {
                printf("Failed to find .strtab section\n");
                free(shdrs);
                fclose(file);
                return 1;
        }

        Elf64_Sym *symtab = malloc(symtab_shdr->sh_size);
        fseek(file, symtab_shdr->sh_offset, SEEK_SET);
        fread(symtab, symtab_shdr->sh_size, 1, file);

        char *strtab = malloc(strtab_shdr->sh_size);
        fseek(file, strtab_shdr->sh_offset, SEEK_SET);
        fread(strtab, strtab_shdr->sh_size, 1, file);

        Elf64_Rela *rela = malloc(rela_plt->sh_size);
        fseek(file, rela_plt->sh_offset, SEEK_SET);
        fread(rela, rela_plt->sh_size, 1, file);
        apply_relocations(base_addr, rela, rela_plt->sh_size / sizeof(Elf64_Rela), symtab, strtab);

        free(shdrs);
        free(symtab);
        free(strtab);
        free(rela);
        fclose(file);
        file=NULL;
        return 0;
}
*/
#endif