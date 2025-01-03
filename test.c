#include <stdio.h>

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


int main(int argc, const char* argv[]){
    printf("hello");
    return 0;

}

#define MEMORY_MAX(1 << 16)
uint16_t memory[MEMORY_MAX] 