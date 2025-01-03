#include <stdio.h>
#include <stdint.h>
/* windows only */
#include <Windows.h>
#include <conio.h>  // _kbhit

enum
{
    R_R0 = 0,
    R_R1,
    R_R2,
    R_R3,
    R_R4,
    R_R5,
    R_R6,
    R_R7,
    R_PC, /* program counter */
    R_COND,
    R_COUNT
};

enum{
    OP_BR = 0,
    OP_ADD,
    OP_LD,
    OP_ST,
    OP_JSR,
    OP_AND,
    OP_LDR,
    OP_STR,
    OP_RTI,
    OP_NOT,
    OP_LDI,
    OP_STI,
    OP_JMP,
    OP_RES,
    OP_LEA,
    OP_TRP
};


int main(int argc, const char* argv[]){
    printf("hello");
    return 0;

}

#define MEMORY_MAX(1 << 16)
uint16_t memory[MEMORY_MAX]; /* There are around 65536, think binary*/
