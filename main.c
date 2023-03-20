#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <getopt.h>

int main(int argc, char *argv[]) {
    int opt;
    char *ip_address = NULL;
    int port = 0;
    int client_running = 1;

    while ((opt = getopt(argc, argv, "i:p:")) != -1) {
        switch (opt) {
            case 'i':
                ip_address = optarg;
                break;
            case 'p':
                port = atoi(optarg);
                break;
            default:
                fprintf(stderr, "Usage: %s -i <IP address> -p <port number>\n", argv[0]);
                exit(EXIT_FAILURE);
        }
    }

    if (ip_address == NULL || port == 0) {
        fprintf(stderr, "Usage: %s -i <IP address> -p <port number>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    struct sockaddr_in serv_addr;
    int sock_fd = 0;
    int message_read;
    char buffer[1024];
    char message[1024];

    // Create a socket
    if ((sock_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        printf("\n Socket creation error \n");
        exit(EXIT_FAILURE);
    }

    // Set up the server address structure
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port);

    // Connect to the server
    if (connect(sock_fd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) == -1) {
        printf("\nConnection Failed \n");
        exit(EXIT_FAILURE);
    } else {
        printf("Connected to Server.\n\n");
    }

    while (client_running)
    {
        memset(message, 0, sizeof(message)); // clear the message buffer
        printf("Please enter a message: ");
        fgets(message, 1024, stdin);
        // Send the message to the server
        send(sock_fd, message, strlen(message), 0);
        printf("\nMessage sent\n");

        memset(message, 0, sizeof(message)); // clear the message buffer

        // Receive a response from the server
        message_read = read(sock_fd, buffer, 1024);
        printf("\n%s\n", buffer);
        memset(buffer, 0, sizeof(buffer)); // clear the buffer
        memset(message, 0, sizeof(message)); // clear the message buffer
    }
    // Close the socket
    close(sock_fd);
    return EXIT_SUCCESS;
}