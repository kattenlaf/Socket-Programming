#ifndef LINKED_LIST
#define LINKED_LIST
#include <stdlib.h>

typedef enum MESSAGE_TYPE {
    ALL,
    LOG,
    ERROR,
    DEBUG
} MESSAGE_TYPE;

typedef struct Node {
    char* message;
    struct Node* next;
    MESSAGE_TYPE type;
} Node;

// Linked List for usage as a persistent memory bus for error messages and potentially other details that might be needed multiple places in server
typedef struct List {
    Node* head;
    Node* tail;
    int length;
} List;

List* InitList(void) {
    List* list = malloc(sizeof(List));
    list->head = NULL;
    list->tail = NULL;
    list->length = 0;
    return list;
}

// declarations
// ------------
void AddContextMessage(List* list, char* message, MESSAGE_TYPE type);
void DumpContextMessages(List* list, char* message_body, MESSAGE_TYPE flag);

// Summary
// adds message to 
void AddContextMessage(List* list, char* message, MESSAGE_TYPE type) {
    Node* new_node = malloc(sizeof(Node));
    new_node->message = message;
    new_node->type = type;
    new_node->next = NULL;
    
    if (list->head == NULL && list->tail == NULL) {
        list->head = new_node;
        list->tail = list->head;
    } else {
        list->tail->next = new_node;
        list->tail = list->tail->next;
    }

    list->length++;
}

// Summary
// Adds messages stored in the linked list to the passed in message body
// allows easy concatenation of messages to response message for client
// type - determines what type of messages to add to message_body
void DumpContextMessages(List* list, char* message_body, MESSAGE_TYPE type) {
    if (list->length > 0) {
        Node* current = list->head;
        while (current != NULL) {
            if (type == ALL) {
                strcat(message_body, current->message);
            }
            // Only dump messages back to client or wherever if the flag matches the error type
            else if (current->type == type) {
                strcat(message_body, current->message);
            }
            current = current->next;
        }
    }
}

#endif
