all: datownik.o odbiornik.o powielacz.o

datownik.o: datownik.c
	gcc datownik.c -lrt -o datownik.o

odbiornik.o: odbiornik.c
	gcc odbiornik.c -lrt -o odbiornik.o

powielacz.o: powielacz.c
	gcc powielacz.c -o powielacz.o

clean:
	rm -rf *.o
