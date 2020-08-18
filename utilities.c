#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>

#include "memory_map.h"
#include "utilities.h"
#include "errors.h"
#include "instructions_table.h"

#define INT21_MIN -1048575
#define INT21_MAX  1048574
#define INT24_MIN -8388607
#define INT24_MAX  8388606

/* returns a pointer the the first non-whitespace char found or null terminator if not found */
char *skip_whitespaces(char *s)
{
    for(; *s && isspace(*s); s++);
    return s;
}

/* return true if s is a reserved word(instruction name) */
int is_reserved_word(char *s)
{
    return get_instruction_id(s) >= 0;
}

/* returns the length of a given label */
unsigned int label_len(char *s)
{
    char *start = s;

    /* check for reserved words */
    if(is_reserved_word(s))
        return 0;

    /* count only legal chars(a-z,A-Z,0-9) */
    for(; *s && (islower(*s) || isupper(*s) || isdigit(*s)); s++);

    /* return 0 on bad strings */
    return s - start;
}

/* returns a pointer to the original string without the label(if found) */
char *skip_label(char *str)
{
    unsigned int len;
    if((len = label_len(str)) && *(str + len) == ':')
        return (str + len + 1);
    return str;
}

/* convert ascii string integer to 21-bit data word. returns SUCCESS on success, error code otherwise. */
int read_int21(char *src, data_word *dst)
{
    int res = ERR_ILLEGAL_CHAR;
    char *end = NULL;
    int tmp;

    /* read the value */
    errno = 0;
    tmp = (int)strtol(src, &end, 10);

    /* check that all chars are valid digits */
    if(end != src && *skip_whitespaces(end) == '\x0')
    {
        /* check that the conversion to 32-bit int worked and also that the result is in 21-bit range */
        if(errno != ERANGE && INT21_MIN <= tmp && tmp <= INT21_MAX)
        {
            dst->val = tmp;
            res = SUCCESS;
        }
        else
        {
            res = ERR_INT21_OVERFLOW;
        }
    }
    return res;
}

/* convert ascii string integer to 24-bit word. returns SUCCESS on success, error code otherwise. */
int read_int24(char *src, word *dst)
{
    int res = ERR_ILLEGAL_CHAR;
    char *end = NULL;
    int tmp;

    /* read the value */
    errno = 0;
    tmp = (int)strtol(src, &end, 10);

    /* check that all chars are valid digits */
    if(end != src && *skip_whitespaces(end) == '\x0')
    {
        /* check that the conversion to 32-bit int worked and also that the result is in 24-bit range */
        if(errno != ERANGE && INT24_MIN <= tmp && tmp <= INT24_MAX)
        {
            dst->val = tmp;
            res = SUCCESS;
        }
        else
        {
            res = ERR_INT24_OVERFLOW;
        }
    }
    return res;
}

int read_guide_statement_type(char *line)
{
    int res = ERR_NOT_GUIDE_STATEMENT;
    if(*line++ == '.') /* check for dot */
    {
        /* process by the specific guide name */
        if (STARTS_WITH(line, "data"))
            res = GUIDE_DATA;
        else if (STARTS_WITH(line, "string"))
            res = GUIDE_STRING;
        else if (STARTS_WITH(line, "entry"))
            res = GUIDE_ENTRY;
        else if (STARTS_WITH(line, "extern"))
            res = GUIDE_EXTERN;
        else
            res = ERR_INVALID_GUIDE;
    }
    return res;
}

int read_reg_number(char *s)
{
    int num;
    if (sscanf(s, "r%d", &num) == 1 && 0 <= num && num <= 7)
        return num;
    return ERR_INVALID_REG_NAME;
}

int read_addressing_method(char *s)
{
    switch (*(s = skip_whitespaces(s)))
    {
        case '#':
            return ADDR_IMMEDIATE;
        case '&':
            return ADDR_RELATIVE;
        case 'r':
            if (read_reg_number(s) >= 0)
                return ADDR_REG_DIRECT;
        default:
            return ADDR_DIRECT;
    }
}

/* count occurrences of char in string */
unsigned int count_occurrences(char of, char *in)
{
    unsigned int count = 0;
    while(*in)
    {
        if(*in == of)
            count++;
        in++;
    }
    return count;
}

/* convert string to word array. returns size of converted string including null terminator */
unsigned int chars_to_words(word *dst, char *src)
{
    char *curr = src;
    while(*curr)
    {
        dst->val = *curr;
        curr++;
        dst++;
    }
    dst->val = 0; /* add null terminator */
    return (curr - src) + 1;
}

void set_flags_absolute(data_word *dst)
{
    dst->A = 1;
    dst->R = 0;
    dst->E = 0;
}

/* encode direct addressing data word */
void encode_direct(data_word *dst, symbol_entry *symbol)
{
    dst->val = symbol->val;
    switch (symbol->type)
    {
        case code:
        case data:
            dst->A = 0;
            dst->R = 1;
            dst->E = 0;
            break;
        case external:
            dst->A = 0;
            dst->R = 0;
            dst->E = 1;
        default:
            break;
    }
}

/* encode relative addressing data word */
void encode_relative(data_word *dst, symbol_entry *symbol, unsigned int ic)
{
    dst->val = symbol->val - ic;
    set_flags_absolute(dst);
}

/* returns OK if label is valid, error code otherwise */
int is_valid_label(char *label)
{
    /* make sure this label is in proper length */
    if(strlen(label) > MAX_LABEL_LEN)
        return ERR_LABEL_TOO_LONG;

    /* make sure it starts with a letter */
    if(!isupper(*label) && !islower(*label))
        return ERR_INVALID_LABEL;

    /* make sure all the chars are letter or digits */
    while(*label)
    {
        if(!isupper(*label) && !islower(*label) && !isdigit(*label))
            return ERR_INVALID_LABEL;
        label++;
    }
    return OK;
}

/* skips the first word separated by whitespaces. returns a pointer a pointer to the start of the second word or to null terminator, whichever comes first */
char *skip_word(char *line)
{
    for(line = skip_whitespaces(line); *line && !isspace(*line) && *line != ','; line++);
    return line;
}

/* split operands string into first and second operands. returns number of operands found. */
int split_operands(char *operands_str, char *operand1, char *operand2)
{
    int res = 0;
    char *start1, *start2, *end1, *end2, *delim;

    /* check that the string is not empty */
    if(*operands_str)
    {
        if(*(start1 = skip_whitespaces(operands_str)))
        {
            res++;
            end1 = skip_word(start1);

            /* split if comma found */
            delim = skip_whitespaces(end1);
            if(*delim == ',')
            {
                /* make sure its not an empty string */
                if(*(start2 = skip_whitespaces(delim + 1)))
                {
                    res++;
                    end2 = skip_word(start2);

                    /* check for extra chars */
                    if(*skip_whitespaces(end2))
                        res = ERR_INVALID_NUMBER_OF_OPERANDS; /* oops there some extra junk chars here */

                    /* trim second operand */
                    if(*end2)
                        *end2 = '\x0';
                }
            }
            else if(*delim)
            {
                res = ERR_INVALID_NUMBER_OF_OPERANDS;
            }

            /* trim whitespaces from the first operand */
            if(*end1)
                *end1 = '\x0';
        }
    }

    /* copy strings to destination buffers */
    if(res >= 1)
        strncpy(operand1, start1, MAX_LABEL_LEN);

    if(res == 2)
        strncpy(operand2, start2, MAX_LABEL_LEN);

    return res;
}

