rm -rf fifos logs

bash createFifos.sh 2
touch fifos/fifo3

./datownik.o -m0.000001 -s1 | ./powielacz.o -pfifos/fifo -c4 -Llogs
