FLAGS = -Wall -std=c99

shell379: shell379.o shell_cmd.o process_table.o
	gcc $(FLAGS) shell_cmd.o shell379.o process_table.o -o shell379  

shell379.o: shell379.h shell379.c
	gcc $(FLAGS) -c -g shell379.c -o shell379.o 

shell_cmd.o: shell379.h shell_cmd.c
	gcc $(FLAGS) -c -g shell_cmd.c -o shell_cmd.o

process_table.o: shell379.h process_table.c
	gcc $(FLAGS) -c -g process_table.c -o process_table.o

debug: shell379.o shell_cmd.o process_table.o
	gcc $(FLAGS) -g shell_cmd.o shell379.o process_table.o -o shell379  

clean: 
	rm -f *.o
	rm -f shell379
