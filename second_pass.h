#include "memory_map.h"
#include "symbols_table.h"
#include "externals.h"

int second_pass(FILE *fh, memory_segment *code_segment, memory_segment *data_segment, symbol_table *symbols, externals_table *external_symbols);
