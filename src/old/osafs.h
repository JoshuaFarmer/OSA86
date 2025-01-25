#pragma once

#include <stdint.h>
#include <stdbool.h>
#include "string.h"

#define FS_SPBLCK_SIZE                1024 // number of blocks in superblock (1024*512 == 512 KiB)
#define FS_BLOCK_SIZE                 512 // size of each block (bytes)
#define FS_MAX_BPF                         488 // maximum number of blocks a file can occupy (so that its 2 sectors pfile)
#define FS_MAX_FILES                         512
#define FS_MAX_PATH_DEPTH         64
#define FS_MAX_NAME_LENGTH         16
#define FS_START                                 256
#define FS_ROOT_DIR_IDX         -1

typedef uint16_t index_t;

index_t CURRENT_DIR_IDX = FS_ROOT_DIR_IDX;
char STR_PATH[FS_MAX_PATH_DEPTH*FS_MAX_NAME_LENGTH] = "";

uint8_t Drive_Letter = 'A';

typedef struct {
        char name[FS_MAX_NAME_LENGTH];
        uint32_t size;
        uint16_t modified;
        bool                exists;
        index_t        idx;
        bool                is_dir;
        index_t        parent_idx;

        uint16_t blocks[FS_MAX_BPF]; // indices of the blocks used by this file
        uint16_t block_count;
} FileEntry_t;

typedef struct {
        uint8_t data[FS_BLOCK_SIZE];
} FS_Block_t;

typedef struct {
        FS_Block_t  blocks[FS_SPBLCK_SIZE];
        FileEntry_t file_entries[FS_MAX_FILES]; // Assuming each file entry
        uint32_t    file_count;
} Super_Block_t;

typedef struct {
        const char FILE_TYPE[FS_MAX_NAME_LENGTH];
        const char TYPE_DESC[32];
} FileType_t;

FileType_t TextFileType = {.FILE_TYPE = "txt", .TYPE_DESC="Text Document"};
FileType_t LinkFileType = {.FILE_TYPE = "lnk", .TYPE_DESC="Link"};
FileType_t ProgFileType = {.FILE_TYPE = "prg", .TYPE_DESC="Program"};
FileType_t ExeFileType  = {.FILE_TYPE = "exe", .TYPE_DESC="Program"};

uint16_t encode_date(uint16_t day, uint16_t month, uint16_t year) {
        return (day & 0x1F) | ((month & 0x0F) << 5) | ((year & 0x7F) << 9);
}

void decode_date(uint16_t encoded_date, uint16_t *day, uint16_t *month, uint16_t *year) {
        *day = encoded_date & 0x1F;
        *month = (encoded_date >> 5) & 0x0F;
        *year = (encoded_date >> 9) & 0x7F;
}

void initialize_super_block(Super_Block_t *super_block) {
        for (int i = 0; i < FS_MAX_FILES; i++) {
                super_block->file_entries[i].exists = false;
        }
        for (int i = 0; i < FS_SPBLCK_SIZE; ++i) {
                memset(super_block->blocks[i].data, 0, FS_BLOCK_SIZE);
        }
        super_block->file_count = 0;
}

const FileEntry_t *find_file(const Super_Block_t *super_block, const char *name, index_t parent_idx) {
        for (index_t i = 0; i < super_block->file_count; i++) {
                if (super_block->file_entries[i].exists &&
                        strcmp(name, super_block->file_entries[i].name) == 0 &&
                        super_block->file_entries[i].parent_idx == parent_idx) {
                        return &super_block->file_entries[i];
                }
        }
        return NULL;
}

bool file_exists(const Super_Block_t *super_block, const char *name, index_t parent_idx) {
        const FileEntry_t* en = find_file(super_block, name, parent_idx);
        return (en != NULL);
}

void read_data(const Super_Block_t *super_block, const char *name, char *buffer, uint32_t buffer_size, index_t parent_idx) {
        const FileEntry_t *entry = find_file(super_block, name, parent_idx);
        if (entry == NULL || buffer_size < entry->size) {
                puts("Buffer size is too small or file not found.\n");
                return;
        }

        for (int i = 0; i < entry->block_count; i++) {
                int block_idx = entry->blocks[i];
                memcpy(buffer + i * FS_BLOCK_SIZE, super_block->blocks[block_idx].data, FS_BLOCK_SIZE);
        }
}

void create_file_entry(Super_Block_t *super_block, const char *name, uint32_t size, int day, int month, int year, bool is_dir, index_t parent_idx) {
        if (super_block->file_count >= FS_MAX_FILES) {
                puts("Super block is full, cannot add more files.\n");
                return;
        }

        for (int i = 0; i < FS_MAX_FILES; ++i) {
                if (!super_block->file_entries[i].exists) {
                        FileEntry_t *entry = &super_block->file_entries[i];
                        strncpy(entry->name, name, sizeof(entry->name) - 1);
                        entry->name[sizeof(entry->name) - 1] = '\0';
                        entry->size = size;
                        entry->modified = encode_date(day, month, year);
                        entry->block_count = 0;
                        entry->exists = true;
                        entry->is_dir = is_dir;
                        entry->parent_idx = parent_idx;
                        entry->idx = i; // Set index as the position in the array

                        ++super_block->file_count;
                        return;
                }
        }
}

void delete_file_entry(Super_Block_t *super_block, const char *name, index_t parent_idx) {
        FileEntry_t *file = (FileEntry_t *)find_file(super_block, name, parent_idx);
        if (file != NULL) {
                memset(file, 0, sizeof(FileEntry_t));
                file->exists = false;
                --super_block->file_count;
        }
}

bool FileHasBlock(Super_Block_t *super_block, int block_idx, int file_idx) {
        if (super_block->file_entries[file_idx].exists == false) return false;
        for (int i = 0; i < super_block->file_entries[file_idx].block_count; ++i) {
                if (super_block->file_entries[file_idx].blocks[i] == block_idx) {
                        return true;
                }
        }

        return false;
}

bool BlockIsUsed(Super_Block_t *super_block, int block_idx) {
        for (int i = 0; i < FS_MAX_FILES; ++i) {
                if (FileHasBlock(super_block, block_idx, i)) {
                        return true;
                }
        }

        return false;
}

void write_data(Super_Block_t *super_block, const char *name, const uint8_t *data, int data_size, index_t parent_idx) {
        FileEntry_t *entry = (FileEntry_t *)find_file(super_block, name, parent_idx);
        if (entry == NULL) {
                puts("File not found.\n");
                return;
        }

        if (data_size > FS_MAX_BPF * FS_BLOCK_SIZE) {
                puts("Data size exceeds maximum file size.\n");
                return;
        }

        int blocks_needed = (data_size + FS_BLOCK_SIZE - 1) / FS_BLOCK_SIZE;

        for (int i = 0, block_idx = 0; i < blocks_needed && block_idx < FS_SPBLCK_SIZE; block_idx++) {
                if (BlockIsUsed(super_block, block_idx) == 0) {
                        memcpy(super_block->blocks[block_idx].data, data + i * FS_BLOCK_SIZE, FS_BLOCK_SIZE);
                        entry->blocks[entry->block_count++] = block_idx;
                        i++;
                }
        }

        entry->size = data_size;
}

// Helper function to format the file size as a right-aligned string
void format_right_aligned(char *buffer, int value, int width) {
        int i = width - 1;
        buffer[width] = '\0'; // Null terminator

        // Fill the buffer from right to left with digits
        do {
                buffer[i--] = (value % 10) + '0';
                value /= 10;
        } while (value > 0 && i >= 0);

        // Fill remaining space with spaces
        while (i >= 0) {
                buffer[i--] = ' ';
        }
}

void print_file_entry(const FileEntry_t *entry, int indent_level) {
        // Indentation for nested directories
        for (int i = 0; i < indent_level; ++i) {
                putc(' ');
                putc(' ');
                putc(' ');
        }

        // Print file or directory name
        puts(entry->name);

        // If it's a directory, indicate this
        if (entry->is_dir) {
                puts("/");
        }
}

void print_directory_tree(const Super_Block_t *super_block, index_t dir_idx, int indent_level) {
        for (index_t i = 0; i < super_block->file_count; i++) {
                const FileEntry_t *entry = &super_block->file_entries[i];
                if (entry->exists && entry->parent_idx == dir_idx) {
                        print_file_entry(entry, indent_level);
                        if (entry->is_dir) {
                                putc('\n');
                                print_directory_tree(super_block, entry->idx, indent_level + 1);
                        }
                }
        }
        putc('\n');
}

void fs_list(const Super_Block_t *super_block) {
        int X = 0;
        for (index_t i = 0; i < super_block->file_count; i++) {
                if (super_block->file_entries[i].exists) {
                        if (super_block->file_entries[i].parent_idx == CURRENT_DIR_IDX) {
                                int tmp = termCol;
                                if (super_block->file_entries[i].is_dir) {
                                        termCol = termCol << 4;
                                }
                                
                                puts(super_block->file_entries[i].name);
                                termCol = tmp;

                                if (++X != 6) {
                                        puts("   ");
                                } else {
                                        X = 0;
                                        puts("\n");
                                }
                        }
                }
        }
        putc('\n');
}

void print_file_system(const Super_Block_t *super_block) {
        puts("-name-           -size-  -date-  -user-\n");
        for (index_t i = 0; i < super_block->file_count; i++) {
                if (super_block->file_entries[i].exists) {
                        if (super_block->file_entries[i].parent_idx == CURRENT_DIR_IDX) {
                                int tmp = termCol;
                                if (super_block->file_entries[i].is_dir == true)
                                { termCol = 0x05; }
                                puts(super_block->file_entries[i].name);
                                termCol = tmp;
                                putc('\n');
                        }
                }
        }

        puts("\nTotal Files: ");
        put_int(super_block->file_count);
}

Super_Block_t* super_block;

// Adjusted to handle dynamic path_list allocation
void create_file(const char* path) {
        char path_copy[FS_MAX_PATH_DEPTH * FS_MAX_NAME_LENGTH];
        strncpy(path_copy, path, sizeof(path_copy) - 1);
        path_copy[sizeof(path_copy) - 1] = '\0';

        char* path_list[FS_MAX_PATH_DEPTH];
        int idx = 0;

        char *token = strtok(path_copy, "/");
        while (token != NULL && idx < FS_MAX_PATH_DEPTH) {
                path_list[idx++] = token;
                token = strtok(NULL, "/");
        }

        if (idx == 0) return;

        index_t current_dir_idx = FS_ROOT_DIR_IDX;

        for (int i = 0; i < idx - 1; i++) {
                bool dir_found = false;
                for (index_t j = 0; j < super_block->file_count; j++) {
                        if (super_block->file_entries[j].exists &&
                                super_block->file_entries[j].is_dir &&
                                strcmp(path_list[i], super_block->file_entries[j].name) == 0 &&
                                super_block->file_entries[j].parent_idx == current_dir_idx) {
                                current_dir_idx = j;
                                dir_found = true;
                                break;
                        }
                }

                if (!dir_found) {
                        create_file_entry(super_block, path_list[i], 0, 0, 0, 0, true, current_dir_idx);
                        current_dir_idx = super_block->file_count - 1;
                }
        }

        const char *file_name = path_list[idx - 1];
        if (strcmp(file_name, " ") != 0)
                create_file_entry(super_block, file_name, 0, 0, 0, 0, false, current_dir_idx);
}

int find_index_from_path(const char* path) {
        char path_copy[FS_MAX_PATH_DEPTH * FS_MAX_NAME_LENGTH];
        strncpy(path_copy, path, sizeof(path_copy) - 1);
        path_copy[sizeof(path_copy) - 1] = '\0';

        char* path_list[FS_MAX_PATH_DEPTH];
        index_t idx = 0;

        char *token = strtok(path_copy, "/");
        while (token != NULL && idx < FS_MAX_PATH_DEPTH) {
                path_list[idx++] = token;
                token = strtok(NULL, "/");
        }

        if (idx == 0) return -1;

        index_t current_dir_idx = FS_ROOT_DIR_IDX;

        for (index_t i = 0; i < idx; i++) {
                bool entry_found = false;
                for (index_t j = 0; j < super_block->file_count; j++) {
                        if (super_block->file_entries[j].exists &&
                                strcmp(path_list[i], super_block->file_entries[j].name) == 0 &&
                                super_block->file_entries[j].parent_idx == current_dir_idx) {
                                current_dir_idx = j;
                                entry_found = true;
                                break;
                        }
                }

                if (!entry_found) {
                        return -1; // Path not found
                }
        }

        return current_dir_idx;
}

bool read_file(const char *path, char *buffer, uint32_t buffer_size) {
        index_t file_idx = find_index_from_path(path);
        
        if (file_idx == (index_t)-1) {
                puts("File not found.\n");
                return false;
        }

        const FileEntry_t *entry = &super_block->file_entries[file_idx];
        if (entry->size == 0) {
                return false;
        }

        if (entry->is_dir) {
                puts("Cannot read a directory.\n");
                return false;
        }

        if (buffer_size < entry->size) {
                puts("Buffer size is too small, buffer size:, entry size:\n");
                PRINT_WORD(buffer_size);
                PRINT_WORD(entry->size);
                return false;
        }

        read_data(super_block, entry->name, buffer, buffer_size, entry->parent_idx);
        return true;
}

bool write_file(const char *path, const void *data, int data_size) {
        index_t file_idx = find_index_from_path(path);
        if (file_idx == (index_t)-1) {
                puts("Write: File not found.\n");
                return false;
        }

        FileEntry_t *entry = &super_block->file_entries[file_idx];
        if (entry->is_dir) {
                puts("Cannot write to a directory.\n");
                return false;
        }

        write_data(super_block, entry->name, data, data_size, entry->parent_idx);
        return true;
}

bool create(const char *path, bool is_dir) {
        char path_copy[FS_MAX_PATH_DEPTH * FS_MAX_NAME_LENGTH];
        strncpy(path_copy, path, sizeof(path_copy) - 1);
        path_copy[sizeof(path_copy) - 1] = '\0';

        char *path_list[FS_MAX_PATH_DEPTH];
        index_t idx = 0;

        char *token = strtok(path_copy, "/");
        while (token != NULL && idx < FS_MAX_PATH_DEPTH) {
                path_list[idx++] = token;
                token = strtok(NULL, "/");
        }

        if (idx == 0) {return false;}

        index_t current_dir_idx = FS_ROOT_DIR_IDX;

        for (index_t i = 0; i < idx - 1; i++) {
                bool dir_found = false;
                for (index_t j = 0; j < super_block->file_count; j++) {
                        if (super_block->file_entries[j].exists &&
                                super_block->file_entries[j].is_dir &&
                                strcmp(path_list[i], super_block->file_entries[j].name) == 0 &&
                                super_block->file_entries[j].parent_idx == current_dir_idx) {
                                current_dir_idx = j;
                                dir_found = true;
                                break;
                        }
                }

                if (!dir_found) {
                        create_file_entry(super_block, path_list[i], 0, 0, 0, 0, true, current_dir_idx);
                        current_dir_idx = super_block->file_count - 1;
                }
        }

        const char *name = path_list[idx - 1];
        create_file_entry(super_block, name, 0, 0, 0, 0, is_dir, current_dir_idx);
        return true;
}

bool change_active_dir(const char *_path) {
        if (strcmp(_path, "/") == 0) {
                CURRENT_DIR_IDX = FS_ROOT_DIR_IDX;
                strncpy(STR_PATH, "/", FS_MAX_PATH_DEPTH*FS_MAX_NAME_LENGTH - 1);
                STR_PATH[FS_MAX_PATH_DEPTH*FS_MAX_NAME_LENGTH - 1] = '\0';
                return true;
        }

        char* path = strcat(STR_PATH, _path);
        if (path[0] == '/') path++;

        int index = find_index_from_path(path);
        if (index >= 0) {
                CURRENT_DIR_IDX = index;
                strcpy(STR_PATH, path); // Update STR_PATH
                STR_PATH[strlen(STR_PATH)] = '/';
                free(path);
                return true;
        } else {
                puts("Directory not found.\n");
                free(path);
                return false;
        }
}

bool exists(const char *path) {
        index_t idx = find_index_from_path(path);
        if (idx != (index_t)-1) { 
                return super_block->file_entries[idx].exists;
        } else return false;
}

uint32_t file_length(const char* path) {
        index_t idx = find_index_from_path(path);
        if (idx == (index_t)-1) {
                puts("File not found.\n");
                return -1;
        }

        FileEntry_t *entry = &super_block->file_entries[idx];
        if (entry->is_dir) {
                puts("Cannot read a directory.\n");
                return -1;
        }

        return entry->size;
}

#include "program.h"

void print_properties(const char* path) {
        index_t index = find_index_from_path(path);
        puts(super_block->file_entries[index].name); putc('\n');
        puts("SIZE : "); PRINT_DWORD(super_block->file_entries[index].size);
        puts("DATE : "); PRINT_DWORD(super_block->file_entries[index].modified);
        puts("EXIST : "); puts(super_block->file_entries[index].exists ? "TRUE\n" : "FALSE\n");
        puts("INDEX : "); PRINT_DWORD(index);
        puts("DIR? : "); puts(super_block->file_entries[index].is_dir ? "TRUE\n" : "FALSE\n");
        puts("PARI : "); PRINT_DWORD(super_block->file_entries[index].parent_idx);
        puts("BLCKCN : "); PRINT_DWORD(super_block->file_entries[index].block_count);
        puts("BLCK0 : "); PRINT_DWORD(super_block->file_entries[index].blocks[0]);
        puts("BLCK0 > "); puts((const char*)super_block->blocks[super_block->file_entries[index].blocks[0]].data); putc('\n');
}

void print_properties_idx(index_t index) {
        puts(super_block->file_entries[index].name); putc('\n');
        puts("SIZE : "); PRINT_DWORD(super_block->file_entries[index].size);
        puts("DATE : "); PRINT_DWORD(super_block->file_entries[index].modified);
        puts("EXIST : "); puts(super_block->file_entries[index].exists ? "TRUE\n" : "FALSE\n");
        puts("INDEX : "); PRINT_DWORD(index);
        puts("DIR? : "); puts(super_block->file_entries[index].is_dir ? "TRUE\n" : "FALSE\n");
        puts("PARI : "); PRINT_DWORD(super_block->file_entries[index].parent_idx);
        puts("BLCKCN : "); PRINT_DWORD(super_block->file_entries[index].block_count);
        puts("BLCK0 : "); PRINT_DWORD(super_block->file_entries[index].blocks[0]);
        puts("BLCK0 > "); puts((const char*)super_block->blocks[super_block->file_entries[index].blocks[0]].data); putc('\n');
}

#include "program.h"

uint32_t exec_file(const char* path) {
        if (exists(path) == false) return 0;

        const char *ext = strrchr(path, '.'); // Find the last period in the path
        if (ext == NULL) {
                return 1; // No period found, unknown type
        }
        
        ext++; // Move past the period
        if (strcmp(ext, TextFileType.FILE_TYPE) == 0) {
                int len = file_length(path);
                char* data = malloc(len);

                read_file(path, data, len);
                putsn((const char*)data, len);
                free(data);
                putc('\n');
                return 2;
        } else if (strcmp(ext, LinkFileType.FILE_TYPE) == 0) {
                int len = file_length(path);
                char *buff = malloc(len);
                read_file(path, buff, len);
                exec_file(buff);
                free(buff);
                return 3;
        } else if (strcmp(ext, ProgFileType.FILE_TYPE) == 0) { // program
                int len = file_length(path);
                char *buff = malloc(len);
                read_file(path, buff, len);
                BrainFuck(buff);
                free(buff);
                return 4;
        } else if (strcmp(ext, ExeFileType.FILE_TYPE) == 0) { // program
                int len = file_length(path);
                char *buff = malloc(len);
                read_file(path, buff, len);

                bool valid = parse_upsa_header((ExeHeader_t*)buff, len);
                if (valid) {
                        add_program_to_list(buff, len);
                }

                free(buff);
                return 5;
        }

        return 0; // Default to unknown type
}

void load_file_system() {
        initialize_super_block(super_block);
        
        uint32_t sector = FS_START;
        uint8_t *work = malloc(1024);

        memset(work, 0, 1024);
        ata_lba_read(sector++, 1, work);
        super_block->file_count = work[0];

        for (index_t i = 0; i < super_block->file_count; i++) {
                memset(work, 0, 1024);
                ata_lba_read(sector, 2, work);
                memcpy(&super_block->file_entries[i], work, sizeof(FileEntry_t));
                sector += 2;
        }

        int size = FS_BLOCK_SIZE / 512;
        for (index_t i = 0; i < FS_SPBLCK_SIZE; i++) {
                memset(work, 0, 1024);
                ata_lba_read(sector, size, work);
                memcpy(&super_block->blocks[i], work, FS_BLOCK_SIZE);
                sector += size;
        }

        free(work);
}

void write_file_system() {
        uint32_t sector = FS_START;
        uint8_t *work = malloc(1024);

        memset(work, 0, 1024);
        work[0] = super_block->file_count;
        ata_lba_write(sector++, 1, work);

        for (index_t i = 0; i < FS_MAX_FILES; i++) {
                if (super_block->file_entries[i].exists) {
                        memset(work, 0, 1024);
                        memcpy(work, &super_block->file_entries[i], sizeof(FileEntry_t));
                        ata_lba_write(sector, 2, work);
                }
                sector += 2;
        }

        int size = FS_BLOCK_SIZE / 512;
        for (index_t i = 0; i < FS_SPBLCK_SIZE; i++) {
                memset(work, 0, 1024);
                memcpy(work, &super_block->blocks[i].data, FS_BLOCK_SIZE);
                ata_lba_write(sector, size, work);
                sector += size;
        }

        free(work);
}

void switch_drive(uint8_t drive) {
        write_file_system();

        switch (drive) {
                case 0x00:
                        ata_select_drive(ATA_PRIMARY_BASE, ATA_MASTER);
                        break;

                case 0x01:
                        ata_select_drive(ATA_PRIMARY_BASE, ATA_SLAVE);
                        break;

                default:
                        puts("Invalid Drive\n");
                        return;
                        break;
        }

        load_file_system();
        Drive_Letter = 'A' + drive;
}
