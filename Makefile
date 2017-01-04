FLAGS = -L./ -Wl,-rpath=./ -lCommon

all: datownik.o odbiornik.o powielacz.o libCommon.so

datownik.o: datownik.c libCommon.so
	gcc datownik.c -lrt -o datownik.o ${FLAGS}

odbiornik.o: odbiornik.c libCommon.so
	gcc odbiornik.c -lrt -o odbiornik.o ${FLAGS}

powielacz.o: powielacz.c libCommon.so
	gcc powielacz.c -lm -o powielacz.o

libCommon.so : common.c
	gcc -o libCommon.so common.c -fPIC --shared -lm

clean:
	rm -rf *.o *.so
