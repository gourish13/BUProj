CC=gcc
USER=user2

process1: process1.h process1.c db.h db.c
	$(CC) -o process1 process1.c db.c `mysql_config --libs` -ljson-c

process2: process2.h process2.c
	$(CC) -o process2 process2.c -ljson-c

install: process1 process2
	sudo cp process2 /home/$(USER)/

clean:
	sudo rm -rf /home/$(USER)/{process2,Employees.json}
