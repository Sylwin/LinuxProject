FLAGS = -L./ -Wl,-rpath=./ -lCommon


all: datownik.o odbiornik.o powielacz.o

datownik.o: datownik.c
	gcc datownik.c -lrt -o datownik.o ${FLAGS}

odbiornik.o: odbiornik.c
	gcc odbiornik.c -lrt -o odbiornik.o ${FLAGS}

powielacz.o: powielacz.c
	gcc powielacz.c -lm -o powielacz.o

libCommon.so : common.c
	gcc -o libCommon.so common.c -fPIC --shared -lm

clean:
	rm -rf *.o
