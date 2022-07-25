#include <json-c/json.h>

#define IPADDR "127.0.0.1"
#define PORT 5568
#define BUFSIZE 100000
#define FILENAME "Employees.json"

void init_socket(void);
json_object * parse_to_json(char *buffer);
void open_file(char *mode);
int file_isempty(void);
void dump_or_append_to_file(json_object *array);
void transfer_data(void);
