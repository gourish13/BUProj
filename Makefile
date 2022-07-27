CC=gcc
USER=user2

dbreader: dbreader.h dbreader.c db.h db.c
	$(CC) -o dbreader dbreader.c db.c `mysql_config --libs` -ljson-c

datareceiver: datareceiver.h datareceiver.c
	$(CC) -o datareceiver datareceiver.c -ljson-c

install: dbreader datareceiver
	sudo cp datareceiver /home/$(USER)/

clean:
	sudo rm -rf /home/$(USER)/{datareceiver,Employees.json}
