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

int main()
{
    struct request_queue *q = request_queue(5);
    struct request* r = (struct request*) malloc(sizeof(struct request));
    for (int i = 0; i < 5; i++) enque_request(q, r);
    

    if (q->size == 5)
    {
        printf("TEST-1 PASSED\n");
    }
    else
    {
        printf("TEST-1 FAILED\n");
    }

    char done = enque_request(q, r);
    if (q->size == 5)
    if (q->size == 5 && (!done))
    {
        printf("TEST-2 PASSED\n");
    }
    else
    {
        printf("TEST-2 FAILED\n");
    }

    for (int i = 0; i < 5; i++)
    {
        struct request dr = deque_request(q);
    }

    if (q->size == 0)
    {
        printf("TEST-3 PASSED\n");
    }
    else
    {
        printf("TEST-3 FAILED\n");
    }
    
    free(r);

    return 0;
}