#ifndef _SYMBOLS_TABLE_H
#define _SYMBOLS_TABLE_H

#include "linked_list.h"

/* maximum valid label length, in chars, without null terminator */
#define MAX_LABEL_LEN 31

typedef enum {
    code,
    data,
    external
} symbol_type;

typedef list symbol_table; /* used encapsulate as specified in the maman */

typedef struct {
    char name[MAX_LABEL_LEN + 1];
    unsigned int val;
    symbol_type type;
    unsigned int is_entry:1;
} symbol_entry;

void init_symbol_table(symbol_table *table);
int add_symbol(symbol_table *table, char *name, unsigned int val, symbol_type type);
symbol_entry *resolve_symbol(symbol_table *table, char *name);
int update_symbols_addresses(symbol_table *table, symbol_type type, unsigned int val);
int write_entries_file(symbol_table *table, char *file_path);
int is_symbols_table_empty(symbol_table *table);
void print_symbols_table();
void free_symbols_table(symbol_table *table);

#endif
