#include <stdio.h>
#include "request_queue.h"
#include <stdlib.h>
#include <string.h>

struct request listen_request(char* buffer)
{
    struct request message; // to store the request made by client

    FILE *stream = fmemopen(buffer, strlen(buffer), "r");

    message.dll_name = (char *) malloc(1024);
    message.func_name = (char *) malloc(1024);
    message.func_args = (char **) malloc(0);
    
    fgets(message.dll_name, 1024, stream);
    fgets(message.func_name, 1024, stream);

    char container[1024] = {0};
    int sz = 0;
    while (fgets(container, 1024, stream) != 0)
    {
        message.func_args = (char **) realloc(message.func_args, (++sz) * sizeof(char *)); // incrementing size of func_args
        int str_size = strlen(container);

        message.func_args[sz - 1] = (char *) calloc(str_size, 1);
        for (int i = 0; i < str_size; i++) message.func_args[sz - 1][i] = container[i]; // copying function arguments into message struct
        message.func_args[sz - 1][str_size - 1] = 0;
    }
    message.func_args = (char **) realloc(message.func_args, (++sz) * sizeof(char *)); // indicate the end of argument list
    message.dll_name[strlen(message.dll_name)-1] = 0;
    message.func_name[strlen(message.func_name)-1] = 0;
    message.func_args[sz - 1] = 0;

    fclose (stream);

    return message;
}