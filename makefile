assembler: assembler.o utilities.o instructions_table.o symbols_table.o memory_map.o first_pass.o second_pass.o linked_list.o externals.o errors.o
	gcc -g -ansi -Wall -pedantic assembler.o utilities.o instructions_table.o symbols_table.o memory_map.o linked_list.o errors.o externals.o first_pass.o second_pass.o -o assembler

assembler.o: assembler.c
	gcc -c -ansi -Wall -pedantic assembler.c -o assembler.o

first_pass.o: first_pass.c first_pass.h
	gcc -c -ansi -Wall -pedantic first_pass.c -o first_pass.o

second_pass.o: second_pass.c second_pass.h
	gcc -c -ansi -Wall -pedantic second_pass.c -o second_pass.o

utilities.o: utilities.c utilities.h
	gcc -c -ansi -Wall -pedantic utilities.c -o utilities.o

instructions_table.o: instructions_table.c instructions_table.h
	gcc -c -ansi -Wall -pedantic instructions_table.c -o instructions_table.o

symbols_table.o: symbols_table.c symbols_table.h
	gcc -c -ansi -Wall -pedantic symbols_table.c -o symbols_table.o

memory_map.o: memory_map.c memory_map.h
	gcc -c -ansi -Wall -pedantic memory_map.c -o memory_map.o

linked_list.o: linked_list.c linked_list.h
	gcc -c -ansi -Wall -pedantic linked_list.c -o linked_list.o

externals.o: externals.c externals.h
	gcc -c -ansi -Wall -pedantic externals.c -o externals.o

errors.o: errors.c errors.h
	gcc -c -ansi -Wall -pedantic errors.c -o errors.o

clean:
	rm *.o assembler
