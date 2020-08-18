#ifndef _EXTERNALS_H
#define _EXTERNALS_H

#include "memory_map.h"
#include "linked_list.h"

typedef list externals_table; /* used encapsulate as specified in the maman */

typedef struct {
    char *name;
    word address;
} external_item;

int add_external_item(externals_table *external_symbols, char *name, unsigned int address);
int write_externals_file(char *file_path, externals_table *external_symbols);
void free_externals_table(externals_table *external_symbols);
void init_externals_table(externals_table *external_symbols);

#endif
