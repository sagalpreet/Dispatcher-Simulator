#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
// #include "client.h"

int main(int argc, char const *argv[])
{
    const int PORT = atoi(argv[1]);

    int sock = 0, valread;
    struct sockaddr_in serv_addr;
    
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        printf("Socket connection could not be established\n");
        exit(-1);
    }
    
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);

    if (inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0)
    {
        printf("inet_pton error encountered\n");
        exit(-1);
    }
    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    {
        printf("Could not connect to server\n");
        exit(-1);
    }

    char *client_name = "client";
    if (argc > 2) client_name = argv[2];

    char message[1024] = {0}, response, c;
    int index = 0;
    while ((c = getc(stdin)) != EOF)
        message[index++] = c;
    message[index] = 0;

    send(sock, message, strlen(message), 0);
    valread = read(sock, &response, sizeof(char));

    if (response) printf("%s's request queued\n", client_name);
    else printf("%s's request rejected\n", client_name);

    return 0;
}