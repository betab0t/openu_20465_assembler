#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>

#include "utilities.h"
#include "instructions_table.h"
#include "symbols_table.h"
#include "memory_map.h"
#include "errors.h"
#include "externals.h"

/* split operands string into first and second operands */
int split_operands(char *operands_str, char *operand1, char *operand2)
{
    char command[12];
    int res = sscanf(operands_str, "%s %[^,\r\n],%[^,\r\n]", (char *)&command, operand1, operand2);
    return res ? --res : res;
}

/* complete the encoding of instructions which depended on symbols/labels */
int complete_instruction_encoding(char *line, memory_segment *code_segment, memory_item *curr, symbol_table *symbols, externals_table *external_symbols)
{
    int res = SUCCESS;
    char operand1[LINE_MAX], operand2[LINE_MAX];
    symbol_entry *symbol;
    char *symbol_name;
    int number_of_operands;
    instruction *inst = (instruction *)curr->data;
    data_word *operands = curr->size_in_words > 1 ? (data_word *)(inst + 1) : NULL;
    number_of_operands = split_operands(line, operand1, operand2);

    /* update the source operand if required */
    if(number_of_operands == 2 && (inst->source_addressing_method == ADDR_DIRECT || inst->source_addressing_method == ADDR_RELATIVE))
    {
        /* read the symbol name string */
        symbol_name = skip_whitespaces(operand1);
        if(inst->source_addressing_method == ADDR_RELATIVE)
            symbol_name++;

        /* resolve symbol */
        if ((symbol = resolve_symbol(symbols, symbol_name)))
        {
            /* encode symbol address according to the operands addressing method */
            if(inst->source_addressing_method == ADDR_DIRECT)
                encode_direct(operands, symbol);
            else
                encode_relative(operands, symbol, calc_absolute_address(code_segment, curr));
            operands++;

            /* save external symbols to externals table */
            if(symbol->type == external)
                add_external_item(external_symbols, symbol->name, calc_absolute_address(code_segment, curr) + 1);
        }
        else
        {
            res = ERR_MISSING_SYMBOL;
        }
    }

    /* update the destination operand if required */
    if(res != ERR_MISSING_SYMBOL && (inst->dest_addressing_method == ADDR_DIRECT || inst->dest_addressing_method == ADDR_RELATIVE))
    {
        /* read the symbol name string */
        symbol_name = skip_whitespaces(number_of_operands == 2 ? operand2 : operand1);
        if(inst->dest_addressing_method == ADDR_RELATIVE)
            symbol_name++; /* skip '&' char */

        /* resolve symbol */
        if((symbol = resolve_symbol(symbols, symbol_name)))
        {
            /* encode symbol address according to the operands addressing method */
            if(inst->dest_addressing_method == ADDR_DIRECT)
                encode_direct(operands, symbol);
            else
                encode_relative(operands, symbol, calc_absolute_address(code_segment, curr));

            /* save external symbols to externals table */
            if(symbol->type == external)
                add_external_item(external_symbols, symbol->name, calc_absolute_address(code_segment, curr) + number_of_operands);
        }
        else
        {
            res = ERR_MISSING_SYMBOL;
        }
    }
    return res;
}

/* update a symbol to entry type */
int update_entry(symbol_table *symbols, char *line)
{
    int res = ERR_MISSING_SYMBOL;
    char *start, *end;
    symbol_entry *symbol;
    start = skip_whitespaces(line + 6); /* skip '.entry' word and extra whitespaces */

    /* remove extra whitespaces after the label */
    for(end = start + 1; *end && !isspace(*end); end++);
    *end = '\x0';

    /* find the symbol */
    if((symbol = resolve_symbol(symbols, start)))
    {
        /* set symbol to be an entry */
        symbol->is_entry = 1;
        res = SUCCESS;
    }
    return res;
}

/* process a single line of the file */
int second_pass_process_line(char *line, unsigned int line_number, memory_segment *code_segment, memory_segment *data_segment, symbol_table *symbols, externals_table *external_symbols)
{
    int res;
    memory_item *curr;

    /* [STEP 2] skip prepended labels */
    line = skip_whitespaces(skip_label(line));

    switch ((res = read_guide_statement_type(line)))
    {
        case GUIDE_DATA:
        case GUIDE_STRING:
        case GUIDE_EXTERN:
            break; /* skip .data .string or .extern guide statements */

        case GUIDE_ENTRY:
            res = update_entry(symbols, line);
            break;

        case ERR_NOT_GUIDE_STATEMENT:
            /* get the instruction we decoded in the first pass */
            curr = get_memory_item_by_matching_line_number(code_segment, line_number);
            if (curr)
                /* complete this instruction's encoding */
                res = complete_instruction_encoding(line, code_segment, curr, symbols, external_symbols);
            else
                res = ERR_INSTRUCTION_NOT_FOUND;
            break;

        case ERR_INVALID_GUIDE:
            break; /* return with error */
    }
    return res;
}

/* re-process the file line by line and complete encoding lacking instructions */
int second_pass(FILE *fh, memory_segment *code_segment, memory_segment *data_segment, symbol_table *symbols, externals_table *external_symbols)
{
    char buf[LINE_MAX];
    char *line = buf;
    unsigned line_number = 1;
    int res;
    int number_of_errors = 0;

    /* process the line by line */
    while(fgets((char *)&buf, LINE_MAX, fh))
    {
        /* skip blank lines and comments */
        if(*(line = skip_whitespaces(buf)) && *line != ';')
        {
            res = second_pass_process_line(line, line_number, code_segment, data_segment, symbols, external_symbols);
            if(res < 0) /* check for errors */
            {
                printf("ERROR! %s [line %d]\r\n", error_code_to_string(res), line_number);
                number_of_errors++;
            }
        }
        line_number++;
    }
    return number_of_errors;
}

