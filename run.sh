# Powielanie wszystkich wpisów datowika do zadanych powielaczowi fifo

rm -rf fifos
bash createFifos.sh 5

(./datownik.o -m2.432 -d0.9852 -s1 | ./powielacz.o -pfifos/fifo -c4 -Llogs) & (./odbiornik.o -d3.98 fifos/fifo3)
