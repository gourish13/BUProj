CC=gcc

process1: process1.c db.h db.c
	$(CC) -o process1 process1.c db.c `mysql_config --libs` -ljson-c

process2: process2.c
	$(CC) -o process2 process2.c -ljson-c

install: process1 process2
	sudo cp process2 /home/sanket/Desktop/clientprj/

clean:
	sudo rm -rf /home/sanket/clientprj/{process2,Employees.json}
