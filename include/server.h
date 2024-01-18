#ifndef SERVER_H
#define SERVER_H

#include <stdint.h>

// Sets up config.txt
void config_server(uint16_t port, uint32_t connections, char *folder);

// Starts a server on a given port reading from config.txt
void start_server();

// Handles internal client threads
void *handle_client(void *arg);

// Lists the given files in the user's directory
// RETURNS 0 on success, -1 on failure
int list_files(char *directory, int client_socket);

// Copies a given file to the user's directory
// RETURNS 0 on success, -1 on failure
int receive_file(char *directory, int client_socket);

// Sends a given file to the client
// RETURNS 0 on success, -1 on failure
int send_file(char *directory, int client_socket);

// Deletes a file in the user's directory
// RETURNS 0 on success, -1 on failure
int delete_file(char *directory, int client_socket);

// // Terminates the server and frees memory
void terminate_server(void);

#endif /* SERVER_H */