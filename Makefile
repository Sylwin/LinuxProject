all: datownik.o odbiornik.o powielacz.o

datownik.o: datownik.c
	gcc datownik.c -lm -lrt -o datownik.o

odbiornik.o: odbiornik.c
	gcc odbiornik.c -lm -o odbiornik.o

powielacz.o: powielacz.c
	gcc powielacz.c -lm -o powielacz.o
