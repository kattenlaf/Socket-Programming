#ifndef DATABASE_CONSTS
#define DATABASE_CONSTS
#include <json-c/json.h>

# define TABLE_LEN 2

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
const char* pokemon_types[] = {"null", "Grass", "Water", "Fire", "Flying", "Poison"};
const char* pc[] = {"pokedex_num", "name", "type1", "type2"}; // pokemon columns

// definitions for pokemon table
typedef struct Pokemon {
    int pokedex_num;
    char* name;
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
    // TODO convert to loop
    struct json_object* json_pokedex_num;
    struct json_object* json_name;
    struct json_object* json_type1;
    struct json_object* json_type2;
    json_object_object_get_ex(json, pc[(int)POKEDEX_NUM], &json_pokedex_num);
    json_object_object_get_ex(json, pc[(int)NAME], &json_name);
    json_object_object_get_ex(json, pc[(int)TYPE1], &json_type1);
    json_object_object_get_ex(json, pc[(int)TYPE2], &json_type2);
    Pokemon pokemon;
    pokemon.pokedex_num = json_object_get_int(json_pokedex_num);
    pokemon.name = json_object_get_string(json_name);
    pokemon.type1 = (POKEMON_TYPE) json_object_get_int(json_type1);
    pokemon.type2 = (POKEMON_TYPE) json_object_get_int(json_type2);

    return pokemon;
}

#endif