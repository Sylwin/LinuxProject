# Powielanie wszystkich wpisów datowika do zadanych powielaczowi fifo

rm -rf fifos
bash createFifos.sh 5

./datownik.o -m0.000002 -s1 | ./powielacz.o -pfifos/fifo -c4 -Llogs
