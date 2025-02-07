#ifndef DATABASE_OP
#define DATABASE_OP
#include "linkedlist.h"
#include "../vendor/sqlite3.h"
#include "../vendor/cJSON.h"
#define TABLE_NAME_LEN 256
#define DATABASE_ERROR_LEN 512
#define DATABASE_NAME "TIMBOG_DEV.db"
#define JSON_LEN 2048
#define ERROR_MSG 256

// declarations
cJSON* ParseJson(char* buffer);

char* GetDatabaseError(int rc, sqlite3* db);
void LogDatabaseError(int rc, sqlite3* db, List* server_message_bus);
bool OpenDatabase(sqlite3* db, List* server_message_bus);
void AddOrUpdateRow(char* table_name, char* client_buffer, List* server_message_bus);

/*
 * Arguments:
 *
 *   unused - Ignored in this case, see the documentation for sqlite3_exec -> use json linked list here so we can send response back to user
 *    count - The number of columns in the result set
 *     data - The row's data
 *  columns - The column names
 */
// https://stackoverflow.com/questions/31146713/sqlite3-exec-callback-function-clarification
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

void InitDatabase() {
    sqlite3* db;
    int rc;
    if (OpenDatabase(db, NULL)) {
        char* sql = "CREATE TABLE POKEMON(" \
                    "POKEDEX_NUM INT PRIMARY KEY NOT NULL" \
                    "NAME                   TEXT NOT NULL" \
                    "TYPE1                   INT NOT NULL" \
                    "TYPE2                   INT );";
        rc = sqlite3_exec(db, sql, callback, NULL, NULL);
        if (rc != SQLITE_OK) {
            char* error = GetDatabaseError(rc, db);
            print_stderr(error);
            fflush(stderr);
            free(error);
        }
    }
}

void ExecuteQuery(char* query, List* server_message_bus) {
    sqlite3* db;
    int rc;
    if (OpenDatabase(db, server_message_bus)) {
        rc = sqlite3_exec(db, query, callback, NULL, NULL);
        if (rc != SQLITE_OK) {
            LogDatabaseError(rc, db, server_message_bus);
        } else {
            AddContextMessage(server_message_bus, "query successful.\n", LOG);
        }
    } else {
        LogDatabaseError(rc, db, server_message_bus);
    }
    sqlite3_close(db);
}

void ValidateQueryExecution(int rc, sqlite3* db, List* server_message_bus) {
    if (rc != SQLITE_OK) {
        LogDatabaseError(rc, db, server_message_bus);
    } else {
        AddContextMessage(server_message_bus, "query successful.\n", LOG);
    }
}

bool OpenDatabase(sqlite3* db, List* server_message_bus) {
    int rc = sqlite3_open(DATABASE_NAME, &db);
    if (rc != SQLITE_OK) {
        if (server_message_bus != NULL) {
            LogDatabaseError(rc, db, server_message_bus);
        }
        return false;
    }
    return true;
}

/*
 * Arguments:
 *
 *     table_name - resource / table to be updated by the query
 *  client_buffer - full data sent by the client
 */
// TODO FIX COMPILATION ISSUES
void AddOrUpdateRow(char* table_name, char* client_buffer, List* server_message_bus) {
    cJSON* json = ParseJson(client_buffer);
    if (json == NULL) {
        const char* error_ptr = cJSON_GetErrorPtr();
        if (error_ptr != NULL) {
            char error[ERROR_MSG];
            sprintf(error, "Error: %s\n", error_ptr);
            AddContextMessage(server_message_bus, error, ERROR);
        }
        cJSON_Delete(json);
    } else {
        // Access Json Data here
        // https://www.geeksforgeeks.org/cjson-json-file-write-read-modify-in-c/
        // https://www.tutorialspoint.com/sqlite/sqlite_c_cpp.htm
        // https://stackoverflow.com/questions/16900874/using-cjson-to-read-in-a-json-array
    }
}

cJSON* ParseJson(char* buffer) {
    const char* current_line = buffer;
    bool read_message_body = false;
    char* json_string = (char*)malloc(JSON_LEN);
    while(current_line) {
        const char* next_line = strchr(current_line, '\n');
        int current_line_len = next_line ? (next_line-current_line) : strlen(current_line);
        char* temp_current_line = (char*)malloc(current_line_len+1);
        if (temp_current_line) {
            memcpy(temp_current_line, current_line, current_line_len);
            temp_current_line[current_line_len] = '\0';
            if (!read_message_body) {
                read_message_body = strcmp(temp_current_line, '\n');
            } else if (!(strcmp(temp_current_line, '\n'))) {
                // If we know we are reading the message body, we want read each line that is not a new line
                json_string = strcat(json_string, temp_current_line);
            }
        free(temp_current_line);
        } else {
            print_stderr("Error allocating memory parsing json string from client buffer\n");
        }
    }

    cJSON* json = cJSON_Parse(json_string);
    return json;
}

char* GetDatabaseError(int rc, sqlite3* db) {
    char* error = malloc(sizeof(char) * DATABASE_ERROR_LEN);
    sprintf(error, "Error opening database with error\n%s\n", sqlite3_errmsg(db));
    return error;
}

void LogDatabaseError(int rc, sqlite3* db, List* server_message_bus) {
    char* error = GetDatabaseError(rc, db);
    AddContextMessage(server_message_bus, error, ERROR);
    free(error);
}

// // Take client buffer, parse out the table name and json for the actual query
// char* CreateQuery(char* table, char* client_buffer) {
    
// }

#endif