all: datownik.o odbiornik.o powielacz.o

datownik.o: datownik.c
	gcc datownik.c -lrt -lm -o datownik.o

odbiornik.o: odbiornik.c
	gcc odbiornik.c -lrt -lm -o odbiornik.o

powielacz.o: powielacz.c
	gcc powielacz.c -lm -o powielacz.o

clean:
	rm -rf *.o
