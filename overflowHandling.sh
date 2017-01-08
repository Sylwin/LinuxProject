rm -rf fifos logs
bash createFifos.sh 4

./datownik.o -m0.0000001 -s1 | ./powielacz.o -pfifos/fifo -c4
