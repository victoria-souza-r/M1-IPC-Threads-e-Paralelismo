all: sender worker

sender: sender.c pgm.c
 gcc -Wall sender.c pgm.c -o sender

worker: worker.c pgm.c filters.c queue.c
 gcc -Wall worker.c pgm.c filters.c queue.c -o worker
 -lptrhead

 clean:
  rm -f sender worker
