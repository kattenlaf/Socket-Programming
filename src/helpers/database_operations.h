#ifndef DATABASE_OP
#define DATABASE_OP
#include "linkedlist.h"
#include "../vendor/sqlite3.h"

void ExecuteQuery(char* database_name, char* url_query, List* server_message_bus) {
    sqlite3* db;
    int rc = sqlite3_open(database_name, &db);
    if (rc == SQLITE_OK) {
        // TODO maybe introduce database messages
        AddContextMessage(server_message_bus, "Database successfully opened\n", LOG);

        // parse query from url, translate to sqlite query and execute
    } else {
        // possibly check the specific error codes
        char sqlite3_error_msg[SERVER_MSG];
        sprintf(sqlite3_error_msg, "Error opening database with error\n%s\n", sqlite3_errmsg(db));
        AddContextMessage(server_message_bus, sqlite3_error_msg, ERROR);
    }
    sqlite3_close(db);
}

#endif