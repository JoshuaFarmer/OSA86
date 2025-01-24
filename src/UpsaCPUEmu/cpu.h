#pragma once
#define RAM_SIZE 8192
#define ROM_SIZE 8192
#define IO_PORTS 1024
#define BASE     4096

typedef struct {
	uint8_t* dat;
	uint32_t len;
	char     lett;
} MemoryChunk_t;

typedef struct {
	uint8_t Rom[ROM_SIZE];
	uint8_t Ram[RAM_SIZE];
	uint8_t IoPorts[IO_PORTS];
	MemoryChunk_t DeviceMemory[8];
} Memory_Map;

typedef struct {
	uint16_t A:16;
	uint16_t B:16;
	uint16_t X:16;
	uint16_t Y:16;
	
	uint16_t M:16;
	uint16_t MY:16;

	uint16_t PC:16, SP:14;

	uint8_t EQ:1,CY:1,PA:1,DI:1,HLT:1,IM:1; // Zero, Carry, Parity, Direction, Halted, Interrupt Mask

	uint8_t  CIR[4];
	uint16_t TEMP;

	Memory_Map memory_map;
} Cpu_t;

typedef enum {
	HLT,
	ADD_B,
	ADD_M,
	ADD_A,
	SUB_B,
	SUB_M,
	ZER,
	MV_A_I,
	MV_B_I,
	MV_M_A,
	MV_A_M,
	MV_A_B,
	MV_B_A,
	SH_B,
	SH_M,
	LDB,
	STB,
	MV_X_A,
	MV_A_X, 
	MV_X_I,
	INC_A,
	INC_B,
	INC_X,
	NAND,
	JMP,
	JEQ,
	JNEQ,
	JCY,
	JNCY,
	JPA,
	JNPA,
	JX,
	DFO,
	DBA,
	PSHA,
	POPA,
	PSHB,
	POPB,
	PSHX,
	POPX,
	MV_SP_A,
	MV_SP_X,
	MV_X_SP,
	NOP,
	CALL,
	RET,
	CALLX,
	
	MV_Y_A,
	MV_A_Y,
	MV_Y_I,
	INC_Y,
	PSHY,
	POPY,
	PSHSP,
	POPSP,
	LDYB,
	STYB,
	
	MV_A_MY,
	MV_MY_A,
	MV_B_MY,
	MV_MY_B,
	ADD_MY,
	SUB_MY,
	_INT,
	DI,
	EI,
	LOAD_A,
	STORE_A,
	LOAD_B,
	STORE_B,
	INC_SP,
	LOAD_X,
	LOAD_Y,
	STORE_X,
	STORE_Y,
	LOAD_X_Y,
	LOAD_Y_X,
	STORE_X_Y,
	STORE_Y_X,
	NOT,
	AND,
	XOR,
	OR,
	STORE_A_FAR,
	STORE_B_FAR,
	STORE_X_FAR,
	STORE_Y_FAR,
	LOAD_A_FAR,
	LOAD_B_FAR,
	LOAD_X_FAR,
	LOAD_Y_FAR,
	MV_FAR_M_A,
	// X << 16 | Y
	MV_FAR_A_M,
	DEC_A,
	DEC_B,
	DEC_X,
	DEC_Y,
	DEC_SP,
	MUL,
	DIV,

	// BYTE only
	STORE_A_B,
	STORE_B_B,
	LOAD_A_B,
	LOAD_B_B,
} inst;

extern uint8_t OPCODE_ARG_COUNT[];

void init_memory_map(Cpu_t* cpu);
void flags(Cpu_t* cpu);
void add(Cpu_t* cpu, uint16_t);
void sub(Cpu_t* cpu, uint16_t);
uint16_t find_chunk_min_addr(Cpu_t* cpu, int chunkIndex);
uint16_t find_chunk_max_addr(Cpu_t* cpu, int chunkIndex);
uint8_t memoryFetch(Cpu_t* cpu, uint32_t addr);
void memoryWrite(Cpu_t* cpu, uint32_t addr, uint8_t val);
void push16(Cpu_t* cpu, uint16_t v);
uint16_t pop16(Cpu_t* cpu);
void interrupt(Cpu_t* cpu, uint8_t irq);
void reset(Cpu_t* cpu);
void execute_next(Cpu_t* cpu);
