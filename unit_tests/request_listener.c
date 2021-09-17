#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <stdlib.h>
#include <stdio.h>

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

int main()
{
    char *buffer = "sagal\niit\n9.16\n10\n";
    struct request r = listen_request(buffer);

    if(!strcmp("sagal", r.dll_name))
    {
        printf("TEST-1 PASSED\n");
    }
    else
    {
        printf("TEST-1 FAILED\n");
    }

    if(!strcmp("iit", r.func_name))
    {
        printf("TEST-2 PASSED\n");
    }
    else
    {
        printf("TEST-2 FAILED\n");
    }

    if(!strcmp("9.16", (r.func_args)[0]))
    {
        printf("TEST-3 PASSED\n");
    }
    else
    {
        printf("TEST-3 FAILED\n");
    }

    if(!strcmp("10", (r.func_args)[1]))
    {
        printf("TEST-4 PASSED\n");
    }
    else
    {
        printf("TEST-4 FAILED\n");
    }

    return 0;
}