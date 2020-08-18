#ifndef _INSTRUCTIONS_TABLE_H
#define _INSTRUCTIONS_TABLE_H

#include "memory_map.h"

int get_number_of_operands(unsigned short instruction_id);
int is_source_addressing_method_supported(unsigned short instruction_id, int method);
int is_dest_addressing_method_supported(unsigned short instruction_id, int method);
unsigned short get_opcode(unsigned short instruction_id);
unsigned short get_funct(unsigned short instruction_id);
int get_instruction_id(char *name);
instruction* init_instruction(instruction *dst, int instruction_id);

#endif
