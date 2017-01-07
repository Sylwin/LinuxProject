rm -rf fifos

bash createFifos.sh 2
touch fifos/fifo3

./datownik.o -m2.34 -d0.523 -s1 | ./powielacz.o -pfifos/fifo -c4
