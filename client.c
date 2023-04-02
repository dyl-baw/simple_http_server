#include <arpa/inet.h>
#include <fcntl.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <unistd.h>

#define BUFFER_SIZE 1024

int main(int argc, char *argv[]) {
    if (argc < 6) {
        printf("Usage: %s <HEADER|GET|POST> <IP_ADDRESS> <SERVER_PORT> <file_path|-> <endpoint> <custom_header>\n", argv[0]);
        return 1;
    }

    int sockfd;
    struct sockaddr_in server_addr;
    struct hostent *host;
    char buffer[BUFFER_SIZE];
    char *method = argv[1];
    char *server_address = argv[2];
    int server_port = atoi(argv[3]);
    char *file_path = argv[4];
    char *endpoint = argv[5];
    char *custom_header = argv[6];

    if (strcmp(method, "GET") != 0 && strcmp(method, "POST") != 0 && strcmp(method, "HEADER") != 0) {
        printf("Error: Invalid method. Use HEADER, GET, or POST.\n");
        return 1;
    }

    host = gethostbyname(server_address);
    if (host == NULL) {
        perror("Error: Unable to get host by name.");
        return 1;
    }

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("Error: Unable to create socket.");
        return 1;
    }

    bzero((char *)&server_addr, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(server_port);
    memcpy(&server_addr.sin_addr, host->h_addr, host->h_length);

    if (connect(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Error: Unable to connect.");
        return 1;
    }

    if (strcmp(method, "POST") == 0) {
        int file_fd = open(file_path, O_RDONLY);
        if (file_fd < 0) {
            perror("Error: Unable to open file.");
            return 1;
        }

        struct stat file_stat;
        fstat(file_fd, &file_stat);

        snprintf(buffer, BUFFER_SIZE,
                 "POST %s HTTP/1.0\r\n"
                 "Content-Length: %ld\r\n"
                 "Content-Type: application/octet-stream\r\n"
                 "%s\r\n"
                 "\r\n",
                 endpoint, file_stat.st_size, custom_header);

        send(sockfd, buffer, strlen(buffer), 0);

        ssize_t bytes_read;
        while ((bytes_read = read(file_fd, buffer, BUFFER_SIZE)) > 0) {
            send(sockfd, buffer, bytes_read, 0);
        }

        close(file_fd);
    } else if (strcmp(method, "GET") == 0) {
        snprintf(buffer, BUFFER_SIZE,
                 "GET %s HTTP/1.0\r\n"
                 "%s\r\n"
                 "\r\n",
                 endpoint, custom_header);

        send(sockfd, buffer, strlen(buffer), 0);
    } else
    {
        snprintf(buffer, BUFFER_SIZE,
                 "HEADER %s HTTP/1.0\r\n"
                 "%s\r\n"
                 "\r\n",
                 endpoint, custom_header);

        send(sockfd, buffer, strlen(buffer), 0);
    }

    ssize_t bytes_received = recv(sockfd, buffer, BUFFER_SIZE -1, 0);
    if (bytes_received > 0)
    {
        buffer[bytes_received] = '\0';
        printf("Server Response: \n %s \n ", buffer);
    }

    close(sockfd);
    return(EXIT_SUCCESS);
}
