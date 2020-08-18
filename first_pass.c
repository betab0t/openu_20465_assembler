#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>

#include "utilities.h"
#include "instructions_table.h"
#include "symbols_table.h"
#include "memory_map.h"
#include "errors.h"

/* read operands from string. returns total size of encoded instruction on success, error code otherwise. */
int read_operands(instruction *inst, int instruction_id, word *opt_operands, char *operands_str)
{
    int res = 1;
    int number_of_operands_found = 0;
    char operand1[LINE_MAX], operand2[LINE_MAX];
    char junk;
    int source_addressing_method, dest_addressing_method;
    char *dest_operand_str = operand1;

    /* try to split the operands string */
    if(*operands_str)
    {
        /* read the operands and also junk chars(if any) */
        number_of_operands_found = sscanf(operands_str, "%[^,\r\n],%[^,\r\n] %c", (char *)&operand1, (char *)&operand2, &junk);

        /* make sure we got the currect number of operands for this instruction */
        if(number_of_operands_found == get_number_of_operands(instruction_id))
        {
            /* decode the source operand(if any) */
            if(number_of_operands_found == 2)
            {
                /* read the source operand addressing method */
                source_addressing_method = read_addressing_method(operand1);

                /* make sure this addressing method is supported by the instruction */
                if(is_source_addressing_method_supported(instruction_id, source_addressing_method))
                {
                    inst->source_addressing_method = source_addressing_method;
                    switch (source_addressing_method)
                    {
                        case ADDR_REG_DIRECT:
                            if((inst->source_register = read_reg_number(operand1)) < 0)
                                res = ERR_INVALID_REG_NAME;
                            break;

                        case ADDR_IMMEDIATE:
                            if(read_int21(skip_whitespaces(operand1) + 1, (data_word *)opt_operands) == SUCCESS)
                            {
                                set_flags_absolute((data_word *)opt_operands);
                                res++;
                                opt_operands++;
                            }
                            else
                            {
                                res = ERR_VALUE_OUT_OF_RANGE;
                            }
                            break;

                        case ADDR_RELATIVE:
                        case ADDR_DIRECT:
                            /* save space for relative and direct addressing mode operands */
                            res++;
                            /* forward for the destination operand */
                            opt_operands++;
                            break;
                    }
                    dest_operand_str = skip_whitespaces(operand2); /* fix the destination operand string */
                }
                else
                {
                    res = ERR_INVALID_ADDR_METHOD;
                }
            }

            /* encode the destination operand only if we didn't encounter any errors on the way */
            if(res >= 1)
            {
                dest_addressing_method = read_addressing_method(dest_operand_str);
                if(is_dest_addressing_method_supported(instruction_id, dest_addressing_method))
                {
                    inst->dest_addressing_method = dest_addressing_method;
                    switch (dest_addressing_method)
                    {
                        case ADDR_REG_DIRECT:
                            if((inst->dest_register = read_reg_number(dest_operand_str)) < 0)
                                res = ERR_INVALID_REG_NAME;
                            break;

                        case ADDR_IMMEDIATE:
                            /* read value as 21 bits long integer */
                            if(read_int21(skip_whitespaces(dest_operand_str) + 1, (data_word *)opt_operands) == SUCCESS)
                            {
                                set_flags_absolute((data_word *)opt_operands); /* set ARE flags */
                                res++;
                            }
                            else
                            {
                                res = ERR_VALUE_OUT_OF_RANGE;
                            }
                            break;
                        case ADDR_RELATIVE:
                        case ADDR_DIRECT:
                            res++; /* save space for relative and direct addressing mode operands */
                            break;
                    }
                }
                else
                {
                    res = ERR_INVALID_ADDR_METHOD;
                }
            }
        }
        else
        {
            res = ERR_INVALID_NUMBER_OF_OPERANDS;
        }
    }
    else
    {
        res = 1;
    }
    return res;
}

/* read instruction name and operands from strings and decode to dst */
int read_instruction_name_and_operands(word **dst, char *instruction_name_str, char *operands_str)
{
    int res = ERR_INSTRUCTION_NOT_FOUND;
    unsigned short instruction_id = get_instruction_id(instruction_name_str);

    if(instruction_id >= 0)
    {
        /* allocate zero initialized memory for decoded instruction & optional data words */
        *dst = (word *)calloc(1 + get_number_of_operands(instruction_id), sizeof(word));
        if(*dst)
        {
            /* decoded instruction and operands */
            init_instruction((instruction *)*dst, instruction_id);
            res = read_operands((instruction *)*dst, instruction_id, *dst + 1, operands_str);
        }
        else
        {
            res = ERR_MEM_ALLOC_FAILED;
        }
    }
    return res;
}

/* read instruction line and decode to dst */
int read_instruction_line(word **dst, char *line)
{
    char *instruction_name_str, *operands_str = NULL;

    /* skip prepended spaces(if any) */
    line = skip_whitespaces(line);

    /* move the pointer to first space(if any) */
    for(instruction_name_str = line; *line && !isspace(*line); line++);

    /* check there are operands */
    if (isspace(*line))
    {
        *line++ = '\x0'; /* split instruction name from operands */
        operands_str = line;
    }

    /* continue processing current line */
    return read_instruction_name_and_operands(dst, instruction_name_str, operands_str);
}

/* read a single data declaration line and decode it into pre-allocated buf */
int read_data_declaration_inner(word *buf, char *data_str)
{
    int res = SUCCESS;
    char *start = data_str, *end;
    int count = 0;
    word tmp;

    do
    {
        start = skip_whitespaces(start);

        /* check if any more number */
        if((end = strchr(start, ',')))
            *end = '\x0'; /* split the string */

        if(!strlen(start))
        {
            /* no value to read */
            res = ERR_EMPTY_STRING;
            break;
        }
        else if((res = read_int24(start, &tmp)) != SUCCESS)
        {
            /* not a valid decimal number */
            res = ERR_INT24_OVERFLOW;
            break;
        }
        /* save current value */
        buf[count].val = tmp.val;

        /* forward the string start pointer */
        start = end;
        count++;
    } while(start++);
    return res != SUCCESS ? res : count;
}

/* allocate memory and read a single data declaration line and decode it into dst */
int read_data_declaration(word **dst, char *data_str)
{
    int res = 0;
    unsigned int expected_number_of_items;
    word *buf;

    /* count how many commas we got so we know how many data items should be */
    expected_number_of_items = count_occurrences(',', data_str) + 1;
    if(expected_number_of_items)
    {
        buf = calloc(expected_number_of_items, sizeof(word));
        if(buf)
        {
            /* read and make sure the we got the right number of items */
            res = read_data_declaration_inner(buf, data_str);
            if (res == expected_number_of_items)
            {
                *dst = buf;
            }
            else
            {
                free(buf); /* free this buffer because it wont be used */
            }
        }
        else
        {
            res = ERR_MEM_ALLOC_FAILED;
        }
    }
    return res;
}

/* read a single string declaration line and decode it into dst */
int read_string_declaration(word **dst, char *data_str)
{
    int res = ERR_INVALID_SYNTAX;
    char *start, *end;
    word *buf;

    /* split the actual string between the quotation marks */
    if((start = strchr(data_str, '"')) && *start)
    {
        /* find closing comma and make sure its the last char */
        if ((end = strchr(++start, '"')) && !*skip_whitespaces(end + 1))
        {
            *end = '\x0'; /* split the string */
            buf = calloc((end - start) + 1, sizeof(word)); /* allocate zero initialized memory */
            if(buf)
            {
                res = chars_to_words(buf, start); /* read the chars to words buffer */
                *dst = buf;
            }
            else
            {
                res = ERR_MEM_ALLOC_FAILED;
            }
        }
    }
    return res;
}

/* read a single external declaration line and add to symbols table */
int read_extern_declaration(char *buf, symbol_table *symbols)
{
    unsigned int len;
    int res = ERR_INVALID_SYNTAX;

    buf = skip_whitespaces(buf);
    len = label_len(buf);

    /* make sure we find a label and no extra chars */
    if(len && !*skip_whitespaces(buf + len))
    {
        *(buf + len) = '\x0';
        if((res = add_symbol(symbols, buf, 0, external)) == SUCCESS)
            res = 0;
    }
    return res;
}

/* read a single guide line and decode it into dst */
int read_guide_line(word **dst, char *line, symbol_table *symbols)
{
    int res = ERR_INVALID_SYNTAX;

    /* check for prepended dot */
    if (*line++ != '.')
        return 0;

    /* read declaration by its type */
    if (STARTS_WITH(line, "data"))
    {
        res = read_data_declaration(dst, line + 4);
    }
    else if (STARTS_WITH(line, "string"))
    {
        res = read_string_declaration(dst, line + 6);
    }
    else if (STARTS_WITH(line, "entry"))
    {
        res = 0; /* we'll handle it on the second pass */
    }
    else if (STARTS_WITH(line, "extern"))
    {
        res = read_extern_declaration(line + 6, symbols);
    }
    return res;
}

/* handle a single data/code line */
int process_line(char *line, unsigned int line_number, memory_segment *code_segment, memory_segment *data_segment, symbol_table *symbols)
{
    char *label = NULL;
    int res, tmp;
    unsigned int label_address = 0;
    word *machine_code;
    symbol_type type = data;
    char *sep;

    /* check for label at the start of this line */
    if((sep = strchr(line, ':')))
    {
        *sep = '\x0';
        label = line;
        line = skip_whitespaces(sep + 1);
    }

    /* quickly check if this is a guide line, further check will be made afterwards */
    if(*line == '.')
    {
        /* read as guide line */
        res = read_guide_line(&machine_code, line, symbols);
        if(res > 0)
        {
            /* save to data segment */
            res = add_memory_item(data_segment, res, machine_code, line_number);
            if(res)
                label_address = res;
        }
    }
    else
    {
        /* read as code/instruction line */
        res = read_instruction_line(&machine_code, line);
        if(res > 0)
        {
            /* save to code segment */
            res = add_memory_item(code_segment, res, machine_code, line_number);
            if(res)
                label_address = res;
            type = code;
        }
    }

    /* save the label(if any) in the symbols table for later */
    if(label)
    {
        tmp = add_symbol(symbols, label, label_address, type);
        if(tmp != SUCCESS)
            res = tmp;
    }
    return res;
}

/* process the file for the first time and decode what we can. returns number of error(lines) found in the file. */
int first_pass(FILE *fh, memory_segment *code_segment, memory_segment *data_segment, symbol_table *symbols)
{
    char buf[LINE_MAX];
    char *line = buf;
    int res;
    unsigned line_number = 1;
    int number_of_errors = 0;

    /* read and process one line at a time */
    while(fgets((char *)&buf, LINE_MAX, fh))
    {
        /* skip blank lines and comments */
        if(*(line = skip_whitespaces(buf)) && *line != ';')
        {
            res = process_line(line, line_number, code_segment, data_segment, symbols);
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
