CC = gcc
CFLAGS = -Wall -std=c99
OBJS = shell379.o shell_cmd.o process_table.o


shell379: $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o shell379  

shell379.o: shell379.h shell379.c
	$(CC) $(CFLAGS) -c -g shell379.c -o shell379.o 

shell_cmd.o: shell379.h shell_cmd.c
	$(CC) $(CFLAGS) -c -g shell_cmd.c -o shell_cmd.o

process_table.o: shell379.h process_table.c
	$(CC) $(CFLAGS) -c -g process_table.c -o process_table.o

debug: $(OBJS) 
	$(CC) $(CFLAGS) -g $(OBJS) -o shell379  

compress: 
	zip assign1.zip shell379.c shell_cmd.c process_table.c shell379.h makefile

clean: 
	rm -f *.o
	rm -f shell379
