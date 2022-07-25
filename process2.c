#include <json-c/json_object.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <sys/socket.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <json-c/json.h>

#include "process2.h"

socklen_t addr_size;
struct sockaddr_in addr;
int sock;
FILE *fp;

void init_socket() {
	sock = socket(AF_INET, SOCK_STREAM, 0);
	if (sock < 0){
		perror("Socket error");
		exit(1);
	}
	printf("TCP client socket created.\n");

	memset(&addr, '\0', sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_port = PORT;
	addr.sin_addr.s_addr = inet_addr(IPADDR);
}

json_object * parse_to_json(char *buffer) {
    json_object *array = json_tokener_parse(buffer);
    return array;
}

void open_file(char *mode) {
    fp = fopen(FILENAME, mode); // Open file in append mode
    
    if (fp == NULL) {
        fprintf(stderr, "Error opening file %s\n", FILENAME);
        exit(1);
    }
}

// Check if file is empty or data already present.
int file_isempty() {
    // Move to end of file and check position if 0 means empty file.
    fseek(fp, 0L, SEEK_END);
    return ftell(fp)== 0 ? 1 : 0;
}

// Dump json string to file or append if already present
void dump_or_append_to_file(json_object *array) {
    const char *json_str = json_object_to_json_string_ext(array, JSON_C_TO_STRING_PRETTY);

    // If file is empty directly dump json string
    if ( file_isempty() ) {
        fwrite(json_str, sizeof(char), strlen(json_str), fp);
        return;
    }

    // Otherwise, remove closing ']' append ',' then append json string 
    fseek(fp, -2L, SEEK_CUR);

    fprintf(fp, ",");
    fwrite(json_str + 1, sizeof(char), strlen(json_str) - 1, fp);
}


int main(int argc, char *argv[]){
    init_socket();
    // Clear the Employee.json file if already exists by opening in w mode and closing.
    open_file("w");
    fclose(fp);

	while (connect(sock, (struct sockaddr*)&addr, sizeof(addr)) < 0)
        sleep(30);

	printf("Connected to the OS_USER_1.\n");
	char buffer[BUFSIZE];

    bzero(buffer, sizeof(buffer));
    strcpy(buffer, "waiting for data");
    send(sock, buffer, strlen(buffer), 0);

    int rows_recvd;
    while (1) {
        bzero(buffer, BUFSIZE);
        recv(sock, buffer, BUFSIZE, 0);

        json_object *recvd_arr = parse_to_json(buffer);
        rows_recvd = json_object_array_length(recvd_arr);

        send(sock, &rows_recvd, sizeof(int), 0);

        printf("%s\n", buffer);
        printf("-------------------- %d --------------------\n", rows_recvd);

        open_file("r+");
        // Dump json string to file or append if already present
        dump_or_append_to_file(recvd_arr);
        // Clear and deallocate json object
        json_object_put(recvd_arr);
        fclose(fp);
    }

	close(sock);
	printf("Disconnected from the OS_USER_1\n");

	return 0;
}
