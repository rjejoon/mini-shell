FLAGS = -Wall -std=c99

shell379: shell379.o
	gcc $(FLAGS) -o shell379 shell379.c

shell379.o: shell379.h shell379.c
	gcc $(FLAGS) -c -g -o shell379.o shell379.c

debug: shell379.o
	gcc $(FLAGS) -g -o shell379 shell379.c

clean: 
	rm -f *.o
	rm -f shell379
