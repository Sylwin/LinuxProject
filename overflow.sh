rm -rf fifos logs
bash createFifos.sh 5

./datownik.o -m0.000001 -s1 | ./powielacz.o -pfifos/fifo -c4
