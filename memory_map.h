#ifndef _MEMORY_MAP_H
#define _MEMORY_MAP_H

#include "linked_list.h"

typedef struct {
    unsigned int E:1;
    unsigned int R:1;
    unsigned int A:1;
    unsigned int funct:5;
    unsigned int dest_register:3;
    unsigned int dest_addressing_method:2;
    unsigned int source_register:3;
    unsigned int source_addressing_method:2;
    unsigned int opcode:6;
} instruction;

typedef struct {
    unsigned int E:1;
    unsigned int R:1;
    unsigned int A:1;
    int val:21;
} data_word;

typedef struct {
    unsigned int val:24;
} word;

typedef struct {
    unsigned int base_address;
    list items;
} memory_segment;

typedef struct {
    unsigned int relative_address;
    unsigned int size_in_words;
    word *data;
    unsigned int matching_line_number;
} memory_item;

void init_memory_segment(memory_segment *segment, unsigned int base_address);
int add_memory_item(memory_segment *segment, unsigned int size_in_words, word *data, unsigned int matching_line_number);
memory_item *get_memory_item_by_matching_line_number(memory_segment *segment, unsigned int matching_line_number);
void print_memory_segment(memory_segment *segment);
unsigned int size_of_segment(memory_segment *segment);
unsigned int calc_absolute_address(memory_segment *segment, memory_item *data);
void free_memory_segment(memory_segment *segment);
int write_object_file(char *file_path, memory_segment *code_segment, memory_segment *data_segment);

#endif

