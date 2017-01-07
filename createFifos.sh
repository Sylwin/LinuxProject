rm -rf fifos
mkdir fifos
cd fifos
for f in $(seq 0 $1); do
    mkfifo fifo$f
done
cd ..
