#pragma once
#include "UpsaCPUEmu/cpu.h"

#define MEMORY_SIZE 4096
#define isdigit(c) (c >= '0' && c <= '9')

// for simple programs
void BrainFuck(const char *code) {
	char memory[MEMORY_SIZE] = {0};
	int ptr = 0;
	int pc = 0;

	while (code[pc]) {
		// Ensure ptr is within the bounds of memory
		if (ptr < 0) ptr = MEMORY_SIZE - 1;
		if (ptr >= MEMORY_SIZE) ptr = 0;

		switch (code[pc]) {
			case '=': {
				// Dynamic parsing of numbers
				char numberBuffer[16];
				uint32_t numIndex = 0;
				pc++; // Move past '='
				
				// Read digits until a non-digit character or end of code
				while (isdigit(code[pc]) && numIndex < sizeof(numberBuffer) - 1) {
					numberBuffer[numIndex++] = code[pc++];
				}
				numberBuffer[numIndex] = '\0'; // Null-terminate the string
				memory[ptr] = atoi(numberBuffer); // Convert to integer and store
				pc--; // Adjust pc for the outer loop increment
				break;
			}
			case 'j': {
				char jumpBuffer[16];
				uint32_t jumpIndex = 0;
				pc++; // Move past 'j'
				
				// Read digits for the jump command
				while (isdigit(code[pc]) && jumpIndex < sizeof(jumpBuffer) - 1) {
					jumpBuffer[jumpIndex++] = code[pc++];
				}
				jumpBuffer[jumpIndex] = '\0'; // Null-terminate the string
				pc = atoi(jumpBuffer); // Set pc to the jump destination
				pc--; // Adjust pc for the outer loop increment
				break;
			}
			case '>':
				++ptr;
				break;
			case '<':
				--ptr;
				break;
			case '+':
				++(memory[ptr]);
				break;
			case '-':
				--(memory[ptr]);
				break;
			case '.':
				putc(memory[ptr]);
				break;
			case ',':
				memory[ptr] = getch(); // Use getchar for input
				break;
			case '[':
				if (memory[ptr] == 0) {
					int brackets = 1;
					while (brackets) {
						pc++;
						if (code[pc] == '[') ++brackets;
						else if (code[pc] == ']') --brackets;
					}
				}
				break;
			case ']':
				if (memory[ptr] != 0) {
					int brackets = 1;
					while (brackets) {
						--pc;
						if (code[pc] == '[') --brackets;
						else if (code[pc] == ']') ++brackets;
					}
				}
				break;
		}
		++pc; // Move to the next instruction
	}
}

// for more complex programs, use an UPSA emulator (Custom CPU)

#define MIN_SUPPORTED_VERSION 0x00
#define MAX_SUPPORTED_VERSION 0x01

const char _MAGIC[] = "86U";

typedef struct {
	uint8_t magic[3];
	uint8_t version;
} ExeHeader_t;

typedef struct _ProgramS {
	Cpu_t*            cpu;
	struct _ProgramS* nxt;
} Program_t;

Program_t root = {NULL, NULL};

bool parse_upsa_header(ExeHeader_t* header, size_t fileLen) {
	if (fileLen < sizeof(ExeHeader_t))     return false;
	if (strncmp((char*)header->magic, _MAGIC, 3)) return false;
	if (header->version >= MIN_SUPPORTED_VERSION && header->version <= MAX_SUPPORTED_VERSION) {
		return true;
	}

	return false;
}

Program_t* add_program_to_list(char* data, size_t fileLen) {
	Program_t* linked = malloc(sizeof(Program_t));
	linked->nxt = root.nxt;
	root.nxt = linked;

	Cpu_t* cpu = malloc(sizeof(Cpu_t)); // emulator
	linked->cpu = cpu;
	init_memory_map(cpu);

	for (size_t i = sizeof(ExeHeader_t); i < fileLen; ++i) {
		cpu->memory_map.Rom[BASE+(i-sizeof(ExeHeader_t))] = data[i];
	}

	return linked;
}

void remove_program(Program_t* prog) {
	free(prog->cpu->memory_map.DeviceMemory[0].dat);
	free(prog->cpu);

	Program_t* linked = &root;
	while (linked->nxt != NULL && linked->nxt != prog) {
		linked = linked->nxt;
	}

	if (linked) {
		linked->nxt = prog->nxt;
	}

	free(prog);
}
