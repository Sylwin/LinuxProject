rm -rf fifos
mkdir fifos
cd fifos
mkfifo $(seq 0 $1)
cd ..
