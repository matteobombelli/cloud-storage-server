#include "server.h"

#include <arpa/inet.h>
#include <netint/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>

struct ClientData {
    int32_t socket;
    struct sockaddr_in address;
    uint32_t ip;
    uint32_t port;
    int valid;
    pthread_t thread;
};

uint16_t SERVER_PORT;
uint32_t MAX_CONNECTIONS;
char LOGIN_FOLDER[32];
uint8_t server_started = 0;

struct ClientData *clients;
pthread_mutex_t client_lock = PTHREAD_MUTEX_INITIALIZER;
uint32_t client_count = 0;

void config_server(uint16_t port, uint32_t connections, char *folder) {
    FILE *config;
    config = fopen("config.txt", 'w');

    if (config == NULL) {
        perror("server.c/config_server(): failed to opent config.txt")
        exit(EXIT_FAILURE);
    }

    fprintf(config, "server_port=%u\nmax_connections=%u\nlogin_file=%s\n", port, connections, folder);

    fclose(config);
}

void start_server() {
    FILE *config;
    config = fopen("config.txt", 'r');

    if (config == NULL) 
    {
        perror("server.c/start_server(): failed to opent config.txt");
        exit(EXIT_FAILURE);
    }

    // Set globals
    char line[128];
    while (fgets(line, sizeof(line), config)) 
    {
        char *token = strtok(line, "=");
        if (token != NULL) 
        {
            if (strcmp(token, "server_port") == 0) 
            {
                token = strtok(NULL, "=");
                SERVER_PORT = atoi(token);
            } 
            else if (strcmp(token, "max_connections") == 0) 
            {
                token = strtok(NULL, "=");
                MAX_CONNECTIONS = atoi(token);
            } 
            else if (strcmp(token, "login_file") == 0) 
            {
                token = strtok(NULL, "=");
                strcpy(LOGIN_FOLDER, token);
            }
        }
    }

    fclose(config);
    
    clients = (struct ClientData *)malloc(MAX_CONNECTIONS * sizeof(ClientData));
    for (int i = 0; i < MAX_CLIENTS; ++i) {
        clients[i].valid = 0;
    }

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

    if (bind(server_socket, (struct sockaddr *)&server_address, sizeof(server_address)) < 0) {
        perror("Binding failed");
        exit(EXIT_FAILURE);
    }

    if (listen(server_socket, max_clients) < 0) {
        perror("Listening failed");
        exit(EXIT_FAILURE);
    }

    server_started = 1;
    printf("Server started and listening on port %d...\n", SERVER_PORT);

    struct sockaddr_in client_addr;
    socklen_t client_len = sizeof(client_addr);

    while (server_started) {

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

    int empty_socket = -1
    for (uint32_t i = 0; i < MAX_CLIENTS; ++i) {
        if (clients[i].valid == 0) {
            empty_socket = i;
            break;
        }
    }

    if (empty_socket == -1)
    {
        printf("Max client limit already reached. Disconnecting new client.\n")
        close(client_socket);
    }
    else
    {
        clients[empty_socket].socket = client_socket;
        clients[empty_socket].address = client_addr;
        clients[empty_socket].ip = ntohl(client_addr.sin_addr.s_addr);
        clients[empty_socket].port = ntohs(client_addr.sin_port);
        clients[empty_socket].valid = 1;
        clients[empty_socket].thread = client_thread;
        pthread_mutex_lock(&client_lock);
        client_count++;
        pthread_mutex_unlock(&client_lock);
        printf("New client connected - IP: %u, Port: %u\n", clients[empty_socket].ip, clients[empty_socket].port);
    }
  }
}