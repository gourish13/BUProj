#define IPADDR "127.0.0.1"
#define PORT 5568
#define BUFSIZE 100000
#define CHUNK_COUNT 100

void init_socket(void);
void do_checksum(int rows_sent);
void transfer_data(void);
void cleanup(void);
