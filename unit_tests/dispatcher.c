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
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdlib.h>

char* duplicate_sptr(char *source)
{
    // deep copy a string
    int size = 0;
    for (int i = 0; ; i++)
    {
        if (source[i]) size++;
        else break;
    }
    char *destination = (char *) calloc(size+1, sizeof(char));
    for (int i = 0; i < size; i++) destination[i] = source[i];
    return destination;
}

char** duplicate_dptr(char **source)
{
    // deep copy an array of strings
    int size = 0;
    for (int i = 0; ; i++)
    {
        if (source[i]) size++;
        else break;
    }
    char **destination = (char **) calloc(size+1, sizeof(char*));

    for (int i = 0; i < size; i++) destination[i] = duplicate_sptr(source[i]);
    return destination;
}

void free_sptr(char **source)
{
    // free the space allocated to a string
    free(*source);
}

void free_dptr(char ***source)
{
    // free the space allocated to array of strings
    for (int i = 0; ; i++)
    {
        if ((*source)[i] == 0) break;
        free_sptr(&((*source)[i]));
    }
    free(*source);
}

struct request
{
    char *dll_name;
    char *func_name;
    char **func_args;
};

struct request_node
{
    struct request rqst;
    struct request_node *next;
};

struct request_queue
{
    struct request_node *head;
    struct request_node *tail;
    int size;
    int SIZE_MAX;
};

struct request_queue* request_queue(int size);
char enque_request (struct request_queue *q, struct request *rqst);
struct request deque_request (struct request_queue *q);

struct request_queue* request_queue(int size)
{
    // similar to a constructor for a request_queue -> returns pointer to one after allocating and initializing it
    struct request_queue* rq = (struct request_queue*) malloc(sizeof(struct request_queue));
    rq->SIZE_MAX = size;
    rq->head = rq->tail = 0;
    rq->size = 0;
    return rq;
}

char enque_request (struct request_queue *q, struct request *rqst)
{
    // enqueues a request after checking if the queue is not full
    if (q->size >= q->SIZE_MAX) return 0;

    struct request_node *newNode = (struct request_node *) malloc(sizeof(struct request_node));
    newNode->next = 0;
    newNode->rqst = *rqst;

    if (!q->head) // queue is empty
    {
        q->tail = q->head = newNode;
    }
    else
    {
        q->tail->next = newNode;
        q->tail = newNode;
    }

    (q->size)++;
    return 1;
}

struct request deque_request (struct request_queue *q)
{
    // dequeue if queue is not empty, else log error message
    if (q->size == 0)
    {
        printf("No request pending. Illegal dequeue\n");
        struct request dummy;
        return dummy;
    }
    else (q->size)--;
    if (q->head == q->tail) q->tail = 0; // if queue is now empty
    struct request_node *temp = q->head;
    q->head = q->head->next;
    temp->next = 0;
    struct request rtn = temp->rqst;
    free(temp); // free memory (not freeing the strings, as they'll be deallocated only after dispatcher has used them)
    return rtn;
}

char* itoa(int num, char* str);
long long execute_command(char *command);
void* dispatch(void *arg);

#ifndef SYS_gettid
#error "SYS_gettid unavailable on this system"
#endif

/*
#define errExit(msg) do { perror(msg); exit(EXIT_FAILURE); \
                        } while (0)
*/

int FILES_MAX, SIZE_MAX;
struct request_queue* requests;
pthread_mutex_t mutex;

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

int main()
{
    freopen("/dev/null", "w", stderr);
    
    char val[10];
    itoa(100, val);
    if (!strcmp(val, "100"))
    {
        printf("TEST-1 PASSED\n");
    }
    else
    {
        printf("TEST-1 FAILED\n");
    }

    itoa(0, val);
    if (!strcmp(val, "0"))
    {
        printf("TEST-2 PASSED\n");
    }
    else
    {
        printf("TEST-2 FAILED\n");
    }

    itoa(1, val);
    if (!strcmp(val, "1"))
    {
        printf("TEST-3 PASSED\n");
    }
    else
    {
        printf("TEST-3 FAILED\n");
    }

    if (execute_command("echo 3") == (long long) 3)
    {
        printf("TEST-4 PASSED\n");
    }
    else
    {
        printf("TEST-4 FAILED\n");
    }

    if (execute_command("sagalpreet") == (long long) -1)
    {
        printf("TEST-5 PASSED\n");
    }
    else
    {
        printf("TEST-5 FAILED\n");
    }
}