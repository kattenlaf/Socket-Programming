#ifndef DATABASE_OP
#define DATABASE_OP
#include "linkedlist.h"
#include "../vendor/sqlite3.h"
#define TABLE_NAME_LEN 256
#define DATABASE_ERROR_LEN 512
#define DATABASE_NAME "TIMBOG_DEV"


char* GetDatabaseError(int rc, sqlite3* db);

/*
 * Arguments:
 *
 *   unused - Ignored in this case, see the documentation for sqlite3_exec -> use json linked list here so we can send response back to user
 *    count - The number of columns in the result set
 *     data - The row's data
 *  columns - The column names
 */
static int callback(void *unused, int col_count, char **data, char **columns)
{
    int idx;

    fprintf(stdout, "There are %d column(s)\n", col_count);
    fflush(stdout);

    for (idx = 0; idx < col_count; idx++) {
        fprintf(stdout, "The data in column \"%s\" is: %s\n", columns[idx], data[idx]);
        fflush(stdout);
    }

    fprintf(stdout, "\n");
    fflush(stdout);

    return 0;
}

void ExecuteQuery(char* query, List* server_message_bus) {
    sqlite3* db;
    int rc = sqlite3_open(DATABASE_NAME, &db);
    if (rc == SQLITE_OK) {
        rc = sqlite3_exec(db, query, callback, NULL, NULL);
        if (rc != SQLITE_OK) {
            char* error = GetDatabaseError(rc, db);
            AddContextMessage(server_message_bus, error, ERROR);
            free(error);
        } else {
            AddContextMessage(server_message_bus, "Successfully applied query\n", LOG);
        }
    } else {
        char* error = GetDatabaseError(rc, db);
        // possibly check the specific error codes
        AddContextMessage(server_message_bus, error, ERROR);
        free(error);
    }
    sqlite3_close(db);
}

/*
 * Arguments:
 *
 *     table_name - resource / table to be updated by the query
 *  client_buffer - full data sent by the client
 */
void AddOrUpdateRow(char* table_name, char* client_buffer) {
    // Check table name valid? - no I don't think that will be necessary, should be handled by db
    // validate if record already exists

    // TODO parse json out of client buffer
    // execute query, handle possible errors
}

char* GetDatabaseError(int rc, sqlite3* db) {
    char* error = malloc(sizeof(char) * DATABASE_ERROR_LEN);
    sprintf(error, "Error opening database with error\n%s\n", sqlite3_errmsg(db));
    return error;
}

// // Take client buffer, parse out the table name and json for the actual query
// char* CreateQuery(char* table, char* client_buffer) {
    
// }

#endif