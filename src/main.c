#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

#ifdef _WIN32
#include <windows.h>
#define PATH_SEPARATOR '\\'
#else
#include <unistd.h>
#include <limits.h>
#define PATH_SEPARATOR '/'
#endif

#include "lib/utils.h"
#include "lib/file_helper.h"
#include "lib/variable.h"

#include "core/func_convert.h"
#include "core/codegen.h"

typedef struct file_node
{
    char* path;
    struct file_node* next;
} FileNode;

typedef struct file_list
{
    struct file_node* head;
    struct file_node* tail;
} FileList;

static void file_init(struct file_list* list)
{
    list -> head = NULL;
    list -> tail = NULL;
}

static void file_add(struct file_list* list, const char* path)
{
    if (list -> head == NULL)
    {
        struct file_node* node = (struct file_node*)(malloc(sizeof(struct file_node)));
        node -> path = utils_new_string(strlen(path) + 16);
        strcpy(node -> path, path);
        node -> next = NULL;
        list -> head = node;
        list -> tail = node;
    }
    else
    {
        struct file_node* node = (struct file_node*)(malloc(sizeof(struct file_node)));
        node -> path = utils_new_string(strlen(path) + 16);
        strcpy(node -> path, path);
        node -> next = NULL;
        list -> tail -> next = node;
        list -> tail = node;
    }
}

bool get_xlang_home(char* home_dst)
{
    const char* env = getenv("XLANG_HOME");

    if (env && env[0] != '\0')
    {
        strcpy(home_dst, env);
        return true;
    }
    else
    {
        char exe_path[1024];
        where("xlang", exe_path);
        char* p = strrchr(exe_path, PATH_SEPARATOR);

        if (!p)
            return 0;

        *p = '\0';
        p = strrchr(exe_path, PATH_SEPARATOR);
        *p = '\0';

        strcpy(home_dst, exe_path);
        return true;
    }

    return false;
}

int main(int argc, char const *argv[])
{
    srand(time(NULL));
    
    char gcc_path[1024];
    int found = where("gcc", gcc_path);

    if (!found)
    {
        printf_err("GCC not found in PATH.\n");
        return 1;
    }

    char xlang_home[1024];
    get_xlang_home(xlang_home);
    printf("xlang_home: %s\n", xlang_home);

    struct file_list* file_list = (struct file_list*)(malloc(sizeof(struct file_list)));
    file_init(file_list);

    char* input = utils_new_string(65536);
    char* c_code = utils_new_string(65536);

    for (int i = 1; i < argc; i++)
    {
        char* arg = (char*)(argv[i]);
        uint64_t index = utils_string_indexof(arg, ".x");

        if (index != (uint64_t)(-1))
        {
            file_read(arg, "UTF-8", input);
            printf("convert file: %s\n", arg);
            codegen_generate_c_code(input, c_code);
            arg[index + 1] = 'c'; 
            file_add(file_list, arg);
            file_write(arg, c_code, false);
        }
    }

    // run c
    char* command = utils_new_string(4096);

    #if defined(_WIN32) || defined(_WIN64)
        sprintf(command, "gcc -I\"%s/include\"", xlang_home);
    #else
        sprintf(command, "gcc -I\"%s/include\"", xlang_home);
    #endif

    for (int i = 1; i < argc; i++)
    {
        strcat(command, " ");
        strcat(command, argv[i]);
    }

    char* lib_cmd = utils_new_string(4096);
    #if defined(_WIN32) || defined(_WIN64)
        sprintf(lib_cmd, " -L\"%s/lib\" -lnative_io -lnative_std", xlang_home);
    #else
        sprintf(lib_cmd, " -L\"%s/lib\" -lnative_io -lnative_std", xlang_home);
    #endif
        strcat(command, lib_cmd);

    //printf("run: %s\n", command);
    utils_cmd(command);
    printf("compile file end!\n");

    //delete .c
    struct file_node* p = file_list -> head;

    while (p != NULL)
    {
        //file_delete(p -> path);
        free(p -> path);
        struct file_node* temp = p;
        p = p -> next;
        free(temp);
    }
    
    free(input);
    free(c_code);
    free(command);
    free(lib_cmd);
    free(file_list);
    puts("done.");
    return 0;
}