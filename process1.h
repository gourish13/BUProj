#define IPADDR "127.0.0.1"
#define PORT 5568
#define BUFSIZE 100000
#define CHUNK_COUNT 100
#define SLEEP_TIME 10
#define TRUE 1
#define FALSE 0

void init_socket(void);
int do_checksum(int chars_sent);
void transfer_data(void);
void cleanup(void);
