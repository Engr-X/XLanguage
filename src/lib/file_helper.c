#include <iconv.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>

#if defined(_WIN32) || defined(_WIN64)
#include <windows.h>
#include <io.h>
#define PATH_SEPARATOR ';'
#define DIR_SEPARATOR '\\'
#elif defined(__APPLE__) && defined(__MACH__)
#include <mach-o/dyld.h>
#include <unistd.h>
#include <unistd.h>
#define PATH_SEPARATOR ':'
#define DIR_SEPARATOR '/'
#else
#include <unistd.h>
#include <unistd.h>
#define PATH_SEPARATOR ':'
#define DIR_SEPARATOR '/'
#endif

#include "lib/file_helper.h"
#include "lib/utils.h"

void remove_r(char* code)
{
    if (*code == '\0')
        return;

    if (*code != '\r')
        remove_r(code + 1);
    else
    {
        memmove(code, code + 1, strlen(code + 1) + 1);
        remove_r(code);
    }
}

void remove_extension(const char* file_path, char* dst)
{
    if (!file_path || !dst)
        return;

    const uint16_t len = strlen(file_path);
    int slash_index = -1;
    int dot_index = -1;

    for (int i = len - 1; i >= 0; i--)
    {
        if (file_path[i] == '/' || file_path[i] == '\\')
        {
            slash_index = i;
            break;
        }
    }

    for (int i = len - 1; i > slash_index; i--)
    {
        if (file_path[i] == '.')
        {
            dot_index = i;
            break;
        }
    }

    if (dot_index == -1)
        dot_index = len;

    uint16_t from_index = (slash_index == -1) ? 0 : slash_index + 1;
    utils_substring(file_path, from_index, dot_index, dst);
}

bool file_read(const char* path, const char* encoding, char* dst)
{
    FILE* fp = fopen(path, "rb");

    if (!fp)
        return false;

    fseek(fp, 0, SEEK_END);
    uint64_t size = ftell(fp);

    if (size <= 0)
    {
        fclose(fp);
        return false;
    }

    rewind(fp);

    char* src = (char*)(malloc((size + 16) * sizeof(char)));
    
    if (!src)
    {
        fclose(fp);
        return false;
    }

    uint64_t read_bytes = fread(src, 1, size, fp);
    src[read_bytes] = '\0';
    fclose(fp);

    iconv_t cd = iconv_open("UTF-8", encoding);
    
    if (cd == (iconv_t)(-1))
    {
        free(src);
        return false;
    }

    uint64_t inbytesleft = read_bytes;
    uint64_t outbytesleft = size * 4 + 4;
    char* inbuf = src;
    char* outbuf = dst;

    uint64_t result = iconv(cd, &inbuf, &inbytesleft, &outbuf, &outbytesleft);
    
    if (result == (uint64_t)(-1))
    {
        iconv_close(cd);
        free(src);
        return false;
    }

    *outbuf = '\0';

    iconv_close(cd);
    free(src);
    remove_r(dst);
    return true;
}

bool file_write(const char* path, const char* content, bool append)
{
    FILE* fp = fopen(path, append ? "ab" : "wb");

    if (!fp)
        return false;

    const uint64_t len = strlen(content);
    fwrite(content, 1, len, fp);
    fclose(fp);
    return true;
}

bool file_delete(const char* path)
{
    return !remove(path);
}

int where(const char* file_name, char* dst)
{
    if (!file_name || !dst)
        return 0;

    // Check current directory first
    char* cwd = utils_new_string(2048);
    char* full_path = utils_new_string(4096);

    if (getcwd(cwd, 4096) != NULL)
    {
        #ifdef _WIN32
            sprintf(full_path, "%s%c%s.exe", cwd, DIR_SEPARATOR, file_name);
        #else
            sprintf(full_path, "%s%c%s", cwd, DIR_SEPARATOR, file_name);
        #endif
            if (access(full_path, X_OK) == 0)
            {  
                // Use X_OK (1) for executable check; on Windows, adjust if needed (e.g., to 0 for existence)
                strcpy(dst, full_path);
                free(cwd);
                free(full_path);
                return 1;
            }
    }

    const char* path_env = getenv("PATH");

    if (!path_env)
        return 0;

    char* path_env_copy = utils_new_string((strlen(path_env) + 4096) * sizeof(char));
    strcpy(path_env_copy, path_env);

    char* saveptr = NULL;
    char* token = strtok_r(path_env_copy, (char[]){PATH_SEPARATOR, 0}, &saveptr);

    while (token)
    {
        #ifdef _WIN32
            sprintf(full_path, "%s%c%s.exe", token, DIR_SEPARATOR, file_name);
        #else
            sprintf(full_path, "%s%c%s", token, DIR_SEPARATOR, file_name);
        #endif

        if (access(full_path, X_OK) == 0)
        {
            strcpy(dst, full_path);
            free(path_env_copy);
            free(cwd);
            free(full_path);
            return 1;
        }

        token = strtok_r(NULL, (char[]) {PATH_SEPARATOR, 0}, &saveptr);
    }

    free(path_env_copy);
    free(cwd);
    free(full_path);

    return 0;
}