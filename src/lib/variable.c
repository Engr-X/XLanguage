#include "lib/variable.h"
#include "lib/utils.h"

void variable_init(struct variable_table* table)
{
    table -> map = NULL;
}

void variable_add(struct variable_table* table, const char* name, const char* type, bool is_const)
{
    struct variable* v = NULL;

    HASH_FIND_STR(table -> map, name, v);

    if (v == NULL)
    {
        v = (struct variable*)(malloc(sizeof(struct variable)));
        v -> is_const = is_const;
        *(v -> name) = '\0';
        *(v -> type) = '\0'; 

        strcpy(v -> name, name);
        strcpy(v -> type, type);

        HASH_ADD_STR(table -> map, name, v);
    }
    else
    {
        // todo
    }
}

struct variable* variable_get(const struct variable_table* table, const char* name)
{
    struct variable* v = NULL;
    HASH_FIND_STR(table -> map, name, v);
    return v;
}

void variable_remove(struct variable_table* table, const char* name)
{
    struct variable* v = NULL;
    HASH_FIND_STR(table -> map, name, v);
    HASH_DEL(table -> map, v);
    free(v);
}

void variable_print(const struct variable_table* table)
{
    int i = 0;

    struct variable* v;

    for (v = table -> map; v != NULL; v = (struct variable*)(v -> hh.next), i++)
        printf("%d: %s %s : %s\n", i, v -> is_const ? "const" : "", v -> name, v -> type);
}

void variable_clear(struct variable_table* table)
{
    struct variable* v;
    struct variable* temp;

    HASH_ITER(hh, table -> map, v, temp)
    {
        HASH_DEL(table -> map, v);
        free(v);
    }
    table -> map = NULL;
}

bool variable_contain(const struct variable_table* table, const char* name)
{
    struct variable* v = NULL;
    HASH_FIND_STR(table -> map, name, v);
    return v != NULL;
}