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
