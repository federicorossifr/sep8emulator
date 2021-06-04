#include <cstdint>
#include "util/Logger.h"
typedef uint8_t mem_row_t;
typedef uint16_t  io_addr_t;
typedef uint8_t	int_type_t;
typedef uint32_t  addr_t;   //MEMORY ADDRESSES ARE OF 24 BITS ANYWAY
#define MEM_SIZE 		0x1000000	//16M

#define SYS_START 		0x000000
#define	SYS_END   		0x1FFFFF
#define USR_START 		0x200000
#define USR_END	  		0xFEFFFF
#define BIOS_START		0xFF0000
#define BIOS_END		0xFFFFFF
#define VGA_START 		0x0A0000
#define VGA_END			0x0AFFFF
#define VGA_DIM			0x010000
#define	IO_DEVICES 16

#define INVALID_MEM_ACCESS 4
#define INVALID_PRI_INSTR  5
typedef enum 
{
	F0,F1,F2,F3,F4,F5,F6,F7
} opcode_t;
typedef enum 
{
	CF,ZF,SF,OF,IF,US
} flag_t;

typedef enum 
{
	ADD,SUB,AND,OR,CMP,NOT,DHL,SHR,SHL
} alu_op;

class Memory { //Memory proxy smtarptr
	mem_row_t _ptr[MEM_SIZE];
public:
	mem_row_t& operator[](addr_t x) {return _ptr[x%(MEM_SIZE)];}
    mem_row_t* begin() {return &_ptr[0];}
   	mem_row_t* end() {return &_ptr[MEM_SIZE-1];}
};