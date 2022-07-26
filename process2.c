#include <json-c/json_tokener.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <sys/socket.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <json-c/json.h>
#include <json-c/json_object.h>

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

// Close socket and reinitialize it for reconnecting 
void restart_socket(void) {
    close(sock);
    init_socket();
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


// Buffer will be converted to json and persisted in file.
void persist_received_buffer_to_file(char *buffer) {
    // Open file in r+ mode to presist json data
    open_file("r+");

    // convert the records received as string to json
    json_object *recvd_arr = json_tokener_parse(buffer);

    // Dump json string to file or append if already present
    dump_or_append_to_file(recvd_arr);
    fclose(fp);

    // Clear and deallocate json object
    json_object_put(recvd_arr);
}


// Data transfer between server and client i.e., process1 and process2 respectively.
void transfer_data(void) {
    int retval;
    char buffer[BUFSIZE];
    bzero(buffer, sizeof(buffer)); // Clear buffer
    strcpy(buffer, "waiting for data"); // copy "waiting for data" to buffer
    send(sock, buffer, strlen(buffer), 0); // send buffer to server i.e., process1

    int chars_recvd; // To store count of received chars from process1
    // Keep receiving data from process1 and persist them to file.
    while (True) {
        bzero(buffer, BUFSIZE);
        retval = recv(sock, buffer, BUFSIZE, 0); // Received records from process1
        if (retval == 0) // If recv returns 0 connection lost
            return;      // return and reconnect

        // Get length of buffer to know the size of received records
        chars_recvd = strlen(buffer);
        // Send count of received chars to server for checksum
        send(sock, &chars_recvd, sizeof(int), 0);

        char buf[BUFSIZE];
        bzero(buf, BUFSIZE);
        // Receive Checksum status, if Failed get data from server again else persist to file
        retval = recv(sock, buf, BUFSIZE, 0);
        if (retval == 0) // If recv returns 0 connection lost 
            return;      // return and reconnect

        if (strcmp(buf, "Failed") == 0) // If checksum failed
            continue;              // continue and do the transfer again

        // Checksum passed, persist buffer
        persist_received_buffer_to_file(buffer);
    }
}


int main(int argc, char *argv[]){
    // Initialize Socket
    init_socket();
    
    // Open file and a+ mode and close it so that it will create the file if not exists
    open_file("a+");
    fclose(fp);

    while (True) {
        // Try connecting each time after sleeping until successfull.
        while (connect(sock, (struct sockaddr*)&addr, sizeof(addr)) < 0)
            sleep(SLEEP_TIME);

        printf("Connected to the OS_USER_1.\n");

        // Start data transfer between server and client i.e., process1 and process2 respectively.
        transfer_data();
        // Restart socket connection
        restart_socket();
    }
    return 0;
}
