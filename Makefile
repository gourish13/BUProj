CC=gcc

process1: process1.c db.h db.c
	$(CC) -o process1 process1.c db.c `mysql_config --libs` -ljson-c

process2: process2.c
	$(CC) -o process2 process2.c -ljson-c

install: process1 process2
	sudo mkdir -p /home/user2/Client
	sudo cp process2 /home/user2/Client

clean:
	rm process1 process2
	sudo rm -rf /home/user2/Client
