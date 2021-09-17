#define _GNU_SOURCE
#define _FILE_OFFSET_BITS 64
#include <time.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/resource.h>
#include <sys/syscall.h>
#include <fcntl.h>
#include <dlfcn.h>
#include "memory.h"
#include "request_queue.h"
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <errno.h>
#include "dispatcher.h"

#ifndef SYS_gettid
#error "SYS_gettid unavailable on this system"
#endif

/*
#define errExit(msg) do { perror(msg); exit(EXIT_FAILURE); \
                        } while (0)
*/

extern int FILES_MAX, SIZE_MAX;
extern struct request_queue* requests;
extern pthread_mutex_t mutex;

char* itoa(int num, char* str)
{
    // converts an integer into string and stores it in the second argument (char* pointer), also returns the same
    int i = 0;
    if (num == 0)
    {
        str[i++] = '0';
        str[i] = '\0';
        return str;
    }
    while (num != 0)
    {
        int rem = num % 10;
        str[i++] = (rem > 9)? (rem-10) + 'a' : rem + '0';
        num = num/10;
    }
    str[i] = '\0';
    int len = strlen(str);
    for (int i = 0; i < len / 2; i++)
    {
        char temp = str[i];
        str[i] = str[len-i-1];
        str[len-i-1] = temp;
    }
    return str;
}

long long execute_command(char *command)
{
    // executes shell command and returns answer provided its long long
    FILE *fp = popen(command, "r");

    long long result = -1;
    
    if (fp == NULL)
    {
        return result;
    }

    fscanf(fp, "%lld", &result);

    pclose(fp);

    return result;
}

void* dispatch(void *arg)
{
    pid_t tid = (pid_t)syscall(SYS_gettid);

    char location[100] = "../execution_results/";
    char str_tid[20];
    itoa(tid, str_tid);
    strcat(location, str_tid);
    FILE* execution_result = fopen(location, "w");

    // process id to track resource usage
    pid_t pid = getpid();

    char file_command[30] = "ls /proc/";
    char str_pid[10];
    itoa(pid, str_pid);
    strcat(file_command, str_pid);
    strcat(file_command, "/fd | wc -l");
    char memory_command[20] = "ps -o rss= ";
    strcat(memory_command, str_pid);

    struct request_queue* requests = (struct request_queue*) arg;

    while (1)
    {
        // mutex lock on queue before dequeue attempt
        pthread_mutex_lock(&mutex);
        if (requests->size == 0)
        {
            pthread_mutex_unlock(&mutex);
            continue;
        }

        struct request to_service = deque_request(requests);
        pthread_mutex_unlock(&mutex);

        double (*func) (double);

        long long initial_memory = execute_command(memory_command), initial_files = execute_command(file_command); // initial resource consumption
        if (initial_files == -1) initial_files = 999999999;
        if (initial_memory == -1) initial_memory = 999999999;

        void *handle;

        // accessing dynamic library
        handle = dlopen(to_service.dll_name, RTLD_LAZY);
        if (!handle)
        {
            fprintf(execution_result, "%s\n", dlerror());
            fflush(execution_result);
            continue;
        }

        func = dlsym(handle, to_service.func_name);
        char* error;
        if ((error = dlerror()) != NULL)
        {
            fprintf(execution_result, "%s\n", error);
            fflush(execution_result);
            continue;
        }

        double ans = (*func)(atof(to_service.func_args[0]));

        long long final_memory = execute_command(memory_command), final_files = execute_command(file_command); // final resource consumption

        // check if resource consumption surpassed the threshold
        if (final_files - initial_files > FILES_MAX)
        {
            fprintf(execution_result, "OPEN FILES LIMIT EXCEEDED\n");
            fflush(execution_result);
            continue;
        }
        if (final_memory - initial_memory > SIZE_MAX)
        {
            fprintf(execution_result, "MEMORY LIMIT EXCEEDED\n");
            fflush(execution_result);
            continue;
        }

        fprintf(execution_result, "Successfully Executed %s(%s) = %lf\n", to_service.func_name, to_service.func_args[0], ans);

        fflush(execution_result);
        dlclose(handle);

        // memory management: free the memory allocated while creating the request using message struct
        free_sptr(&(to_service.dll_name));
        free_sptr(&(to_service.func_name));
        free_dptr(&(to_service.func_args));
    }

    fclose(execution_result);
}
