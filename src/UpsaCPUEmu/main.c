#include<stdint.h>
#include<stdbool.h>
#include"malloc.h"
#include"cpu.h"

#define ExtendedSize (1 << 14)

void init_memory_map(Cpu_t* cpu) {
	MemoryChunk_t* chunk = malloc(sizeof(MemoryChunk_t));
	chunk->dat  = malloc(ExtendedSize);
	chunk->len  = ExtendedSize;
	chunk->lett = 'E';

	cpu->memory_map.DeviceMemory[0] = *chunk;
	free(chunk);
}

// Number of arguments for each instruction
uint8_t OPCODE_ARG_COUNT[] = {
	0, 0,
	0, 0, 0, 0,
	0, 2, 2, 0,
	0, 0, 0, 0,
	0, 0, 0, 0,
	0, 2, 0, 0,
	0, 0, 2, 2,
	2, 2, 2, 2,
	2, 0, 0, 0,
	0, 0, 0, 0,
	0, 0, 0, 0,
	0, 0, 2, 0,
	0, 0, 0, 2,
	0, 0, 0, 0,
	0, 0, 0, 0,
	0, 0, 0, 0,
	2, 0, 0, 2,
	2, 2, 2, 0,
	2, 2, 2, 2,
	0, 0, 0, 0,
	0, 0, 0, 0,
	3, 3, 3, 3,
	3, 3, 3, 3,
	0, 0, 0, 0,
	0, 0, 0, 0,
	0, 0, 3, 3,
	3, 3
};

void flags(Cpu_t* cpu) {
	cpu->EQ = cpu->TEMP == 0;
	cpu->CY = cpu->TEMP > cpu->A; // if Temp is greater than A, then it MUST have overflowed (as its TEMP = A; A = A + B)
	cpu->PA = (cpu->TEMP % 2) == 0;
}

void add(Cpu_t* cpu, uint16_t SRC) {
	cpu->TEMP = cpu->A;
	cpu->A += SRC;
	flags(cpu);
}

void sub(Cpu_t* cpu, uint16_t SRC) {
	cpu->TEMP = cpu->A;
	cpu->A -= SRC;
	flags(cpu);
}

uint16_t find_chunk_min_addr(Cpu_t* cpu, int chunkIndex) {
	// Calculate the starting address for the given chunk index
	uint16_t baseAddress = ROM_SIZE + RAM_SIZE + IO_PORTS;
	for (int i = 0; i < chunkIndex; i++) {
		baseAddress += cpu->memory_map.DeviceMemory[i].len; // Accumulate the sizes of previous chunks
	}
	return baseAddress;
}

uint16_t find_chunk_max_addr(Cpu_t* cpu, int chunkIndex) {
	// Calculate the ending address for the given chunk index
	return find_chunk_min_addr(cpu, chunkIndex) + cpu->memory_map.DeviceMemory[chunkIndex].len - 1; // Inclusive
}

uint8_t memoryFetch(Cpu_t* cpu, uint32_t addr) {
	if (addr < ROM_SIZE) {
		return cpu->memory_map.Rom[addr]; // Fetch from ROM
	} else if (addr < (ROM_SIZE + RAM_SIZE)) {
		return cpu->memory_map.Ram[addr - ROM_SIZE]; // Fetch from RAM
	} else if (addr < (ROM_SIZE + RAM_SIZE + IO_PORTS)) {
		return cpu->memory_map.IoPorts[addr - (ROM_SIZE + RAM_SIZE)]; // Fetch from I/O ports
	} else {
		// Check for device memory
		for (int i = 0; i < 8; i++) {
			uint16_t minAddr = find_chunk_min_addr(cpu, i);
			uint16_t maxAddr = find_chunk_max_addr(cpu, i);
			if (addr >= minAddr && addr <= maxAddr) {
				return cpu->memory_map.DeviceMemory[i].dat[addr - minAddr]; // Fetch from device memory
			}
		}
	}
	return 0; // Return 0 if address is invalid
}

void memoryWrite(Cpu_t* cpu, uint32_t addr, uint8_t val) {
	if (addr < ROM_SIZE) {
		return; // ROM is read-only
	} else if (addr < (ROM_SIZE + RAM_SIZE)) {
		cpu->memory_map.Ram[addr - ROM_SIZE] = val; // Write to RAM
	} else if (addr < (ROM_SIZE + RAM_SIZE + IO_PORTS)) {
		cpu->memory_map.IoPorts[addr - (ROM_SIZE + RAM_SIZE)] = val; // Write to I/O ports
	} else {
		// Check for device memory
		for (int i = 0; i < 8; i++) {
			uint16_t minAddr = find_chunk_min_addr(cpu, i);
			uint16_t maxAddr = find_chunk_max_addr(cpu, i);
			if (addr >= minAddr && addr <= maxAddr) {
				cpu->memory_map.DeviceMemory[i].dat[addr - minAddr] = val; // Write to device memory
				return;
			}
		}
	}
}

void push16(Cpu_t* cpu, uint16_t v) {
	--cpu->SP;
	memoryWrite(cpu, cpu->SP, v >> 8);
	--cpu->SP;
	memoryWrite(cpu, cpu->SP, v & 255);
}

uint16_t pop16(Cpu_t* cpu) {
	uint16_t x=memoryFetch(cpu, cpu->SP++);
	x |= memoryFetch(cpu, cpu->SP++) << 8;
	return x;
}

void interrupt(Cpu_t* cpu, uint8_t irq) {
	if (cpu->IM == false) return;
	int H = memoryFetch(cpu, irq * 2+1);
	int L = memoryFetch(cpu, irq * 2);
	push16(cpu, cpu->PC);
	cpu->PC = (H << 8) | L;
}

void reset(Cpu_t* cpu) {
	cpu->PC = BASE;
}

void execute_next(Cpu_t* cpu) {
	// fetch
	cpu->CIR[0] = memoryFetch(cpu, cpu->PC++);
	for (int i = 1; i <= OPCODE_ARG_COUNT[cpu->CIR[0]]; ++i) {
		cpu->CIR[i] = memoryFetch(cpu, cpu->PC++);
	}
	
	cpu->M = memoryFetch(cpu, cpu->X);
	cpu->MY = memoryFetch(cpu, cpu->Y);
	
	switch (cpu->CIR[0]) {
		case NOP:
			break;
		case ADD_B:
			add(cpu, cpu->B);
			break;
		case ADD_M:
			add(cpu, cpu->M);
			break;
		case ADD_A:
			add(cpu, cpu->A);
			break;
		case SUB_B:
			sub(cpu, cpu->B);
			break;
		case SUB_M:
			sub(cpu, cpu->M);
			break;
		case ZER:
			sub(cpu, cpu->A);
			break;
		case MV_A_I:
			cpu->A = (cpu->CIR[2] << 8) | cpu->CIR[1];
			puts("MOV A, "); PRINT_DWORD((cpu->CIR[2] << 8) | cpu->CIR[1]);
			break;
		case MV_B_I:
			cpu->B = (cpu->CIR[2] << 8) | cpu->CIR[1];
			break;
		case MV_M_A:
			memoryWrite(cpu, cpu->X, cpu->A);
			break;
		case MV_A_M:
			cpu->A = cpu->M;
			break;
		case MV_A_B:
			cpu->A = cpu->B;
			break;
		case MV_B_A:
			cpu->B = cpu->A;
			break;
		case SH_B:
			cpu->TEMP = cpu->A;
			if (cpu->DI)
				cpu->A >>= cpu->B;
			else
				cpu->A <<= cpu->B;
			break;
		case SH_M:
			cpu->TEMP = cpu->A;
			if (cpu->DI)
				cpu->A >>= cpu->M;
			else
				cpu->A <<= cpu->M;
			break;
		case LDB:
			cpu->A = cpu->M;
			cpu->X++;
			break;
		case STB:
			cpu->M = cpu->A;
			memoryWrite(cpu, cpu->X, cpu->A);
			cpu->X++;
			break;
		case MV_X_A:
			cpu->X = cpu->A;
			break;
		case MV_A_X:
			cpu->A = cpu->X;
			break;
		case MV_X_I:
			cpu->X = (cpu->CIR[2] << 8) | cpu->CIR[1];
			break;
		case INC_A:
			add(cpu, 1);
			break;
		case DEC_A:
			sub(cpu, 1);
			break;
		case INC_B:
			{
			int A = cpu->A;
			cpu->TEMP = cpu->B;
			cpu->B += 1;
			cpu->A = cpu->B;
			flags(cpu);
			cpu->A = A;
			}
			break;
		case DEC_B:
			{
			int A = cpu->A;
			cpu->TEMP = cpu->B;
			cpu->B -= 1;
			cpu->A = cpu->B;
			flags(cpu);
			cpu->A = A;
			}
			break;
		case INC_X:
			{
			int A = cpu->A;
			cpu->TEMP = cpu->X;
			cpu->X += 1;
			cpu->A = cpu->X;
			flags(cpu);
			cpu->A = A;
			}
			break;
		case DEC_X:
			{
			int A = cpu->A;
			cpu->TEMP = cpu->X;
			cpu->X -= 1;
			cpu->A = cpu->X;
			flags(cpu);
			cpu->A = A;
			}
			break;
		case INC_SP:
			{
			int A = cpu->A;
			cpu->TEMP = cpu->SP;
			cpu->SP += 1;
			cpu->A = cpu->SP;
			flags(cpu);
			cpu->A = A;
			}
			break;
		case DEC_SP:
			{
			int A = cpu->A;
			cpu->TEMP = cpu->SP;
			cpu->SP -= 1;
			cpu->A = cpu->SP;
			flags(cpu);
			cpu->A = A;
			}
			break;
		case NAND:
			cpu->A = cpu->A & cpu->B;
			cpu->A = ~cpu->A;
			break;
		case JMP:
JUMP:
			cpu->PC = (cpu->CIR[2] << 8) | cpu->CIR[1];
			break;
		case JEQ:
			if (cpu->EQ)
				goto JUMP;
			break;
		case JNEQ:
			if (!cpu->EQ)
				goto JUMP;
			break;
		case JCY:
			if (cpu->CY)
				goto JUMP;
			break;
		case JNCY:
			if (!cpu->CY)
				goto JUMP;
			break;
		case JPA:
			if (cpu->PA)
				goto JUMP;
			break;
		case JNPA:
			if (!cpu->PA)
				goto JUMP;
			break;
		case DFO:
			cpu->DI = 0;
			break;
		case DBA:
			cpu->DI = 1;
			break;
		case PSHA:
			push16(cpu, cpu->A);
			break;
		case POPA:
			cpu->A = pop16(cpu);
			break;
		case PSHB:
			push16(cpu, cpu->B);
			break;
		case POPB:
			cpu->B = pop16(cpu);
			break;
		case PSHX:
			push16(cpu, cpu->X);
			break;
		case POPX:
			cpu->X = pop16(cpu);
			break;
		case MV_SP_A:
			cpu->SP = cpu->A;
			break;
		case MV_SP_X:
			cpu->SP = cpu->X;
			break;
		case MV_X_SP:
			cpu->X = cpu->SP;
			break;
		case JX:
			cpu->PC = cpu->X;
			break;
		case CALL:
			push16(cpu, cpu->PC);
			goto JUMP;
			break;
		case RET:
			cpu->PC = pop16(cpu);
			break;
		case CALLX:
			push16(cpu, cpu->PC);
			cpu->PC = cpu->X;
			break;

		case MV_Y_A:
			cpu->Y = cpu->A;
			break;
		case MV_A_Y:
			cpu->A = cpu->Y;
			break;
		case MV_Y_I:
			cpu->Y = (cpu->CIR[2] << 8) | cpu->CIR[1];
			break;
		case INC_Y:
			{
			int A = cpu->A;
			cpu->TEMP = cpu->Y;
			cpu->Y += 1;
			cpu->A = cpu->Y;
			flags(cpu);
			cpu->A = A;
			}
			break;
		case DEC_Y:
			{
			int A = cpu->A;
			cpu->TEMP = cpu->Y;
			cpu->Y -= 1;
			cpu->A = cpu->Y;
			flags(cpu);
			cpu->A = A;
			}
			break;
		case PSHY:
			push16(cpu, cpu->X);
			break;
		case POPY:
			cpu->Y = pop16(cpu);
			break;
		case PSHSP:
			push16(cpu, cpu->SP);
			break;
		case POPSP:
			cpu->SP = pop16(cpu);
			break;
		case LDYB:
			cpu->A = cpu->M;
			cpu->Y++;
			break;
		case STYB:
			cpu->M = cpu->A;
			memoryWrite(cpu, cpu->Y, cpu->A);
			cpu->Y++;
			break;

		case MV_A_MY:
			cpu->A = cpu->MY;
			break;
		case MV_MY_A:
			memoryWrite(cpu, cpu->Y, cpu->A);
			break;
		case MV_B_MY:
			cpu->B = cpu->MY;
			break;
		case MV_MY_B:
			memoryWrite(cpu, cpu->Y, cpu->B);
			break;
		case ADD_MY:
			add(cpu, cpu->MY);
			break;
		case SUB_MY:
			sub(cpu, cpu->MY);
			break;
		case _INT:
			{
			interrupt(cpu, cpu->CIR[1]);
			}
			break;
		case DI:
			cpu->IM = false;
			break;
		case EI:
			cpu->IM = true;
			break;
		case LOAD_A:
			cpu->A  = memoryFetch(cpu, (cpu->CIR[2] << 8) | cpu->CIR[1]);
			cpu->A |= memoryFetch(cpu, ((cpu->CIR[2] << 8) | cpu->CIR[1]) + 1) << 8;
			break;
		case LOAD_B:
			cpu->B  = memoryFetch(cpu, (cpu->CIR[2] << 8) | cpu->CIR[1]);
			cpu->B |= memoryFetch(cpu, ((cpu->CIR[2] << 8) | cpu->CIR[1]) + 1) << 8;
			break;
		case STORE_A:
			memoryWrite(cpu, (cpu->CIR[2] << 8) | cpu->CIR[1], cpu->A);
			memoryWrite(cpu, ((cpu->CIR[2] << 8) | cpu->CIR[1]) + 1, cpu->A >> 8);
			break;
		case STORE_B:
			memoryWrite(cpu, (cpu->CIR[2] << 8) | cpu->CIR[1], cpu->B);
			memoryWrite(cpu, ((cpu->CIR[2] << 8) | cpu->CIR[1]) + 1, cpu->B >> 8);
			break;
		case LOAD_X:
			{
				int L = memoryFetch(cpu, (cpu->CIR[2] << 8) | cpu->CIR[1]);
				int H = memoryFetch(cpu, ((cpu->CIR[2] << 8) | cpu->CIR[1]) + 1);
				cpu->X = (H << 8) | L;
			}
			break;
		case LOAD_Y:
			{
				int L = memoryFetch(cpu, (cpu->CIR[2] << 8) | cpu->CIR[1]);
				int H = memoryFetch(cpu, ((cpu->CIR[2] << 8) | cpu->CIR[1]) + 1);
				cpu->Y = (H << 8) | L;
			}
			break;
		case STORE_X:
			memoryWrite(cpu, (cpu->CIR[2] << 8) | cpu->CIR[1], cpu->X);
			memoryWrite(cpu, ((cpu->CIR[2] << 8) | cpu->CIR[1]) + 1, cpu->X >> 8);
			break;
		case STORE_Y:
			memoryWrite(cpu, (cpu->CIR[2] << 8) | cpu->CIR[1], cpu->Y);
			memoryWrite(cpu, ((cpu->CIR[2] << 8) | cpu->CIR[1]) + 1, cpu->Y >> 8);
			break;

		case LOAD_X_Y:
			{
				int L = memoryFetch(cpu, cpu->Y);
				int H = memoryFetch(cpu, cpu->Y + 1);
				cpu->X = (H << 8) | L;
			}
			break;
		case LOAD_Y_X:
			{
				int L = memoryFetch(cpu, cpu->X);
				int H = memoryFetch(cpu, cpu->X + 1);
				cpu->Y = (H << 8) | L;
			}
			break;
		case STORE_X_Y:
			memoryWrite(cpu, cpu->Y, cpu->X);
			memoryWrite(cpu, cpu->Y + 1, cpu->X >> 8);
			break;
		case STORE_Y_X:
			memoryWrite(cpu, cpu->X, cpu->Y);
			memoryWrite(cpu, cpu->X + 1, cpu->Y >> 8);
			break;

		case NOT:
			cpu->TEMP = cpu->A;
			cpu->A = !cpu->A;
			flags(cpu);
			break;
		case AND:
			cpu->TEMP = cpu->A;
			cpu->A = cpu->A & cpu->B;
			flags(cpu);
			break;
		case XOR:
			cpu->TEMP = cpu->A;
			cpu->A = cpu->A ^ cpu->B;
			flags(cpu);
			break;
		case OR:
			cpu->TEMP = cpu->A;
			cpu->A = cpu->A | cpu->B;
			flags(cpu);
			break;

		case STORE_A_FAR:
			memoryWrite(cpu, (cpu->CIR[3] << 16) | (cpu->CIR[2] << 8) | cpu->CIR[1], cpu->A);
			break;
		case STORE_B_FAR:
			memoryWrite(cpu, (cpu->CIR[3] << 16) | (cpu->CIR[2] << 8) | cpu->CIR[1], cpu->B);
			break;
		case STORE_X_FAR:
			memoryWrite(cpu, (cpu->CIR[3] << 16) | (cpu->CIR[2] << 8) | cpu->CIR[1], cpu->X);
			memoryWrite(cpu, ((cpu->CIR[3] << 16) | (cpu->CIR[2] << 8) | cpu->CIR[1]) + 1, cpu->X >> 8);
			break;
		case STORE_Y_FAR:
			memoryWrite(cpu, (cpu->CIR[3] << 16) | (cpu->CIR[2] << 8) | cpu->CIR[1], cpu->Y);
			memoryWrite(cpu, ((cpu->CIR[3] << 16) | (cpu->CIR[2] << 8) | cpu->CIR[1]) + 1, cpu->Y >> 8);
			break;
		case LOAD_A_FAR:
			cpu->A  = memoryFetch(cpu, (cpu->CIR[3] << 16)  | (cpu->CIR[2] << 8) | cpu->CIR[1]);
			cpu->A |= memoryFetch(cpu, ((cpu->CIR[3] << 16) | (cpu->CIR[2] << 8) | cpu->CIR[1]) + 1) << 8;
			break;
		case LOAD_B_FAR:
			cpu->B  = memoryFetch(cpu, (cpu->CIR[3] << 16)  | (cpu->CIR[2] << 8) | cpu->CIR[1]);
			cpu->B |= memoryFetch(cpu, ((cpu->CIR[3] << 16) | (cpu->CIR[2] << 8) | cpu->CIR[1]) + 1) << 8;
			break;
		case LOAD_X_FAR:
			cpu->X  = memoryFetch(cpu, (cpu->CIR[3] << 16)  | (cpu->CIR[2] << 8) | cpu->CIR[1]);
			cpu->X |= memoryFetch(cpu, ((cpu->CIR[3] << 16) | (cpu->CIR[2] << 8) | cpu->CIR[1]) + 1) << 8;
			break;
		case LOAD_Y_FAR:
			cpu->Y  = memoryFetch(cpu, (cpu->CIR[3] << 16)  | (cpu->CIR[2] << 8) | cpu->CIR[1]);
			cpu->Y |= memoryFetch(cpu, ((cpu->CIR[3] << 16) | (cpu->CIR[2] << 8) | cpu->CIR[1]) + 1) << 8;
			break;
		case MV_FAR_M_A:
			memoryWrite(cpu, (cpu->X << 16) | cpu->Y, cpu->A);
			memoryWrite(cpu, ((cpu->X << 16) | cpu->Y) + 1, cpu->A >> 8);
			break;
		case MV_FAR_A_M:
			cpu->A  = memoryFetch(cpu, (cpu->X << 16) | cpu->Y);
			cpu->A |= memoryFetch(cpu, ((cpu->X << 16) | cpu->Y) + 1) << 8;
			break;

		case MUL:
			cpu->TEMP = cpu->A;
			cpu->A *= cpu->B;
			flags(cpu);
			break;
		case DIV:
			cpu->TEMP = cpu->A;
			cpu->A /= cpu->B;
			flags(cpu);
			break;
		case LOAD_A_B:
			cpu->A  = memoryFetch(cpu, (cpu->CIR[2] << 8) | cpu->CIR[1]);
			break;
		case LOAD_B_B:
			cpu->B  = memoryFetch(cpu, (cpu->CIR[2] << 8) | cpu->CIR[1]);
			break;
		case STORE_A_B:
			memoryWrite(cpu, (cpu->CIR[2] << 8) | cpu->CIR[1], cpu->A);
			break;
		case STORE_B_B:
			memoryWrite(cpu, (cpu->CIR[2] << 8) | cpu->CIR[1], cpu->B);
			break;
		case HLT:
		default:
			cpu->HLT = 1;
			return;
	}
}
