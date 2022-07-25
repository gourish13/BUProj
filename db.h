#include <mysql/mysql.h>

#define DBHOST "localhost"
#define DBUSER "root"
#define DBPASS "123456"
#define DBNAME "employeedb"

void finish_with_error(MYSQL *con);
MYSQL_RES * fetch_rows(MYSQL *con, long rows_to_fetch);
long get_db_rows_count(MYSQL *con);
void print_rows(MYSQL_RES *result);
MYSQL * connect_db(void);
void close_db_connection(MYSQL *con);
int convert_to_json(MYSQL_RES *result, char *json_str);
