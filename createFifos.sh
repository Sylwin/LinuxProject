rm -rf fifos
mkdir fifos
cd fifos

for f in $(seq 1 $1); do
    mkfifo fifo$f
done

cd ..
