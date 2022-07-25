CC=gcc

LIBS=-L/usr/lib64/mysql -lmysqlclient -lz -lzstd -lssl -lcrypto -lresolv -lm -ljson-c

process1: process1.c db.h db.c
	$(CC) -o process1 process1.c db.c $(LIBS)

process2: process2.c
	$(CC) -o process2 process2.c -ljson-c

install: process1 process2
	sudo mkdir -p /home/user2/Client
	sudo cp process2 /home/user2/Client

clean:
	rm process1 process2
	sudo rm -rf /home/user2/Client
