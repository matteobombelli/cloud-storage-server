#include "server.h"
#include <arpa/inet.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <inttypes.h>
#include <dirent.h>
#include <fcntl.h>
#include <sys/stat.h>

struct ClientInfo {
    int32_t socket_fd;
    struct sockaddr_in address;
    uint32_t ip;
    uint16_t port;
    int valid;
    pthread_t thread;
};

uint16_t SERVER_PORT;
uint32_t MAX_CLIENTS;
char *login_folder;
uint32_t SERVER_SOCKET;
int server_started = 0;


uint32_t empty_socket = 0;
pthread_mutex_t client_lock = PTHREAD_MUTEX_INITIALIZER;
uint32_t client_count = 0;
struct ClientInfo *clients;

void config_server(uint16_t port, uint32_t connections, char *folder) {
    FILE *fptr = fopen("config.txt", "w");

    if (fptr == NULL) {
        perror("Error opening file");
        exit(EXIT_FAILURE);
    }

    fprintf(fptr, "port=%" PRIu16 "\n", port); // Use PRIu16 for uint16_t
    fprintf(fptr, "max_connections=%" PRIu32 "\n", connections); // Use PRIu32 for uint32_t
    fprintf(fptr, "login_folder=%s\n", folder);

    fclose(fptr);
}

void start_server() {
    // Extract from config.txt
    FILE *config = fopen("config.txt", "r");

    if (config == NULL) {
        perror("Error opening file");
        exit(EXIT_FAILURE);
    }

    login_folder = (char *)malloc(256 * sizeof(char)); // Assuming a maximum length for login_folder

    // Use proper format specifiers for reading uint16_t and uint32_t
    fscanf(config, "port=%" SCNu16 "\nmax_connections=%" SCNu32 "\nlogin_folder=%s\n", &SERVER_PORT, &MAX_CLIENTS, login_folder);

    fclose(config);

    // Initialize client array
    clients = (struct ClientInfo *)malloc(MAX_CLIENTS * sizeof(struct ClientInfo));

    for (int i = 0; i < MAX_CLIENTS; ++i) {
        clients[i].valid = 0;
    }

    // Initialize server
    int32_t server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket == -1) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }
    SERVER_SOCKET = server_socket;

    struct sockaddr_in server_address;
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = INADDR_ANY;
    server_address.sin_port = htons(SERVER_PORT);

    // Bind server address and socket
    if (bind(server_socket, (struct sockaddr *)&server_address, sizeof(server_address)) < 0) {
        perror("Binding failed");
        exit(EXIT_FAILURE);
    }

    // Start listening on socket
    if (listen(SERVER_SOCKET, MAX_CLIENTS) < 0) {
        perror("Listening failed");
        exit(EXIT_FAILURE);
    }

    server_started = 1;
    printf("Server started and listening on port %d...\n", SERVER_PORT);

    struct sockaddr_in client_addr;
    socklen_t client_len = sizeof(client_addr);

    while (server_started) {
        if (client_count >= MAX_CLIENTS) {
            printf("Maximum client limit reached. Rejecting new connections.\n");

            int32_t client_socket = accept(server_socket, (struct sockaddr *)&client_addr, &client_len);
            if (client_socket < 0) {
                perror("Accept failed");
                continue;
            }

            close(client_socket);
            printf("Rejected new client connection.\n");
        }

        int32_t client_socket = accept(server_socket, (struct sockaddr *)&client_addr, &client_len);
        if (client_socket < 0) {
            perror("Accept failed");
            exit(EXIT_FAILURE);
        }

        pthread_t client_thread;
        int32_t *new_client = (int32_t *)malloc(sizeof(int32_t));
        *new_client = client_socket;

        if (pthread_create(&client_thread, NULL, handle_client, (void *)new_client) != 0) {
            perror("Failed to create client thread");
            free(new_client);
            continue;
        }

        for (uint32_t i = 0; i < MAX_CLIENTS; ++i) {
            if (clients[i].valid == 0) {
                empty_socket = i;
                break;
            }
        }

        pthread_mutex_lock(&client_lock);
        clients[empty_socket].socket_fd = client_socket;
        clients[empty_socket].address = client_addr;
        clients[empty_socket].ip = ntohl(client_addr.sin_addr.s_addr);
        clients[empty_socket].port = ntohs(client_addr.sin_port);
        clients[empty_socket].valid = 1;
        clients[empty_socket].thread = client_thread;
        client_count++;
        pthread_mutex_unlock(&client_lock);

        printf("New client connected - IP: %u, Port: %u\n", clients[empty_socket].ip, clients[empty_socket].port);
    }

    terminate_server();
}

int list_files(char *directory, int client_socket) {
    DIR *dir = opendir(directory);
    if (dir == NULL) {
        perror("Error opening directory");
        return -1;
    }

    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) {
        // Construct the full path of the file
        char filepath[256];  // Adjust the buffer size according to your needs
        snprintf(filepath, sizeof(filepath), "%s/%s", directory, entry->d_name);

        // Get information about the file
        struct stat file_info;
        if (stat(filepath, &file_info) == -1) {
            perror("Error getting file information");
            continue;
        }

        // Check if it's a regular file
        if (S_ISREG(file_info.st_mode)) {
            printf("%s\n", entry->d_name);
        }
    }

    closedir(dir);
    return 0;
}


int receive_file(char *directory, int client_socket) {
    char filename[256];  // Adjust the buffer size according to your needs

    // Get the filename from the client
    ssize_t bytes_received = recv(client_socket, filename, sizeof(filename) - 1, 0);

    if (bytes_received <= 0) {
        perror("Error receiving filename");
        return -1;
    }

    // Construct the full path of the file
    char filepath[256];  // Adjust the buffer size according to your needs
    snprintf(filepath, sizeof(filepath), "%s/%s", directory, filename);

    // Open the file for writing
    int fd = open(filepath, O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
    if (fd == -1) {
        perror("Error opening file for writing");
        return -1;
    }

    // Receive and write file data
    char buffer[1024];
    ssize_t bytes_read;
    while ((bytes_read = recv(client_socket, buffer, sizeof(buffer), 0)) > 0) {
        write(fd, buffer, bytes_read);
    }

    // Close the file
    close(fd);

    return 0;
}

int send_file(char *directory, int client_socket) {
    char filename[256];  // Adjust the buffer size according to your needs

    // Get the filename to send from the client
    ssize_t bytes_received = recv(client_socket, filename, sizeof(filename) - 1, 0);

    if (bytes_received <= 0) {
        perror("Error receiving filename");
        return -1;
    }

    // Construct the full path of the file
    char filepath[256];  // Adjust the buffer size according to your needs
    snprintf(filepath, sizeof(filepath), "%s/%s", directory, filename);

    // Open the file for reading
    int fd = open(filepath, O_RDONLY);
    if (fd == -1) {
        perror("Error opening file for reading");
        return -1;
    }

    // Read and send file data
    char buffer[1024];
    ssize_t bytes_read;
    while ((bytes_read = read(fd, buffer, sizeof(buffer))) > 0) {
        send(client_socket, buffer, bytes_read, 0);
    }

    // Close the file
    close(fd);

    return 0;
}

int delete_file(char *directory, int client_socket) {
    char filename[256];  // Adjust the buffer size according to your needs

    // Get the filename to delete from the client
    ssize_t bytes_received = recv(client_socket, filename, sizeof(filename) - 1, 0);

    if (bytes_received <= 0) {
        perror("Error receiving filename for deletion");
        return -1;
    }

    // Construct the full path of the file
    char filepath[256];  // Adjust the buffer size according to your needs
    snprintf(filepath, sizeof(filepath), "%s/%s", directory, filename);

    // Attempt to delete the file
    if (remove(filepath) != 0) {
        perror("Error deleting file");
        return -1;
    }

    return 0;
}


void terminate_server() {
    // Close server socket
    if (close(SERVER_SOCKET) == -1) {
        perror("Error closing server socket");
        exit(EXIT_FAILURE);
    }

    // Close client sockets
    for (uint32_t i = 0; i < MAX_CLIENTS; ++i) {
        if (clients[i].valid) {
            send(clients[i].socket_fd, "quit", 4, 0);
            if (close(clients[i].socket_fd) == -1) {
                perror("Error closing client socket");
                exit(EXIT_FAILURE);
            }
        }
    }

    // Free allocated memory
    free(login_folder);
    free(clients);

    // Destroy the client lock
    if (pthread_mutex_destroy(&client_lock) != 0) {
        perror("Error destroying client lock");
        exit(EXIT_FAILURE);
    }

    printf("Server terminated.\n");
    exit(EXIT_SUCCESS);
}


char* access_client_directory(const char *username) {
    char client_dir[256];  // Adjust the buffer size according to your needs
    snprintf(client_dir, sizeof(client_dir), "./clients/%s", username);

    // Create the client directory if it doesn't exist
    if (mkdir(client_dir, 0777) == -1) {
        perror("Error creating client directory");
        return NULL;
    }

    return strdup(client_dir);  // Return the dynamically allocated directory name
}

void *handle_client(void *arg) {
    int32_t client_socket = *((int32_t *)arg);
    free(arg);

    char login[256];  // Adjust the buffer size according to your needs
    memset(login, 0, sizeof(login));  // Initialize the login buffer

    // Ask the client for login information
    ssize_t bytes_received = recv(client_socket, login, sizeof(login) - 1, 0);

    if (bytes_received <= 0) {
        perror("Error receiving login information");
        close(client_socket);
        return NULL;
    }

    // Open the login file
    FILE *login_file = fopen(login_folder, "r");
    if (login_file == NULL) {
        perror("Error opening login file");
        close(client_socket);
        return NULL;
    }

    char line[256];
    int found = 0;

    // Check if the received login is in the login file
    while (fgets(line, sizeof(line), login_file) != NULL) {
        if (strstr(line, login) != NULL) {
            found = 1;
            break;
        }
    }

    fclose(login_file);

    // Send the result back to the client
    if (found) {
        send(client_socket, "Login successful\n", 17, 0);
        printf("Client login successful\n");

        // Create a directory for the client based on a simple hash of the username
        char *client_dir = create_client_directory(login);

        // Receive input from the client in a while loop
        char client_input[256];
        while (recv(client_socket, client_input, sizeof(client_input) - 1, 0) > 0) {
            // Process the received input
            if (strcmp(client_input, "list") == 0) {
                list_files(client_dir, client_socket);
            }
            else if (strcmp(client_input, "upload") == 0) {
                receive_file(client_dir, client_socket);
            }
            else if (strcmp(client_input, "download") == 0) {
                send_file(client_dir, client_socket);
            }
            else if (strcmp(client_input, "delete") == 0) {
                delete_file(client_dir, client_socket);
            }
            else if (strcmp(client_input, "quit") == 0) {
                send(client_socket, "quit", 4, 0);
                break;
            }
            else {
                send(client_socket, "invalid", 7, 0);
            }
        }
    } else {
        send(client_socket, "login_failed", 12, 0);
        printf("Client login failed\n");
    }

    close(client_socket);

    pthread_mutex_lock(&client_lock);
    for (uint32_t i = 0; i < MAX_CLIENTS; ++i) {
        if (clients[i].socket_fd == client_socket) {
            clients[i].valid = 0;
            break;
        }
    }
    client_count--;
    pthread_mutex_unlock(&client_lock);
    return NULL;
}