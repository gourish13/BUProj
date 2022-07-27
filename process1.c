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

/* rows_fetched initialized in db.c 
 * Keeps count of previously retrieved rows */
extern long rows_fetched;
MYSQL *con;
int server_sock, client_sock;
struct sockaddr_in server_addr, client_addr;

/* Initialize socket, bind and listen */
void init_socket(void) {
    // Initialize server socket, on error print and exit
	server_sock = socket(AF_INET, SOCK_STREAM, 0);
	if (server_sock < 0){
		perror("Socket ERROR !");
		exit(1);
	}
	printf("TCP server socket created.\n");

    // Assign address and port to server_addr
	memset(&server_addr, '\0', sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = PORT;
	server_addr.sin_addr.s_addr = inet_addr(IPADDR);

    // bind socket to address and port assigned to server_addr, on error print and exit.
	int n = bind(server_sock, (struct sockaddr*)&server_addr, sizeof(server_addr));
	if (n < 0){
		perror("Bind error");
		exit(1);
	}
	printf("Bind to the port number: %d\n", PORT);
    // Start listening for connection from client
	listen(server_sock, 5);
	printf("Listening to OS_USER_2\n");
}


// Check if no. of rows transfered is same as expected
int do_checksum(int chars_sent) {
        int checksum;
        recv(client_sock, &checksum, sizeof(int), 0);
        printf("Checksum %d\n", checksum);

        if (checksum == chars_sent) { // Checksum OK return back.
            send(client_sock, "Ok", 3, 0); 
            return TRUE_VAL; 
        }
        // Otherwise, Handle checksum fail case
        send(client_sock, "Failed", 7, 0); 
        return FALSE_VAL;
}


// Data transfer between server and client i.e., process1 and process2 respectively.
void transfer_data(void) {
	char buffer[BUFSIZE];
    bzero(buffer, BUFSIZE);

    recv(client_sock, buffer, BUFSIZE, 0); // "waiting for data" message received from process2
    
    // keep checking database for new rows and kepp sending them to process2
    while (TRUE_VAL) {
        long total_rows = get_db_rows_count(con); // Get count of current database records
        // // If no newly inserted rows, then sleep 30sec and check again
        if (rows_fetched == total_rows) {
            sleep(SLEEP_TIME);
            continue;
        }
        // Get result pointer of retrieved rows from DB.
        // CHUNK_COUNT is max. count of rows read from db and send in one go.
        MYSQL_RES *result = fetch_rows(con, CHUNK_COUNT);

        // Clear buffer 
        bzero(buffer, BUFSIZE);
        // Convert result to json string and copy to buffer also get count of records in result
        int rows_converted_count = convert_to_json(result, buffer);

        // Send created json schema to process2 i.e., client
        send(client_sock, buffer, strlen(buffer), 0);
        printf("Chars sent count = %lu\n", strlen(buffer));

        // Checksum
        if ( do_checksum(strlen(buffer)) == TRUE_VAL )
            // Persist the value of rows_fetched
            persist_rows_fetched(con);
        else
            // Set rows_fetched to its previous value
            rows_fetched -= rows_converted_count;

        // Free memory allocated for mysql result struct 
        mysql_free_result(result);
    }
}

// Close socket and deallocate memory for MySQL connection and close it.
void cleanup(void) {
    close_db_connection(con);
    close(server_sock);
}

int main(int argc, char *argv[]){
    
    con = connect_db(); // Connect to DB
    get_persisted_rows_fetched(con); // Get previously read rows count
    init_socket(); // Initialize server socket, bind it to address and listen
    socklen_t addr_size = sizeof(client_addr);

    printf("Rows_Fetched %ld\n", rows_fetched);

    while(TRUE_VAL){
        // Keep on accepting until client successfully connected.
        while ( (client_sock = accept(server_sock, (struct sockaddr*)&client_addr, &addr_size)) < 0 );
        printf("OS_USER_2 connected.\n");
        // Start data transfer between server and client i.e., process1 and process2 respectively.
        transfer_data();
    }

	return 0;
}
