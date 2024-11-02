#pragma once

#define MEMORY_SIZE 4096
#define isdigit(c) (c >= '0' && c <= '9')

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

const char _MAGIC[] = "86";

typedef struct {
	uint8_t magic[3];
	uint8_t version;
	uint32_t entryPoint; // offset
} ExeHeader_t;
