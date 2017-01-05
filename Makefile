all: datownik.o odbiornik.o powielacz.o

datownik.o: datownik.c
	gcc datownik.c -Wall -lrt -lm -o datownik.o

odbiornik.o: odbiornik.c
	gcc odbiornik.c -Wall -lrt -lm -o odbiornik.o

powielacz.o: powielacz.c
	gcc powielacz.c -Wall -lm -o powielacz.o

clean:
	rm -rf *.o
