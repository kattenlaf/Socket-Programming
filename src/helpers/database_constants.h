#ifndef DATABASE_CONSTS
#define DATABASE_CONSTS
#include <json-c/json.h>

#define TABLE_LEN 2
#define POKEMON_COLUMNS 4
#define SUCCESSFUL_QUERY "\n\nQuery Successful\n"

// Tables
typedef enum TABLES {
    NONE = -1,
    GYM_LEADERS = 0,
    POKEMON = 1
} TABLES;

// Pokemon Columns
typedef enum PC {
    POKEDEX_NUM,
    NAME,
    TYPE1,
    TYPE2
} PC;

typedef enum POKEMON_TYPE {
    NON_TYPE,
    GRASS = 1,
    WATER,
    FIRE,
    POISON = 5
} POKEMON_TYPE;

// Sync with TABLES to get the table name to update
char* tables[] = {"gymleaders", "pokemon"};
const char* pokemon_types[] = {"null", "grass", "water", "fire", "flying", "poison"};
const char* pc[] = {"pokedex_num", "name", "type1", "type2"}; // pokemon columns
const char* pcUpper[] = {"POKEDEX_NUM", "NAME", "TYPE1", "TYPE2"}; // pokemon columns uppercase

// definitions for pokemon table
typedef struct Pokemon {
    int pokedex_num;
    const char* name;
    POKEMON_TYPE type1;
    POKEMON_TYPE type2;
} Pokemon;

typedef enum QUERY_TYPE {
    DATABASE_SELECT,
    DATABASE_UPDATE,
    DATABASE_INSERT,
    DATABASE_DELETE
} QUERY_TYPE;

Pokemon GetPokemonFromJson(struct json_object* json);


Pokemon GetPokemonFromJson(struct json_object* json) {
    Pokemon pokemon;
    for (int i = 0; i < POKEMON_COLUMNS; i++) {
        struct json_object* jobject;
        json_object_object_get_ex(json, pc[i], &jobject);
        switch(i) {
            case (int)POKEDEX_NUM:
                pokemon.pokedex_num = json_object_get_int(jobject);
                break;
            case (int)NAME:
                pokemon.name = json_object_get_string(jobject);
                break;
            case (int)TYPE1:
                // In future, handle strings being sent here2
                pokemon.type1 = (POKEMON_TYPE) json_object_get_int(jobject);
                break;
            case (int)TYPE2:
                pokemon.type2 = (POKEMON_TYPE) json_object_get_int(jobject);
                break;
        }
    }
    return pokemon;
}

#endif