//
// Created by David Lee on 2023-03-27.
//
#include <arpa/inet.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#define SERVER_ADDR "127.0.0.1"
#define SERVER_PORT 8080
#define BUFFER_SIZE 1024

void send_file(FILE *file, int sockfd) {
    char buffer[BUFFER_SIZE];
    size_t bytes_read;

    while ((bytes_read = fread(buffer, 1, BUFFER_SIZE, file)) > 0) {
        send(sockfd, buffer, bytes_read, 0);
    }
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("Usage: %s <file-to-upload>\n", argv[0]);
        exit(1);
    }

    FILE *file = fopen(argv[1], "rb");
    if (!file) {
        perror("fopen");
        exit(1);
    }

    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("socket");
        exit(1);
    }

    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(SERVER_PORT);
    inet_pton(AF_INET, SERVER_ADDR, &server_addr.sin_addr);

    if (connect(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("connect");
        exit(1);
    }

    fseek(file, 0, SEEK_END);
    long file_size = ftell(file);
    fseek(file, 0, SEEK_SET);

    char header[BUFFER_SIZE];
    snprintf(header, sizeof(header),
             "POST /upload HTTP/1.0\r\n"
             "Host: %s:%d\r\n"
             "Content-Type: multipart/form-data; boundary=----WebKitFormBoundary7MA4YWxkTrZu0gW\r\n"
             "Content-Length: %ld\r\n"
             "\r\n"
             "------WebKitFormBoundary7MA4YWxkTrZu0gW\r\n"
             "Content-Disposition: form-data; name=\"file\"; filename=\"%s\"\r\n"
             "Content-Type: application/octet-stream\r\n"
             "\r\n",
             SERVER_ADDR, SERVER_PORT, file_size, argv[1]);

    send(sockfd, header, strlen(header), 0);
    send_file(file, sockfd);
    send(sockfd, "\r\n------WebKitFormBoundary7MA4YWxkTrZu0gW--\r\n", 44, 0);

    fclose(file);
    close(sockfd);
    return 0;
}
