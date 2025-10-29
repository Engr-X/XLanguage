#include "lib/function.h"
#include "lib/utils.h"

#define TOLERANCE 16

static void get_hash(const char* name, const char* signature, char* dst)
{
    sprintf(dst, "%s$%s", name, signature);
}

void function_init(struct function_table* table)
{
    table -> map = NULL;
}

void function_add(struct function_table* table, const char* name, const char* signature, const char* return_type)
{
    struct function* v = NULL;

    HASH_FIND_STR(table -> map, name, v);

    if (v == NULL)
    {
        v = (struct function*)(malloc(sizeof(struct function)));
        char* hash_tag = utils_new_string(1024);
        get_hash(name, signature, hash_tag);

        v -> name = utils_new_string(strlen(name) + TOLERANCE);
        v -> return_type = utils_new_string(strlen(return_type) + TOLERANCE);
        v -> signature = utils_new_string(strlen(signature) + TOLERANCE);
        v -> c_name = utils_new_string(TOLERANCE * sizeof(char));
        v -> hash_tag = hash_tag;

        strcpy(v -> name, name);
        strcpy(v -> return_type, return_type);
        strcpy(v -> signature, signature);

        struct function* tmp = NULL;

        while (true)
        {
            utils_random_name(v -> c_name, MAX_C_FUNCTION_LENGTH);

            HASH_FIND_STR(table -> map, v -> c_name, tmp);

            if (tmp == NULL)
                break;
        }
        
        HASH_ADD_STR(table -> map, hash_tag, v);
    }
    else
    {
        // todo error and overload
    }
}

void function_add_native(struct function_table* table, const char* name, const char* c_name, const char* signature, const char* return_type)
{
    struct function* v = NULL;

    HASH_FIND_STR(table -> map, name, v);
    char* hash_tag = utils_new_string(1024);
    get_hash(name, signature, hash_tag);

    v = (struct function*)(malloc(sizeof(struct function)));
    v -> name = utils_new_string(strlen(name) + TOLERANCE);
    v -> return_type = utils_new_string(strlen(return_type) + TOLERANCE);
    v -> signature = utils_new_string(strlen(signature) + TOLERANCE);
    v -> c_name = utils_new_string(128 * sizeof(char));
    v -> hash_tag = hash_tag;

    strcpy(v -> name, name);
    strcpy(v -> return_type, return_type);
    strcpy(v -> signature, signature);
    strcpy(v -> c_name, c_name);
        
    HASH_ADD_STR(table -> map, hash_tag, v);
}

// todo overload
struct function* function_get(const struct function_table* table, const char* name, const char* signature)
{
    struct function* v = NULL;
    char hash[1024];
    get_hash(name, signature, hash);
    HASH_FIND_STR(table -> map, hash, v);
    return v;
}

void function_remove(struct function_table* table, const char* name, const char* signature)
{
    struct function* v = NULL;
    char hash[1024];
    get_hash(name, signature, hash);
    HASH_FIND_STR(table -> map, hash, v);
    
    if (v != NULL)
    {
        free(v -> name);
        free(v -> signature);
        free(v -> return_type);
        free(v -> c_name);
        free(v);
    }
    return;
}

void function_print(const struct function_table* table)
{
    int i = 0;
    struct function* v;

    for (v = table -> map; v != NULL; v = (struct function*)(v -> hh.next), i++)
    {
        printf("%d: %s %s(%s) | ", i, v -> return_type, v -> name, v -> signature);
        printf("%d: %s %s(%s)\n", i, v -> return_type, v -> c_name, v -> signature);
    }
}

void function_clear(struct function_table* table)
{
    struct function* v;
    struct function* temp;

    HASH_ITER(hh, table -> map, v, temp)
    {
        HASH_DEL(table -> map, v);
        free(v -> name);
        free(v -> signature);
        free(v -> return_type);
        free(v -> c_name);
        free(v -> hash_tag);
        free(v);
    }

    table -> map = NULL;
}

bool function_contain(const struct function_table* table, const char* name, const char* signature)
{
    struct function* v = NULL;
    char hash[1024];
    get_hash(name, signature, hash);
    HASH_FIND_STR(table -> map, hash, v);
    return v != NULL;
}

void function_add_default(struct function_table* dst_table)
{
    // x function -> c function -> parameters_type -> return_type
    function_add_native(dst_table, "print", "__native_xl_print_bool", "_bool", "void");
    function_add_native(dst_table, "print", "putchar", "_char", "void");
    function_add_native(dst_table, "print", "__native_xl_print_byte", "_byte", "void");
    function_add_native(dst_table, "print", "__native_xl_print_short", "_short", "void");
    function_add_native(dst_table, "print", "__native_xl_print_int", "_int", "void");
    function_add_native(dst_table, "print", "__native_xl_print_long", "_long", "void");
    function_add_native(dst_table, "print", "__native_xl_print_float", "_float", "void");
    function_add_native(dst_table, "print", "__native_xl_print_double", "_double", "void");
    function_add_native(dst_table, "print", "__native_xl_print_str", "_string", "void");

    function_add_native(dst_table, "println", "__native_xl_println_bool", "_bool", "void");
    function_add_native(dst_table, "println", "__native_xl_println_char", "_char", "void");
    function_add_native(dst_table, "println", "__native_xl_println_byte", "_byte", "void");
    function_add_native(dst_table, "println", "__native_xl_println_short", "_short", "void");
    function_add_native(dst_table, "println", "__native_xl_println_int", "_int", "void");
    function_add_native(dst_table, "println", "__native_xl_println_long", "_long", "void");
    function_add_native(dst_table, "println", "__native_xl_println_float", "_float", "void");
    function_add_native(dst_table, "println", "__native_xl_println_double", "_double", "void");
    function_add_native(dst_table, "println", "puts", "_string", "void");

    // x function -> c function -> parameters_type -> return_type
    function_add_native(dst_table, "newline", "__native_xl_newline", "", "void");

    function_add_native(dst_table, "max", "__native_xl_max_int", "_int_int", "int");
    function_add_native(dst_table, "max", "__native_xl_max_double", "_double_double", "double");

    function_add_native(dst_table, "min", "__native_xl_min_int", "_int_int", "int");
    function_add_native(dst_table, "min", "__native_xl_min_double", "_double_double", "double");

    function_add_native(dst_table, "sum", "__native_xl_sum_3double", "_double_double_double", "double");
    function_add_native(dst_table, "pi", "__native_xl_pi", "", "double");
    function_add_native(dst_table, "get_time", "__native_xl_get_time", "", "long");
}