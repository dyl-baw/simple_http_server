#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <getopt.h>
#include <unistd.h>
#include <ndbm.h>
#include <stdio.h>


#define MAX_CONNECTIONS 5
#define DEFAULT_PORT 80
#define BUFFER_SIZE 1024

int socket_fd;

void create_socket()
{
    socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_fd == -1)
    {
        perror("Socket creation error");
        exit(EXIT_FAILURE);
    }
}

void bind_socket(int server_fd, const char *ip_address, int port)
{
    struct sockaddr_in address;
    memset(&address, 0, sizeof(address));

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = (ip_address == NULL) ? INADDR_ANY : inet_addr(ip_address);
    address.sin_port = htons(port);

    if (bind(server_fd, (struct sockaddr *) &address, sizeof(address)) == -1)
    {
        perror("Binding socket failed.");
        exit(EXIT_FAILURE);
    }
}


void listen_socket(int server_fd)
{
    if (listen(server_fd, MAX_CONNECTIONS) == -1)
    {
        perror("Listening failed");
        exit(EXIT_FAILURE);
    }
}

int get_file_from_db(const char *path, char **data, size_t *data_len)
{
    DBM *db = dbm_open("file_database", O_RDONLY, 0644);
    if (db == NULL)
    {
        perror("Failed to open ndbm database");
        return -1;
    }

    datum key = {.dptr = (char *) path, .dsize = strlen(path) + 1};
    datum value = dbm_fetch(db, key);

    if (value.dptr != NULL)
    {
        *data_len = value.dsize;
        *data = malloc(value.dsize);
        if (*data == NULL)
        {
            perror("Failed to allocate memory");
            dbm_close(db);
            return -1;
        }
        memcpy(*data, value.dptr, value.dsize);
        dbm_close(db);
        return 0;
    }

    dbm_close(db);
    return -1;
}

void store_file_to_db(const char *path, const char *data, size_t data_len)
{
    DBM *db = dbm_open("file_database", O_CREAT | O_RDWR, 0644);
    if (db == NULL)
    {
        perror("Failed to open ndbm database");
        return;
    }

    datum key = {.dptr = (char *) path, .dsize = strlen(path) + 1};
    datum value = {.dptr = (char *) data, .dsize = data_len};

    int result = dbm_store(db, key, value, DBM_REPLACE);
    if (result != 0)
    {
        perror("Failed to store file in ndbm database");
    }

    dbm_close(db);
}


void handle_request(int client_fd)
{
    char buffer[BUFFER_SIZE];
    ssize_t bytes_received = recv(client_fd, buffer, BUFFER_SIZE - 1, 0);
    if (bytes_received > 0)
    {
        buffer[bytes_received] = '\0';
        printf("Client request: %s\n", buffer);

        char method[16];
        char path[256];
        sscanf(buffer, "%s %s", method, path);

        if (strcmp(method, "GET") == 0) {
            // Remove the leading '/' from the path
            memmove(path, path + 1, strlen(path));

            char *file_content = NULL;
            size_t content_length = 0;
            int result = get_file_from_db(path, &file_content, &content_length);

            if (result == 0 && file_content != NULL) {
                char *content_type;

                // Determine the content type based on the file extension
                if (strstr(path, ".html") != NULL)
                {
                    content_type = "text/html";
                } else if (strstr(path, ".css") != NULL)
                {
                    content_type = "text/css";
                } else if (strstr(path, ".js") != NULL)
                {
                    content_type = "application/javascript";
                } else
                {
                    content_type = "text/plain";
                }

                snprintf(buffer, BUFFER_SIZE,
                         "HTTP/1.0 200 OK\r\n"
                         "Content-Type: %s\r\n"
                         "Content-Length: %zu\r\n"
                         "\r\n",
                         content_type, content_length);
//                send(client_fd, buffer, strlen(buffer), 0);
                send(client_fd, file_content, content_length, 0);

                free(file_content);
            } else {
                snprintf(buffer, BUFFER_SIZE,
                         "HTTP/1.0 404 Not Found\r\n"
                         "Content-Type: text/plain\r\n"
                         "\r\n"
                         "File not found: %s\r\n",
                         path);
                send(client_fd, buffer, strlen(buffer), 0);
            }
    } else if (strcmp(method, "POST") == 0)
        {
            // Remove the leading '/' from the path
            memmove(path, path + 1, strlen(path));

            // Assuming the file content comes after an empty line in the buffer
            char *file_content = strstr(buffer, "\r\n\r\n");
            if (file_content != NULL)
            {
                file_content += 4; // Move pointer to the beginning of the file content
                size_t content_length = buffer + bytes_received - file_content;

                store_file_to_db(path, file_content, content_length);

                snprintf(buffer, BUFFER_SIZE,
                         "HTTP/1.0 200 OK\r\n"
                         "Content-Type: text/plain\r\n"
                         "\r\n"
                         "Stored POST request file: %s\r\n",
                         path);
            } else
            {
                snprintf(buffer, BUFFER_SIZE,
                         "HTTP/1.0 400 Bad Request\r\n"
                         "Content-Type: text/plain\r\n"
                         "\r\n"
                         "Invalid POST request format\r\n");
            }
            send(client_fd, buffer, strlen(buffer), 0);
        } else if (strcmp(method, "HEADER") == 0)
        {
            snprintf(buffer, BUFFER_SIZE,
                     "HTTP/1.0 200 OK\r\n"
                     "Content-Type: text/plain\r\n"
                     "\r\n"
                     "Received HEADER request to: %s\r\n",
                     path);
        } else
        {
            snprintf(buffer, BUFFER_SIZE,
                     "HTTP/1.0 400 Bad Request\r\n"
                     "Content-Type: text/plain\r\n"
                     "\r\n"
                     "Invalid request method: %s\r\n",
                     method);
        }

        send(client_fd, buffer, strlen(buffer), 0);
    }
}

void accept_connection(int server_fd)
{
    int client_fd;
    struct sockaddr_in client_addr;
    socklen_t client_addr_length = sizeof(client_addr);

    while (1)
    {
        client_fd = accept(server_fd, (struct sockaddr *) &client_addr, &client_addr_length);
        if (client_fd == -1)
        {
            perror("Accept failed");
            exit(EXIT_FAILURE);
        }

        handle_request(client_fd);
        close(client_fd);
    }
}

void run_server(int argc, char *argv[])
{
    char *ip_address = "127.0.0.1"; //default to local host if no ip is provided.
    int port = DEFAULT_PORT;
    int opt;

    while ((opt = getopt(argc, argv, "i:p:")) != -1)
    {
        switch (opt)
        {
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

    if (ip_address == NULL)
    {
        fprintf(stderr, "Warning: No IP address provided. Binding to all available interfaces.\n");
    }

    create_socket();
    bind_socket(socket_fd, ip_address, port);
    listen_socket(socket_fd);

    printf("Server running on IP:%s PORT:%d\n", ip_address, port);

    accept_connection(socket_fd);
}

int main(int argc, char *argv[])
{
    run_server(argc, argv);

    return EXIT_SUCCESS;
}

