CC=gcc

LIBS=-L/usr/lib64/mysql -lmysqlclient -lz -lzstd -lssl -lcrypto -lresolv -lm -ljson-c

process1: process1.c db.h db.c
	$(CC) -o process1 process1.c db.c $(LIBS)

process2: process2.c
	$(CC) -o process2 process2.c -ljson-c
