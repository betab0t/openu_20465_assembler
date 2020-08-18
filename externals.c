#include <stdio.h>
#include <stdlib.h>

#include "externals.h"
#include "linked_list.h"
#include "errors.h"
#include "utilities.h"

/* add new external("extern") symbol to list. returns SUCCESS if succeeded, error code otherwise. */
int add_external_item(externals_table *external_symbols, char *name, unsigned int address)
{
    int res = ERR_MEM_ALLOC_FAILED;
    external_item *new_external_item = malloc(sizeof(external_item)); /* allocate heap memory for new item */

    /* make source allocation succeeded */
    if(new_external_item)
    {
        /* copy everything */
        new_external_item->name = name;
        new_external_item->address.val = address;

        /* insert to memory items list */
        res = insert(external_symbols, new_external_item);
    }
    return res;
}

/* initialize table */
void init_externals_table(externals_table *external_symbols)
{
    init_list((list *)external_symbols);
}

/* dump external symbols to file in the format spec in the maman */
int write_externals_file(char *file_path, externals_table *external_symbols)
{
    char name[MAX_FILE_PATH];
    FILE *fh;
    int number_of_lines_written = 0;
    node *curr_node = external_symbols->head;
    external_item *curr_item;

    sprintf((char *)&name, "%s.ext", file_path);
    fh = fopen(name, "w");
    if(fh)
    {
        while(curr_node)
        {
            curr_item = (external_item *)curr_node->data;
            fprintf(fh, "%s %07u\n", curr_item->name, curr_item->address.val);
            number_of_lines_written++;
            curr_node = curr_node->next;
        }
        fclose(fh);
    }
    return number_of_lines_written;
}

/* free the whole table */
void free_externals_table(externals_table *external_symbols)
{
    node *prev_node, *curr_node = external_symbols->head;
    while(curr_node)
    {
        /* free the data */
        free(curr_node->data);

        /* free the node itself */
        prev_node = curr_node;
        curr_node = curr_node->next;
        free(prev_node);
    }
}


