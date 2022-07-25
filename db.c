#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <mysql/mysql.h>
#include <json-c/json.h>
#include <json-c/json_object.h>

#include "db.h"

/* Keep track of rows retrieved from database */
long rows_fetched = 0;


/* On error exit cleaning up database connection */
void finish_with_error(MYSQL *con) {
    fprintf(stderr, "Error: %s\n", mysql_error(con));
    close_db_connection(con);
    exit(1);
}


/* Get the current no. of rows in table by executing query */
long get_db_rows_count(MYSQL *con) {
    // Execute the SQL Query to get count of records
    if (mysql_query(con, "SELECT COUNT(*) FROM Employee"))
        finish_with_error(con);

    // Get result pointer for executed query
    MYSQL_RES *result = mysql_store_result(con);
    // if result is null, there is error so print error and exit
    if (result == NULL)
        finish_with_error(con);

    // Get the count of records from result and convert to long
    MYSQL_ROW row = mysql_fetch_row(result);
    long count = strtol(row[0], NULL, 10);

    // Deallocate result pointer and return count of records.
    mysql_free_result(result);
    return count;
}


/* Execute the select query and retrive records as result */
MYSQL_RES * fetch_rows(MYSQL *con, long rows_to_be_fetch) {
    char stmt[100];

    // SQL Statement
    sprintf(stmt, "SELECT \
EmpId, FirstName, LastName, Organization \
FROM Employee LIMIT %ld, %ld", rows_fetched, rows_to_be_fetch);

    // Execute Query
    if (mysql_query(con, stmt))
        finish_with_error(con);

    // Get result pointer i.e., result of executed query
    MYSQL_RES *result = mysql_store_result(con);
    // if result is null, there is error so print error and exit
    if (result == NULL)
        finish_with_error(con);

    // Update rows_fetched to tract no. of retrieved rows and return result
    rows_fetched += mysql_num_rows(result);
    return result;
}

// For DEBUGGING Purpose
/* Print the retrieved rows and check if the rows retrieved *
* from database properly or not */
void print_rows(MYSQL_RES *result) {
    MYSQL_ROW row;
    int num_fields = mysql_num_fields(result); // Get no. of DB columns

    // Loop through DB rows and print each column
    while ((row = mysql_fetch_row(result))) {

        for(int i = 0; i < num_fields; i++)
            printf("%s ", row[i] ? row[i] : "NULL");

        printf("\n");
    }
}


/* Connect to the MySQL database */
MYSQL * connect_db(void) {

    MYSQL *con = mysql_init(NULL);
    // If con is NULL means error initializing MySQL library, so exit with error
    if (con == NULL) {
        fprintf(stderr, "%s\n", mysql_error(con));
    exit(1);
    }

    /* If connction not successfull exit with error */
    if (mysql_real_connect(con, DBHOST, DBUSER, DBPASS, DBNAME,
                           0, NULL, 0) == NULL)
    {
        fprintf(stderr, "%s\n", mysql_error(con));
        close_db_connection(con);
    exit(1);
    }
    // Return con on successfull connection
    return con;
}


/* Close database connection and cleanup allocated memory */
void close_db_connection(MYSQL *con) {
    mysql_close(con);
    mysql_library_end();
}


/* Convert MYSQL result to json schema */
int convert_to_json(MYSQL_RES *result, char *json_str) {
    MYSQL_ROW row;
    json_object *array = json_object_new_array(); // initialize new json array object

    while ((row = mysql_fetch_row(result))) {
        json_object *obj = json_object_new_object(); // initialize new json object

        // Convert each MySQL field value to json string.
        json_object *empid = json_object_new_string(row[0]); 
        json_object *first_name = json_object_new_string(row[1]);
        json_object *last_name = json_object_new_string(row[2]);
        json_object *organization = json_object_new_string(row[3]);

        // Add json string to json object with field name as key.
        json_object_object_add(obj, "EmpId", empid);
        json_object_object_add(obj, "FirstName", first_name);
        json_object_object_add(obj, "LastName", last_name);
        json_object_object_add(obj, "Organization", organization);

        // Add json object to json array
        json_object_array_add(array, obj);
    }
    // Get the length of json array
    int rows_converted = json_object_array_length(array);
    // Convert json array to string and copy it with strcpy.
    strcpy(json_str, json_object_to_json_string(array));
    /* strcpy(json_str, json_object_to_json_string_ext(array , JSON_C_TO_STRING_PRETTY)); */

    // Deallocate json array and objects, then return the length of json array.
    json_object_put(array);
    return rows_converted;
}
