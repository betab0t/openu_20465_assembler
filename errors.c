#include "errors.h"

char *error_code_to_string(int error_code)
{
    switch (error_code)
    {
        case ERR_ILLEGAL_CHAR:
            return "illegal character found";
        case ERR_INVALID_SYNTAX:
            return "invalid syntax";
        case ERR_INSTRUCTION_NOT_FOUND:
            return "instruction not found";
        case ERR_INVALID_ADDR_METHOD:
            return "invalid addressing method";
        case ERR_INVALID_NUMBER_OF_OPERANDS:
            return "invalid number of operands";
        case ERR_MEM_ALLOC_FAILED:
            return "memory allocation failed";
        case ERR_SYMBOL_ALREADY_EXISTS:
            return "symbol already exists";
        case ERR_OVERFLOW:
            return "overflow";
        case ERR_LEFTOVER:
            return "leftover text/data";
        case ERR_MISSING_SYMBOL:
            return "missing symbol";
        case ERR_EMPTY_STRING:
            return "empty string";
        case ERR_INVALID_VALUE:
            return "invalid value";
        case ERR_NOT_GUIDE_STATEMENT:
            return "not a guide statement";
        case ERR_INVALID_GUIDE:
            return "invalid guide statement";
        case ERR_COULD_NOT_OPEN_FILE:
            return "could not open file";
        case ERR_INVALID_REG_NAME:
            return "invalid register name";
        case ERR_VALUE_OUT_OF_RANGE:
            return "integer value out of range";
        case ERR_LABEL_TOO_LONG:
            return "label too long";
        case ERR_INVALID_LABEL:
            return "invalid label";
        case ERR_INT24_OVERFLOW:
            return "number too big for 24-bit integer";
        case ERR_INT21_OVERFLOW:
            return "number too big for 21-bit integer";
        case ERR_MISSING_VALUE:
            return "missing value";
        default:
            return "unknown error code";
    }
}
