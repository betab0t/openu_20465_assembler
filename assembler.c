#include <stdio.h>

#include "first_pass.h"
#include "second_pass.h"
#include "memory_map.h"
#include "symbols_table.h"
#include "errors.h"
#include "externals.h"
#include "utilities.h"

/* write all the output(object, externals & entries) files */
void write_output_files(char *original_file_path,
                       memory_segment *code_segment,
                       memory_segment *data_segment,
                       symbol_table *symbols,
                       list *external_symbols)
{
    int res;

    /* write the machine code to file */
    res = write_object_file(original_file_path, code_segment, data_segment);
    if(res < 0)
        printf("ERROR! failed to create object file for \"%s\"\n", original_file_path);

    if(!is_symbols_table_empty(symbols))
    {
        res = write_entries_file(symbols, original_file_path);
        if(res == 0)
            printf("ERROR! failed to create entries file for \"%s\"\n", original_file_path);
    }

    /* write externals to file(if any) */
    if(!is_empty(external_symbols))
    {
        res = write_externals_file(original_file_path, external_symbols);
        if(res == 0)
            printf("ERROR! failed to create externals file for \"%s\"\n", original_file_path);
    }
}

/* assemble a single input file */
void assemble(char *file_path)
{
    int res;
    char filename[MAX_FILE_PATH];
    FILE *fh = NULL;
    memory_segment code_segment, data_segment;
    symbol_table symbols;
    externals_table external_symbols;
    int number_of_errors;

    /* initialize memory segments with base IC 100 and base DC 0 */
    init_memory_segment(&code_segment, 100);
    init_memory_segment(&data_segment, 0);

    /* initialize the symbols table */
    init_symbol_table(&symbols);

    /* initialize the list for external symbols (which we might find on the second pass) */
    init_externals_table(&external_symbols);

    /* add '.as' type to filename */
    snprintf((char *)&filename, sizeof(filename), "%s.as", file_path);

    /* try to open input file if specified by the user */
    fh = fopen(filename, "r");
    if (fh)
    {
        /* print current filename */
        printf(">> Assembling \"%s\"...\n", filename);

        /* start the first pass */
        number_of_errors = first_pass(fh, &code_segment, &data_segment, &symbols);

        /* calculate were data segment should start */
        res = size_of_segment(&code_segment) + code_segment.base_address;

        /* updated symbols and data addresses according to the final size of the code segment */
        update_symbols_addresses(&symbols, data, res);
        data_segment.base_address = res;

        /* set the file pointer back to start */
        rewind(fh);

        /* start the second pass */
        number_of_errors += second_pass(fh, &code_segment, &data_segment, &symbols, &external_symbols);

        /* only create the files if no errors */
        if(!number_of_errors)
        {
            write_output_files(file_path, &code_segment, &data_segment, &symbols,  &external_symbols);
            puts(">> No errors... writing to disk...");
        }
        else
        {
            printf(">> %s found, quitting...\n", res > 1 ? "Errors" : "Error");
        }

        /* free everything */
        free_memory_segment(&code_segment);
        free_memory_segment(&data_segment);
        free_symbols_table(&symbols);
        free_externals_table(&external_symbols);

        /* close the file */
        fclose(fh);
    }
    else
    {
        printf("ERROR! could not open file \"%s\"\n", filename);
    }
}

/* parse command line and assemble files */
int main(int argc, char *argv[])
{
    int i;

    /* assemble all the files in argv */
    for(i = 1; i < argc; i++)
    {
        assemble(argv[i]);
    }

    /* return number of files */
    return i;
}

