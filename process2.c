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

// Initialize Socket
void init_socket(void) {
    // Initialize client socket, on error print and exit
    sock = socket(AF_INET, SOCK_STREAM, 0); 
    if (sock < 0){
        perror("Socket error");
        exit(1);
    }
    printf("TCP client socket created.\n");

    // Assign address and port of server to addr
    memset(&addr, '\0', sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = PORT;
    addr.sin_addr.s_addr = inet_addr(IPADDR);
}

// Parse the string in buffer to json object/array and return
json_object * parse_to_json(char *buffer) {
    json_object *array = json_tokener_parse(buffer);
    return array;
}

// open file in specified mode.
void open_file(char *mode) {
    fp = fopen(FILENAME, mode); // Open file in append mode
    
    if (fp == NULL) {
        fprintf(stderr, "Error opening file %s\n", FILENAME);
        exit(1);
    }
}

// Check if file is empty or data already present.
int file_isempty(void) {
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

    // Add ',' before appending
    fprintf(fp, ",");
    // Write json string removing the begining '['
    fwrite(json_str + 1, sizeof(char), strlen(json_str) - 1, fp);
}


void transfer_data(void) {
    char buffer[BUFSIZE];
    bzero(buffer, sizeof(buffer)); // Clear buffer
    strcpy(buffer, "waiting for data"); // copy "waiting for data" to buffer
    send(sock, buffer, strlen(buffer), 0); // send buffer to server i.e., process1

    int rows_recvd; // To store count of received records
    // Keep receiving data from process1 and persist them to file.
    while (1) {
        bzero(buffer, BUFSIZE);
        recv(sock, buffer, BUFSIZE, 0); // Received records from process1

        // convert the records received as string to json
        json_object *recvd_arr = parse_to_json(buffer);
        // get length of json array to know the count of received records.
        rows_recvd = json_object_array_length(recvd_arr);
        // Send count of received records to server for checksum
        send(sock, &rows_recvd, sizeof(int), 0);

        /* printf("%s\n", buffer); */
        /* printf("-------------------- %d --------------------\n", rows_recvd); */

        // Open file in r+ mode to presist json data
        open_file("r+");
        // Dump json string to file or append if already present
        dump_or_append_to_file(recvd_arr);
        // Clear and deallocate json object
        json_object_put(recvd_arr);
        fclose(fp);
    }
}


int main(int argc, char *argv[]){
    // Initialize Socket
    init_socket();

    // Try connecting each time after sleeping 30sec until successfull.
    while (connect(sock, (struct sockaddr*)&addr, sizeof(addr)) < 0)
        sleep(30);

    printf("Connected to the OS_USER_1.\n");

    // Start data transfer between server and client i.e., process1 and process2 respectively.
    transfer_data();

    close(sock);
    printf("Disconnected from the OS_USER_1\n");
    return 0;
}
