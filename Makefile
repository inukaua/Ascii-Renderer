out: main.o
	gcc main.o -lm -o out

main.o: main.c
	gcc -c main.c

clean:
	rm *.o out
