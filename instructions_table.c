#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>

#include "utilities.h"
#include "memory_map.h"
#include "errors.h"

#define INSTRUCTION_TABLE_SIZE (sizeof(instruction_table) / sizeof(instruction_descriptor))

typedef struct {
    unsigned int immediate:1;
    unsigned int direct:1;
    unsigned int relative:1;
    unsigned int register_direct:1;
} supported_addressing_methods;

typedef struct {
    const char *name;
    unsigned short opcode;
    unsigned short funct;
    supported_addressing_methods src_op_addressing;
    supported_addressing_methods dst_op_addressing;
} instruction_descriptor;

/* all instructions are defined here with the opcode, func and supported addressing methods */
instruction_descriptor instruction_table[] = {{"mov", 0, 0, {1, 1, 0, 1}, {0, 1, 0, 1}},
                                              {"cmp", 1, 0, {1, 1, 0, 1}, {1, 1, 0, 1}},
                                              {"add", 2, 1, {1, 1, 0, 1}, {0, 1, 0, 1}},
                                              {"sub", 2, 2, {1, 1, 0, 1}, {0, 1, 0, 1}},
                                              {"lea", 4, 0, {0, 1, 0, 0}, {0, 1, 0, 1}},
                                              {"clr", 5, 1, {0, 0, 0, 0}, {0, 1, 0, 1}},
                                              {"not", 5, 2, {0, 0, 0, 0}, {0, 1, 0, 1}},
                                              {"inc", 5, 3, {0, 0, 0, 0}, {0, 1, 0, 1}},
                                              {"dec", 5, 4, {0, 0, 0, 0}, {0, 1, 0, 1}},
                                              {"jmp", 9, 1, {0, 0, 0, 0}, {0, 1, 1, 0}},
                                              {"bne", 9, 2, {0, 0, 0, 0}, {0, 1, 1, 0}},
                                              {"jsr", 9, 3, {0, 0, 0, 0}, {0, 1, 1, 0}},
                                              {"red", 12, 0, {0, 0, 0, 0}, {0, 1, 0, 1}},
                                              {"prn", 13, 0, {0, 0, 0, 0}, {1, 1, 0, 1}},
                                              {"rts", 14, 0, {0, 0, 0, 0}, {0, 0, 0, 0}},
                                              {"stop", 15, 0, {0, 0, 0, 0}, {0, 0, 0, 0}}};

/* get instruction number in the table by its name */
int get_instruction_id(char *name)
{
    int i;
    for (i = 0; i < INSTRUCTION_TABLE_SIZE; i++)
    {
        if(!strcmp(name, instruction_table[i].name))
            return i;
    }
    return ERR_INSTRUCTION_NOT_FOUND;
}

/* get instruction opcode by the instruction number in the table */
unsigned short get_opcode(unsigned short instruction_id)
{
    return instruction_table[instruction_id].opcode;
}

/* get instruction funct by the instruction number in the table */
unsigned short get_funct(unsigned short instruction_id)
{
    return instruction_table[instruction_id].funct;
}

/* returns the number of operands given instruction takes */
int get_number_of_operands(unsigned short instruction_id)
{
    int n = 0;
    if (*((unsigned int *)&instruction_table[instruction_id].src_op_addressing))
        n++;
    if (*((unsigned int *)&instruction_table[instruction_id].dst_op_addressing))
        n++;
    return n;
}

/* check if addressing method is in the supported methods */
int is_addressing_method_supported(supported_addressing_methods supported_methods, int method)
{
    switch (method)
    {
        case ADDR_IMMEDIATE:
            return supported_methods.immediate;
        case ADDR_DIRECT:
            return supported_methods.direct;
        case ADDR_RELATIVE:
            return supported_methods.relative;
        case ADDR_REG_DIRECT:
            return supported_methods.register_direct;
        default:
            return 0;
    }
}

/* check if addressing method is in the supported methods of the source operands */
int is_source_addressing_method_supported(int instruction_id, int method)
{
    return is_addressing_method_supported(instruction_table[instruction_id].src_op_addressing, method);
}

/* check if addressing method is in the supported methods of the destination operands */
int is_dest_addressing_method_supported(int instruction_id, int method)
{
    return is_addressing_method_supported(instruction_table[instruction_id].dst_op_addressing, method);
}

/* initialize instruction struct by it's id */
instruction* init_instruction(instruction *dst, int instruction_id)
{
    dst->opcode = get_opcode(instruction_id);
    dst->funct = get_funct(instruction_id);
    dst->source_addressing_method = 0;
    dst->source_register = 0;
    dst->dest_addressing_method = 0;
    dst->dest_register = 0;
    dst->A = 1;
    dst->R = 0;
    dst->E = 0;
    return dst;
}
