#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <getopt.h>

#define MAX_CONNECTIONS 5
#define DEFAULT_PORT 80

int socket_fd;
void createSocket()
{
    socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_fd == -1 ) {
        perror("Socket creation error");
        exit(EXIT_FAILURE);
    }
}

void bindSocket(int server_fd, const char *ip_address, int port)
{
    struct sockaddr_in address;
    memset(&address, 0 , sizeof(address));

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = inet_addr(ip_address);
    address.sin_port = htons(port);

    if (bind(server_fd, (struct sockaddr *) &address, sizeof(address)) == - 1)
    {
        perror("Binding socket failed.");
        exit(EXIT_FAILURE);
    }
}

void listenSocket(int server_fd)
{
    if ( listen(socket_fd, MAX_CONNECTIONS) == -1)
    {
        perror("Listening failed");
        exit(EXIT_FAILURE);
    }
}

void acceptConnection(int server_fd)
{
    int client_fd;
    struct sockaddr_in client_addr;
    socklen_t client_addr_length = sizeof(client_addr);

    client_fd = accept(server_fd, (struct sockaddr *) &client_addr, &client_addr_length);
    if (client_fd == -1)
    {
        perror("Accept failed");
        exit(EXIT_FAILURE);
    }
}

void run_server(int argc, char *argv[])
{
    char *ip_address = NULL;
    int port = DEFAULT_PORT;
    int opt;

    while ((opt = getopt(argc, argv, "i:p:")) != -1)
    {
        switch (opt) {
            case 'i':
                ip_address = optarg;
                break;
            case 'p':
                port = atoi(optarg);
                break;
            default:
                fprintf(stderr, "Usage: %s [-i ip address ] [-p port] \n", argv[0]);
                exit(EXIT_FAILURE);
        }
    }
    //if no port is provided, use the default port of 80.
    if (port == 0)
    {
        port = DEFAULT_PORT;
    }

    createSocket();
    bindSocket(socket_fd,ip_address, port);
    listenSocket(socket_fd);

    printf("Server running on IP:%s PORT:%d\n", ip_address, port);

    acceptConnection(socket_fd);
}

int main(int argc, char *argv[])
{
    run_server(argc, argv);

    return EXIT_SUCCESS;
}