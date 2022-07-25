#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <sys/socket.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <mysql/mysql.h>

#include "db.h"
#include "process1.h"

/* rows_fetched initialized in db.c */
extern long rows_fetched;
MYSQL *con;
int server_sock, client_sock;
struct sockaddr_in server_addr, client_addr;

/* Initialize socket, bind and listen */
void init_socket() {
	server_sock = socket(AF_INET, SOCK_STREAM, 0);
	if (server_sock < 0){
		perror("Socket ERROR !");
		exit(1);
	}
	printf("TCP server socket created.\n");

	memset(&server_addr, '\0', sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = PORT;
	server_addr.sin_addr.s_addr = inet_addr(IPADDR);

	int n = bind(server_sock, (struct sockaddr*)&server_addr, sizeof(server_addr));
	if (n < 0){
		perror("Bind error");
		exit(1);
	}
	printf("Bind to the port number: %d\n", PORT);

	listen(server_sock, 5);
	printf("Listening to OS_USER_2\n");
}


void do_checksum(int rows_sent) {
        int checksum;
        recv(client_sock, &checksum, sizeof(int), 0);
        printf("Checksum %d\n", checksum);

        if (checksum == rows_sent)
            return; // Checksum OK return back.
        // Otherwise, Handle checksum fail case
}


void transfer_data() {
	char buffer[BUFSIZE];
    bzero(buffer, BUFSIZE);

    recv(client_sock, buffer, BUFSIZE, 0); // waiting for data 
    
    while (1) {
        long total_rows = get_db_rows_count(con);
        if (rows_fetched == total_rows) {
            sleep(30);
            continue;
        }

        MYSQL_RES *result = fetch_rows(con, 100);

        bzero(buffer, BUFSIZE);
        int rows_converted_count = convert_to_json(result, buffer);

        send(client_sock, buffer, strlen(buffer), 0);
        printf("Rows sent count = %d\n", rows_converted_count);

        // Checksum
        do_checksum(rows_converted_count);
        // Free memory allocated for mysql result struct 
        mysql_free_result(result);
    }
}

void cleanup(void) {
    close_db_connection(con);
    close(server_sock);
}

int main(int argc, char *argv[]){
    
    con = connect_db(); // Connect to DB
    init_socket(); // Initialize server socket, bind it to address and listen
    socklen_t addr_size = sizeof(client_addr);

	while(1){
        // Keep on accepting until client successfully connected.
		while ( (client_sock = accept(server_sock, (struct sockaddr*)&client_addr, &addr_size)) < 0 );
		printf("OS_USER_2 connected.\n");
        // Start data transfer between server and client i.e., process1 and process2 respectively.
        transfer_data();
	}

	return 0;
}
