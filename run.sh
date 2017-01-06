rm -rf fifos
bash createFifos.sh 5
(./datownik.o -m2.432 -ffifos/1 -ffifos/3 -s1 | ./powielacz.o -pfifos/ -c4) & (./odbiornik.o -d3.98 fifos/3)
