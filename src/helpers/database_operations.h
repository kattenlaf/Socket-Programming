#ifndef DATABASE_OP
#define DATABASE_OP
#include <json-c/json.h>
#include <ctype.h>
#include "src/helpers/linkedlist.h"
#include "database_constants.h"
#include "../vendor/sqlite3.h"
#define TABLE_NAME_LEN 256
#define DATABASE_ERROR_LEN 512
#define DATABASE_NAME "TIMBOG_DEV.db"
#define JSON_LEN 2048
#define JSON_KEY_LEN 64
#define JSON_VALUE_LEN 512
#define ERROR_MSG 256
#define QUERY_LEN 512
#define SERVER_DATABASE_LOG 32

struct json_object* ParseJson(char* buffer);
char* GetDatabaseError(int rc, sqlite3* db);
void LogDatabaseError(int rc, sqlite3* db, List* server_message_bus);
bool OpenDatabase(sqlite3* db, List* server_message_bus);
void AddRow(char* table_name, char* client_buffer, List* server_message_bus);
void InitDatabase();
bool DropTable(char* table_name);
TABLES GetDatabaseTable(char* table_name);
char* BuildQuery(TABLES table, struct json_object* json, QUERY_TYPE query_type, void* primary_key);
char* BuildQueryForInsert(TABLES table, struct json_object* json);
char* BuildQueryForSelect(TABLES table, void* primary_key);
char* BuildQueryForDelete(TABLES table, void* primary_key);
void GetRow(char* table_name, List* server_message_bus);

/*
 * Arguments:
 *
 *   unused - Ignored in this case, see the documentation for sqlite3_exec -> use json linked list here so we can send response back to user
 *    count - The number of columns in the result set
 *     data - The row's data
 *  columns - The column names
 */
// https://stackoverflow.com/questions/31146713/sqlite3-exec-callback-function-clarification
static int callback(void* ptr_server_msg_bus, int col_count, char **data, char **columns)
{
    List* server_message_bus = (List*)ptr_server_msg_bus;
    int idx;
    fprintf(stdout, "There are %d column(s)\n", col_count);
    fflush(stdout);
    struct json_object* jobject = json_object_new_object();;
    

    for (idx = 0; idx < col_count; idx++) {
        fprintf(stdout, "The data in column \"%s\" is: %s\n", columns[idx], data[idx]);
        fflush(stdout);
        char key[JSON_KEY_LEN];
        sprintf(key, "\"%s\"", pc[idx]);
        switch (idx) {
            int pokedex_num, type1, type2;
            case (int) POKEDEX_NUM:
                // Segmentation fault here, fix tomorrow....
                pokedex_num = strtol(data[idx], NULL, 10);
                json_object_object_add(jobject, key, json_object_new_int(pokedex_num));
                break;
            case (int) NAME:
                json_object_object_add(jobject, key, json_object_new_string(data[idx]));
                break;
            case (int) TYPE1:
                type1 = strtol(data[idx], NULL, 10);
                json_object_object_add(jobject, key, json_object_new_string(pokemon_types[type1]));
                break;
            case (int) TYPE2:
                type2 = strtol(data[idx], NULL, 10);
                json_object_object_add(jobject, key, json_object_new_string(pokemon_types[type2]));
                break;
        }
    }

    fprintf(stdout, "\n");
    fflush(stdout);
    // https://gist.github.com/alan-mushi/19546a0e2c6bd4e059fd
    char json_string[JSON_VALUE_LEN];
    sprintf(json_string, "\n%s", json_object_to_json_string_ext(jobject, (JSON_C_TO_STRING_SPACED | JSON_C_TO_STRING_PRETTY)));
    AddContextMessage(server_message_bus, json_string, RESPONSE);
    json_object_put(jobject); // Delete the json object

    return 0;
}

bool DropTable(char* table_name) {
    sqlite3* db;
    int rc = sqlite3_open(DATABASE_NAME, &db);
    if (rc == SQLITE_OK) {
        char sql[QUERY_LEN];
        sprintf(sql, "DROP TABLE IF EXISTS %s;", table_name);
        rc = sqlite3_exec(db, sql, callback, NULL, NULL);
        return rc == SQLITE_OK;
    }
    return false;
}

void InitDatabase() {
    sqlite3* db;
    int rc;
    rc = sqlite3_open(DATABASE_NAME, &db);
    if (rc == SQLITE_OK) {
        if (DropTable("POKEMON")) {
            char* sql = "CREATE TABLE POKEMON(" \
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
    } else {
        LogDatabaseError(rc, db, NULL);
    }
    sqlite3_close(db);
}

void ExecuteQuery(char* query, List* server_message_bus) {
    sqlite3* db = NULL;
    int rc = 0;
    rc = sqlite3_open(DATABASE_NAME, &db);
    if (rc == SQLITE_OK) {
        rc = sqlite3_exec(db, query, callback, server_message_bus, NULL);
        if (rc != SQLITE_OK) {
            // Abort due to constraint violation error
            LogDatabaseError(rc, db, server_message_bus);
        } else {
            AddContextMessage(server_message_bus, "\nquery successful.\n", LOG);
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
        AddContextMessage(server_message_bus, "\nquery successful.\n", LOG);
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
void AddRow(char* table_name, char* client_buffer, List* server_message_bus) {
    struct json_object* json = ParseJson(client_buffer);
    char server_log[SERVER_DATABASE_LOG];
    server_log[0] = '\0';
    if (json == NULL) {
        snprintf(server_log, 27, "Error parsing json object\n");
        AddContextMessage(server_message_bus, server_log, ERROR);
    } else {
        snprintf(server_log, 23, "\nSuccess Parsing Json\n");
        AddContextMessage(server_message_bus, server_log, LOG);
        // https://www.youtube.com/watch?v=dQyXuFWylm4&t=260s&ab_channel=HathibelagalProductions

        TABLES table = GetDatabaseTable(table_name);
        if (table != NONE) {
            // Build and execute query using json
            char* sql = BuildQuery(table, json, DATABASE_INSERT, NULL);
            ExecuteQuery(sql, server_message_bus);
        }
    }
}

/*
 * Arguments:
 *
 *     table_name - resource / table to be updated by the query
 *  client_buffer - full data sent by the client
 */
void GetRow(char* table_name, List* server_message_bus) {
    TABLES table = GetDatabaseTable(table_name);
    if (table != NONE) {
        // to test, when implemented parsing primary key from request, this is where the value will go
        // int primary_key = 1;
        // void* ptr = &primary_key;
        char* sql = BuildQuery(table, NULL, DATABASE_SELECT, NULL);
        ExecuteQuery(sql, server_message_bus);
    }
}

// Driver for building query
// should take primary key as a parameter
char* BuildQuery(TABLES table, struct json_object* json, QUERY_TYPE query_type, void* primary_key) {
    void* ptr = primary_key;
    char* sql = (char*)malloc(QUERY_LEN);
    sql[0] = '\0';
    switch(query_type) {
        case DATABASE_UPDATE:
        case DATABASE_INSERT:
            sql = BuildQueryForInsert(table, json);
            break;
        case DATABASE_SELECT:
            sql = BuildQueryForSelect(table, ptr);
            break;
        case DATABASE_DELETE:
            sql = BuildQueryForDelete(table, ptr);
            break;
    }
    return sql;
}

char* BuildQueryForInsert(TABLES table, struct json_object* json) {
    char* sql = (char*)malloc(QUERY_LEN);
    switch(table) {
        case POKEMON: ;
            Pokemon pokemon = GetPokemonFromJson(json);
            sprintf(sql, "INSERT INTO %s (%s, %s, %s, %s) VALUES (%d, '%s', %d, %d);",\
                         tables[table], pc[POKEDEX_NUM], pc[NAME], pc[TYPE1], pc[TYPE2],\
                         pokemon.pokedex_num, pokemon.name, (int) pokemon.type1, (int) pokemon.type2);
            break;
        case GYM_LEADERS:
            sprintf(sql, "SELECT * FROM GYMLEADERS;");
        default:
        sprintf(sql, "SELECT * FROM POKEMON;");
    }
    return sql;
}

char* BuildQueryForSelect(TABLES table, void* primary_key) {
    char* sql = (char*)malloc(QUERY_LEN);
    int pokedex_num;
    switch(table) {
        case POKEMON:
            if (primary_key == NULL) {
                snprintf(sql, 26, "SELECT * FROM %s;", tables[table]);
                break;
            } else {
                pokedex_num = *(int*)primary_key;
                sprintf(sql, "SELECT * FROM %s WHERE %s = %d;", tables[table], pc[POKEDEX_NUM], pokedex_num);
                break;
            }
        case GYM_LEADERS:
            snprintf(sql, 16 + strlen(tables[table]), "SELECT * FROM %s;", tables[table]);
            break;
        case NONE:
            break;  
    }
    return sql;
}

char* BuildQueryForDelete(TABLES table, void* primary_key) {
    char* sql = (char*)malloc(QUERY_LEN);
    int pokedex_num;
    switch(table) {
        case POKEMON:
            pokedex_num = *(int*)primary_key;
            sprintf(sql, "DELETE * FROM %s WHERE %s = %d", tables[table], pc[POKEDEX_NUM], pokedex_num);
            break;
        case GYM_LEADERS:
            snprintf(sql, 14 + strlen(tables[table]), "DELETE FROM %s;", tables[table]);
            break;
        case NONE:
            break;  
    }
    return sql;
}

TABLES GetDatabaseTable(char* table_name) {
    int i = 0;
    int result = -1;
    while (table_name[i]) {
        table_name[i] = tolower(table_name[i]);
        i++;
    }
    for (i = 0; i < TABLE_LEN; i++) {
        result = strcmp(tables[i], table_name);
        if (result == 0) {
            return (TABLES) i;
        }
    }
    return NONE;
}

// Parse json to get details needed to update
// TODO add functionality later to parse out the id from the request as well if possible for post request
json_object* ParseJson(char* buffer) {
    const char* current_line = buffer;
    bool read_message_body = false;
    char* json_string = (char*)malloc(JSON_LEN);
    json_string[0] = '\0'; // Clear string
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
                strcat(json_string, temp_current_line);
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
    free(json_string);
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

#endif