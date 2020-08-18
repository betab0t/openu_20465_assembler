#ifndef _UTILITIES_H
#define _UTILITIES_H

#include "memory_map.h"
#include "symbols_table.h"

/* check if line starts with word */
#define STARTS_WITH(line, word) (!strncmp(line, word, strlen(word)))

#define LINE_MAX 80 + 3 /* 80 chars + \n (or \r\n) + null terminator */
#define MAX_FILE_PATH 1024 /* maximum valid file full path */

/* guide types */
#define GUIDE_DATA 1
#define GUIDE_STRING 2
#define GUIDE_ENTRY 3
#define GUIDE_EXTERN 4

/* addressing methods */
#define ADDR_IMMEDIATE 0
#define ADDR_DIRECT 1
#define ADDR_RELATIVE 2
#define ADDR_REG_DIRECT 3

char *skip_whitespaces(char *s);
unsigned int count_occurrences(char of, char *in);

unsigned int chars_to_words(word *dst, char *src);
unsigned int label_len(char *s);
char *skip_label(char *str);
int is_valid_label(char *label);

int read_guide_statement_type(char *line);
int read_reg_number(char *s);
int read_addressing_method(char *s);
int read_int21(char *src, data_word *dst);
int read_int24(char *src, word *dst);

void set_flags_absolute(data_word *dst);
void encode_direct(data_word *dst, symbol_entry *symbol);
void encode_relative(data_word *dst, symbol_entry *symbol, unsigned int ic);

#endif
