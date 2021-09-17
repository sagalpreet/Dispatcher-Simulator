#include <pthread.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <unistd.h>
#include "request_queue.h"
#include "request_listener.h"
#include "dispatcher.h"

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
int PORT, CONNECTIONS_MAX, THREADS_MAX, FILES_MAX, SIZE_MAX;

struct request_queue* requests;

void* handleConnections(void *arg)
{
    int new_socket = *((int*) arg);
    char buffer[1024] = {0};
    int valread = read(new_socket, buffer, 1024);

    struct request message = listen_request(buffer); // processes raw input from client and converts it into struct request

    pthread_mutex_lock(&mutex);
    char status = enque_request(requests, &message); // attempting to enqueue request
    pthread_mutex_unlock(&mutex);

    send(new_socket, &status, sizeof(char), 0); // send the client confirmation/disapproval depending upon whether the request is queued or not
    close(new_socket);
    free(arg);

    return NULL;
}

void* runServer(void* arg)
{
    struct sockaddr_in address;
    int server_fd, new_socket, opt = 1, addrlen = sizeof(address);

    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0)
    {
        printf("Could not setup socket connection\n");
        exit(EXIT_FAILURE);
    }
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt)))
    {
        printf("Could not setup up server\n");
        exit(EXIT_FAILURE);
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);
    
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0)
    {
        printf("Could not setup server\n");
        exit(EXIT_FAILURE);
    }
    if (listen(server_fd, CONNECTIONS_MAX) < 0) // specify the max number of connections allowed concurrently
    {
        printf("Could not setup server\n");
        exit(EXIT_FAILURE);
    }

    while (1)
    {
        int *new_socket = (int *) malloc(sizeof(int));
        printf("Waiting for new connection ...\n");

        *new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t *)&addrlen); // try to connect to a client

        if ((*new_socket) == -1) // if couldn't connect due to connection limit reached
        {
            printf("Connection limit reached, can't connect\n");
            free(new_socket);
            continue;
        }
        printf("Connection established with socket descriptor %d\n\n", *new_socket);

        pthread_t thread;
        pthread_create(&thread, NULL, handleConnections, new_socket); // connection handling on a different thread
    }

    return NULL;
}

void* runDispatcher(void* arg)
{
    // create specified number of threads and let them perform dispatch operation concurrently
    pthread_t threads[THREADS_MAX];
    for (int i = 0; i < THREADS_MAX; i++) pthread_create(&threads[i], NULL, &dispatch, requests);
    for (int i = 0; i < THREADS_MAX; i++) pthread_join(threads[i], NULL);

    return NULL;
}

int main(int argc, char const *argv[])
{
    // accept command line arguments
    PORT = atoi(argv[1]);
    requests = request_queue(atoi(argv[2]));
    CONNECTIONS_MAX = atoi(argv[3]);
    THREADS_MAX = atoi(argv[4]);
    FILES_MAX = atoi(argv[5]);
    SIZE_MAX = atoi(argv[6]);

    printf("PORT: %d\n", atoi(argv[1]));
    printf("QUEUE SIZE: %d\n", atoi(argv[2]));
    printf("MAX CONNECTIONS: %d\n", CONNECTIONS_MAX);
    printf("MAX THREADS: %d\n", THREADS_MAX);
    printf("MAX FILES ON EACH THREAD: %d\n", FILES_MAX);
    printf("MAX MEMORY ON EACH THREAD: %d kB\n\n", SIZE_MAX);

    // concurrently run server and dispatcher routines
    pthread_t server, dispatcher;

    pthread_create(&server, NULL, &runServer, NULL);
    pthread_create(&dispatcher, NULL, &runDispatcher, NULL);

    pthread_join(server, NULL);
    pthread_join(dispatcher, NULL);

    return 0;
}
