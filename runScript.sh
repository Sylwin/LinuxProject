rm -rf fifos logs
bash createFifos.sh 5

(./datownik.o -m2.112 -d0.9852 -s1 | ./powielacz.o -pfifos/fifo -c5 -Llogs) & (./odbiornik.o -d5.98 fifos/fifo3)
