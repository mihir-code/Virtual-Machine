#include <stdio.h>
#include <stdint.h>
/* windows only */
#include <Windows.h>
#include <conio.h>  // _kbhit



/*
    Bits (11-9): identify a destination register
    Bits (8-6): used for source registers
    Bits (5-0): bootleg/extra

*/

// trap code is a very common repeatable process, this is system specific, opcode is general purpose
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
enum{ 
    FL_POS = 1 << 0, //Postive check//
    FL_ZRO = 1 << 1, // Zero Check //
    FL_NEG = 1 << 2, // Negative Check //
};

unint16_t sign_extend(unint16_t x, int bit_count){
    if((x>>(bit_count-1))&1){
        x |=(0xFFFF << bit_count);
    }
    return x;
}
enum{
    TRAP_GETC = 0x20,
    TRAP_OUT = 0x21,
    TRAP_PUTS = 0x22, 
    TRAP_IN = 0x23,
    TRAP_PUTSP = 0x24,
    TRAP_HALT = 0x25,

};

enum{
    MR_KBSR = 0xFE00, // keyboard status, register, whether a key is pressed
    MR_KBDR = 0xFE02 // keyboard data, register, where a key is pressed
};
int main(int argc, const char* argv[]){
    if (argc < 2){
        /*show usage string */
        printf("lc3 [image-file1]... \n");
        exit(2);
    }
    for (int j = 1; j < argc; ++j){
        if(!read_image(argv[j])){
            printf("failed to load image: %s\n", argv[j]);
            exit(1);
        }
    }
    @{Load Arguments}    
    signal(SIGINT, handle_interrupt);
    disable_input_buffering();
    @{Setup}
    /* setting the Z flag since it's positive */
    reg[R_COND] = FL_ZRO;

    /* set the PC register to starting position - PC holds the memory address of the next executable instruction */
    /* 0x300 is default*/

    enum { PC_START =  0x3000};
    reg[R_PC] = PC_START;

    int running = 1;
    while(running)
    {
        /* Fetch the data*/
        uint16_t instr = mem_read(reg[R_PC]++);
        uint16_t op = instr >> 12;
        
        switch(op){
            case OP_ADD:
                {
                    /* destination register (DR) */
                    uint16_t r0 = (instr >> 9) & 0x7;
                    /* first operand (SR1) */
                    uint16_t r1 = (instr >> 6) & 0x7;
                    /* whether we are in immediate mode */
                    uint16_t imm_flag = (instr >> 5) & 0x1;

                    if (imm_flag)
                    {
                        uint16_t imm5 = sign_extend(instr & 0x1F, 5);
                        reg[r0] = reg[r1] + imm5;
                    }
                    else
                    {
                        uint16_t r2 = instr & 0x7;
                        reg[r0] = reg[r1] + reg[r2];
                    }

                    update_flags(r0);
                }
                break;
            case OP_AND:
                {

                /* destination register (DR) */
                uint16_t r0 = (instr >> 9) & 0x7;
                /* first operand (SR1) */
                uint16_t r1 = (instr >> 6) & 0x7;
                /* whether we are in immediate mode */
                uint16_t imm_flag = (instr >> 5) & 0x1;

                if (imm_flag)
                {
                    uint16_t imm5 = sign_extend(instr & 0x1F, 5);
                    reg[r0] = reg[r1] + imm5;
                }
                else
                {
                    uint16_t r2 = instr & 0x7;
                    reg[r0] = reg[r1] + reg[r2];
                }

                    update_flags(r0);
                }
                break;
            case OP_NOT:
                {
                uint16_t r0 = (instr >> 9) & 0x7;
                uint16_t r1 = (instr >> 6) & 0x7;
                
                reg[r0] = ~ reg[r1];
                update_flags(r0);
                }
                break;
            case OP_BR:
                {
                    uint16_t pc_offset = sign_extend(instr & 0x1FF, 9);
                    uint16_t cond_flag = (instr << 9) & 0x7;
                    if(cond_flag & reg[R_COND]){
                        reg[R_PC] += pc_offset;
                    }
                }
                break;
            case OP_JMP:
                { /* "jump" away the ownership, transfer the execution flow by transfering control to a different address in memory*/
                    uint16_t r1 = (instr >> 6) & 0x7;
                    reg[R_PC] = reg[r1];
                }
                break;
            case OP_JSR:
                { /*We are calling subroutines, similar to code for a jump*/
                    uint16_t long_flag = (instr >> 11) & 1;
                    reg[R_R7] = reg[R_PC];
                    if(long_flag){
                        uint16_t long_pc_offset = sign_extend(instr & 0x7FF, 11);
                        reg[R_PC] += long_pc_offset;
                    }
                    else{
                        uint16_t r1 = (instr >> 6) & 0x7;
                        reg[R_PC] = reg[r1];
                    }
                }
                break;
            case OP_LD:
                {
                    uint16_t r0 = (instr >> 9) & 0x7;
                    uint16_t pc_offsett = sign_extend(instr & 0x1FF, 9);
                    reg[r0] = mem_read(reg[R_PC] + pc_offset); // reads from program counter
                    update_flags(r0);
                }
                break;
            case OP_LDI:
                {
                /* destination register (DR) */
                uint16_t r0 = (instr >>  9) & 0x7;
                /* PC offset 9*/
                uint16_t pc_offset = sign_extend(instr & 0x1FF, 9);
                /*adds offset, look at location to get memory address*/
                reg[r0] = mem_read(mem_read(reg[R_PC]+pc_offset));
                update_flags(r0);
                }
                break;
            case OP_LDR:
                { /*this uses a base register*/
                    uint16_t r0 = (instr >> 9) & 0x7;
                    uint16_t r1 = (instr >> 6) & 0x7;
                    uint16_t offset = sign_extend(inst & 0x3F, 6);
                    reg[r0] = mem_read(reg[r1] + offset); // reads from register
                    update_flags(r0);
                }
                break;
            case OP_LEA:
                {
                    uint16_t r0 = (instr >> 9) & 0x7;
                    uint16_t pc_offset = sign_extend(instr & 0x1FF, 9);
                    reg[r0] = reg[R_PC] + pc_offset;
                    update_flags(r0);
                }
                break;
            case OP_ST:
                {
                    uint16_t r0 = (instr >> 9) & 0x7;
                    uint16_t pc_offset = sign_extend(instr & 0x1FF, 9);
                    mem_write(reg[R_PC] + pc_offset, reg[r0]);
                }
                break;
            case OP_STI:
                {
                    uint16_t r0 = (instr >> 9) & 0x7;
                    uint16_t pc_offset = sign_extend(instr & 0x1FF, 9);
                    mem_write(mem_read(reg[R_PC] + pc_offset), reg[r0]);
                }
                break;
            case OP_STR:
                {
                    uint16_t r0 = (instr >> 9) & 0x7; // destination
                    uint16_t r1 = (instr >> 6) & 0x7; // source
                    uint16_t offset = sign_extend(instr & 0x3F, 6);
                    mem_write(reg[r1] + offset, reg[r0]);
                }
                break;
            case OP_TRAP:
                {
                    reg[R_R7] = reg[R_PC];
                    switch (instr & 0xFF){
                        case TRAP_GETC:
                            { // reads ASCI
                                reg[R_RO] = (uint16_t) getchar();
                                update_flags(R_RO);
                            }
                            break;
                        case TRAP_OUT:
                            {
                                putc((char)reg[R_RO], stdout);
                                fflush(stdout);
                            }
                            break;
                        case TRAP_PUTS:
                            {   // reads one character
                                uint16_t* c = memory + reg[R_RO]; // *c is a pointer that points to the c variable
                                while (*c){
                                    putc((char) *c, stdout); // puts the c in stdout -> system.out.print()
                                    ++c;
                                }
                                fflush(stdout); // sends it to stdout.                            
                            }
                            break;
                        case TRAP_IN:
                            { // enter a character
                                printf("Enter a character: ");
                                char c = getchar();
                                putc(c, stdout);
                                fflush(stdout);
                                reg[R_RO] = (uint16_t)c;
                                update_flags(R_RO);
                            }
                            break;
                        case TRAP_PUTSP:
                            { // one char per byte
                                uint16_t* c = memory + reg[R_RO];
                                while (*c){ // while the pointer is active
                                    char char1 = (*c) & 0xFF;
                                    putc(char1, stdout);
                                    char char2 = (*x) >> 8;
                                    if (char2) putc(char2, stdout);
                                    ++c;
                                }
                                fflush(stdout);
                                
                            }
                            break;
                        case TRAP_HALT:
                            {
                                puts("HALT");
                                fflush(stdout);
                                running = 0;
                            }
                            break;
                    }    
                }
                break;
            case OP_RES:
            {
                abort();
            }
            case OP_RTI:
            {
                abort();
            }
            default:
                @{BAD OPCODE}
                break;
        }
    }
    @{Shutdown}
}

#define MEMORY_MAX(1 << 16)
uint16_t memory[MEMORY_MAX]; /* There are around 65536, think binary*/
uint16_t reg[R_COUNT];

void read_image_file(FILE* file){
    /* we care about the first 16 bits, tells us where in memory the program should start*/
    uint16_t origin; // memory address for start
    fread(&origin, sizeof(origin), 1, file);
    origin = swap16(origin);

    /* we know the maximum file size so we only need one fread */
    uint16_t max_read = MEMORY_MAX - origin;
    uint16_t* p = memory + origin;
    size_t read = fread(p, sizeof(uint16_t), max_read, file);

    /* swap to little endian */
    // little endian means the least significant byte is stored first
    // big endian has the most important byte stored first
    // endian is byte order 
    while (read-- > 0)
    {
        *p = swap16(*p);
        ++p;
    }
}
void update_flags(uint16_t r){
    if(reg[r] == 0)
    {
        reg[R_COND] = FL_ZRO;
    }
    else if (reg[r] >> 15) /* a 1 in the left-most bit indicates negative */
    {
        reg[R_COND] = FL_NEG;
    }
    else{
        reg[R_COND] = FL_POS;
    }
}
{

    /* destination register (DR) */
    uint16_t r0 = (instr >> 9) & 0x7;
    /* first operand (SR1) */
    uint16_t r1 = (instr >> 6) & 0x7;
    /* whether we are in immediate mode */
    uint16_t imm_flag = (instr >> 5) & 0x1;

    if (imm_flag)
    {
        uint16_t imm5 = sign_extend(instr & 0x1F, 5);
        reg[r0] = reg[r1] + imm5;
    }
    else
    {
        uint16_t r2 = instr & 0x7;
        reg[r0] = reg[r1] + reg[r2];
    }

    update_flags(r0);
}

/* this is the and operator, not the add operator*/
{
    uint16_t r0 = (instr >> 9) & 0x7;
    uint16_t r1 = (instr >> 6) & 0x7;

    uint16_t imm_flag = (instr >> 5) & 0x1;

    if(imm_flag){
        uint16_t imm5 = sign_extend(instr & 0x1F, 5);
        reg[r0] = reg[r1] & imm5;
    }
    else{
        uint16_t r2 = instr & 0x7;
        reg[r0] = reg[r1] & reg[r2];
    }
    update_flags(r0);
}

 // to follow lil endian format, we need to shift the first 8 bytes left and then shift 
uint16_t swap(uint16_t x){
    return (x << 8) | (x >> 8);
    // shift the original x left 8 bits
    // shift original x right 8 bits
    // combine with the | operator
}

int read_image(const char* image_path){
    FILE* file = fopen(image_path, "rb"); // "rb" says that the file will be read
    if(!file) {return 0;}; // makes sure the program doesnt crash due to a pointer dereferncing
    read_image_file(file);
    fclose(file);
    return 1;
}


