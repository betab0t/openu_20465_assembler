#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "symbols_table.h"
#include "utilities.h"
#include "errors.h"

/* init a given symbol table */
void init_symbol_table(symbol_table *table)
{
    table->head = NULL;
    table->tail = NULL;
}

/* check if this table contains any entries */
int is_symbols_table_empty(symbol_table *table)
{
    return is_empty((list *)table);
}

/* add a new symbol to table */
int add_symbol(symbol_table *table, char *name, unsigned int val, symbol_type type)
{
    int res = ERR_MEM_ALLOC_FAILED;
    symbol_entry *new_symbol_entry = malloc(sizeof(symbol_entry)); /* allocate heap memory for new item */
    if(new_symbol_entry)
    {
        res = is_valid_label(name); /* validate this label */
        if(res == OK)
        {
            /* copy everything */
            strncpy(new_symbol_entry->name, name, MAX_LABEL_LEN); /* we did check the label is valid so its in proper length */
            new_symbol_entry->val = val;
            new_symbol_entry->type = type;
            new_symbol_entry->is_entry = 0;
            
            /* insert to memory items list */
            res = insert((list *)table, new_symbol_entry);
        }
    }
    return res;
}

/* resolve a symbol from the table by name. returns symbol entry pointer on success, NULL otherwise. */
symbol_entry *resolve_symbol(symbol_table *table, char *name)
{
    node *curr = table->head;
    while(curr)
    {
        if(!strcmp(((symbol_entry *)curr->data)->name, name))
        {
            return (symbol_entry *)curr->data;
        }
        curr = curr->next;
    }
    return NULL;
}

/* add val to values of all symbols of type. returns number of symbol that were updated */
int update_symbols_addresses(symbol_table *table, symbol_type type, unsigned int val)
{
    int res = 0;
    node *curr = table->head;
    while(curr)
    {
        if(((symbol_entry *)curr->data)->type == type)
        {
            ((symbol_entry *)curr->data)->val += val;
            res++;
        }
        curr = curr->next;
    }
    return res;
}

/* print a given table */
void print_symbols_table(symbol_table *table)
{
    node *curr = table->head;
    printf("DEBUG: SYMBOLS TABLE\r\n=======================\r\n");
    while(curr)
    {
        printf("'%s'\t%d\t%d %d\r\n",
                ((symbol_entry *)curr->data)->name,
                ((symbol_entry *)curr->data)->val,
                ((symbol_entry *)curr->data)->type,
                ((symbol_entry *)curr->data)->is_entry);
        curr = curr->next;
    }
    printf("=======================\r\n");
}


/* write all the symbols marked as entry to entries file in the format specified in the maman */
int write_entries_file(symbol_table *table, char *file_path)
{
    char name[MAX_FILE_PATH];
    FILE *fh;
    int number_of_lines_written = -1;
    node *curr = table->head;

    sprintf((char *)&name, "%s.ent", file_path);
    fh = fopen(name, "w");
    if(fh)
    {
        while(curr)
        {
            if (((symbol_entry *)curr->data)->is_entry)
            {
                fprintf(fh, "%s %07u\n", ((symbol_entry *)curr->data)->name, ((symbol_entry *)curr->data)->val);
                number_of_lines_written++;
            }
            curr = curr->next;
        }
        fclose(fh);
    }
    return number_of_lines_written;
}

/* free the whole table */
void free_symbols_table(symbol_table *table)
{
    node *prev_node, *curr_node = table->head;
    while(curr_node)
    {
        /* free the data */
        free((symbol_entry *)curr_node->data);

        /* free the node itself */
        prev_node = curr_node;
        curr_node = curr_node->next;
        free(prev_node);
    }
}

