#ifndef SERVER_H
#define SERVER_H

#include <stdint.h>

// Sets up config.txt
void config_server(uint16_t port, uint32_t connections, char *folder);

// Starts a server on a given port reading from config.txt
void start_server();

// Handles internal client threads
void *hande_client(void *arg);

// Checks user credentials against logins file
// RETURNS username
char *get_user(char *login_folder);

// Gets messages from a client given a client socket
// RETURNS char *message
char *get_messages(int32_t socket);

// Copies a given file to the user's directory
// RETURNS 0 on success, -1 on failure
uint8_t store_file(struct ClientData, char *filename);

// Sends a given file to the client
// RETURNS 0 on success, -1 on failure
uint8_t send_file(struct ClientData, char *filename);

// Deletes a file in the user's directory
// RETURNS 0 on success, -1 on failure
uint8_t delete_file(struct ClientData, char *filename);

// Terminates client thread given client address
void terminate_client(uint32_t client_addr);

// Terminates the server and frees memory
void terminate_server(void);

#endif /* SERVER_H */