rm -rf fifos

bash createFifos.sh 2
#mkdir fifos
touch fifos/fifo3

./datownik.o -m0.00000002 -s1 | ./powielacz.o -pfifos/fifo -c4
