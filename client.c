#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#define BUFFER_SIZE 4096
#define DEFAULT_PORT 80


void send_request(int sock, const char *method, const char *path, const char *server) {
    char buffer[BUFFER_SIZE];

    if (strcmp(method, "GET") == 0 || strcmp(method, "HEAD") == 0)
        {
        snprintf(buffer, BUFFER_SIZE,
                 "%s %s HTTP/1.0\r\n"
                 "Server: %s\r\n"
                 "User-Agent: Custom Client\r\n"
                 "Accept: */*\r\n"
                 "\r\n",
                 method, path, server);
        send(sock, buffer, strlen(buffer), 0);

        while (recv(sock, buffer, BUFFER_SIZE - 1, 0) > 0)
        {
            printf("%s", buffer);
        }
    } else if (strcmp(method, "POST") == 0)
        {
        FILE *file = fopen(path, "rb");
        if (!file) {
            perror("Error opening file");
            exit(EXIT_FAILURE);
        }

        fseek(file, 0, SEEK_END);
        size_t file_size = ftell(file);
        fseek(file, 0, SEEK_SET);

        char *file_contents = malloc(file_size);
        fread(file_contents, 1, file_size, file);
        fclose(file);

        snprintf(buffer, BUFFER_SIZE,
                 "POST %s HTTP/1.0\r\n"
                 "Server: %s\r\n"
                 "User-Agent: Custom Client\r\n"
                 "Content-Type: application/octet-stream\r\n"
                 "Content-Length: %zu\r\n"
                 "\r\n",
                 path, server, file_size);
        send(sock, buffer, strlen(buffer), 0);
        send(sock, file_contents, file_size, 0);

        free(file_contents);
    }
}


int main(int argc, char *argv[])
{
    if (argc < 2) {
        fprintf(stderr, "Usage: %s [GET|HEAD|POST] <path> [Server IP] [port]\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    const char *method = argv[1];
    const char *path = argc > 2 ? argv[2] : "/"; // if argc is greater than 2, set path to 2. Otherwise, it is set to /
    const char *server_ip = argc > 3 ? argv[3] : "127.0.0.1"; // if argc is greater than 3, if so, set server_ip to argv[3]
    int port = argc > 4 ? atoi(argv[4]) : DEFAULT_PORT; // if argc is > than we set the integer value of port to argv[4], otherwise it is set to DEFAULT_PORT

    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0)
    {
        perror("Error creating socket");
        exit(EXIT_FAILURE);
    }

    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    server_addr.sin_addr.s_addr = inet_addr(server_ip);

    if (connect(sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        perror("Error connecting to server");
        exit(EXIT_FAILURE);
    }

    send_request(sock, method, path, server_ip);

    close(sock);
    return 0;
}
