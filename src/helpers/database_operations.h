#ifndef DATABASE_OP
#define DATABASE_OP
#include "src/helpers/linkedlist.h"
#include "../vendor/sqlite3.h"
#include <json-c/json.h>
#define TABLE_NAME_LEN 256
#define DATABASE_ERROR_LEN 512
#define DATABASE_NAME "TIMBOG_DEV.db"
#define JSON_LEN 2048
#define ERROR_MSG 256

// declarations
json_object* ParseJson(char* buffer);

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
    rc = sqlite3_open(DATABASE_NAME, &db);
    if (rc == SQLITE_OK) {
        char* sql;
        sql = "DROP TABLE POKEMON;";
        rc = sqlite3_exec(db, sql, callback, NULL, NULL);
        sql = "CREATE TABLE POKEMON(" \
                    "POKEDEX_NUM INT PRIMARY KEY NOT NULL," \
                    "NAME                   TEXT NOT NULL," \
                    "TYPE1                   INT NOT NULL," \
                    "TYPE2                   INT );";
        rc = sqlite3_exec(db, sql, callback, NULL, NULL);
        if (rc != SQLITE_OK) {
            char* error = GetDatabaseError(rc, db);
            print_stderr(error);
            fflush(stderr);
            free(error);
        }
    } else {
        LogDatabaseError(rc, db, NULL);
    }
    sqlite3_close(db);
}

void ExecuteQuery(char* query, List* server_message_bus) {
    sqlite3* db = NULL;
    int rc = 0;
    rc = sqlite3_open(DATABASE_NAME, &db);
    if (rc != SQLITE_OK) {
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
    struct json_object* json = ParseJson(client_buffer);
    if (json == NULL) {
        AddContextMessage(server_message_bus, "Error parsing json object\n", ERROR);
    } else {
        AddContextMessage(server_message_bus, "Success Parsing Json\n", LOG);
        // https://www.youtube.com/watch?v=dQyXuFWylm4&t=260s&ab_channel=HathibelagalProductions
        struct json_object* json_name;
        // TODO put database column names in a separate file with macro definitions
        // Parse out each item, build the query and execute it
        // Respond to client if the record was added or not
        json_object_object_get_ex(json, "name", &json_name);
        fprintf(stdout, "Name of pokemon: %s\n", json_object_get_string(json_name));
        fflush(stdout);
    }
}

json_object* ParseJson(char* buffer) {
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
                // 0x7ffffc482 "\n\r\n{\r\n    name: \"bulbasaur\"\r\n}"
                if (temp_current_line[0] == '{') {
                    read_message_body = true;
                }
            }
            if (read_message_body) {
                json_string = strcat(json_string, temp_current_line);
            } 
        free(temp_current_line);
        } else {
            print_stderr("Error allocating memory parsing json string from client buffer\n");
            return NULL;
        }
        current_line = next_line ? (next_line+1) : NULL;
    }

    // Testing, TODO, see why parsing json string sent from request is incorrect
    struct json_object* json;
    FILE* fp;
    char tbuffer[1024];
    fp = fopen("test.json", "r");
    fread(tbuffer, 1024, 1, fp);
    fclose(fp);

    json = json_tokener_parse(json_string);
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