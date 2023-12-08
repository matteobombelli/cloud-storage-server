#ifndef SERVER_H
#define SERVER_H

#include <stdint.h>

// Starts a server on a given port reading from config.txt
void start_server(uint16_t port);

// Sets up the server if run without config.txt
void config_server();

// Handles internal client threads
void *hande_client(void *arg);

// Checks user credentials against logins file
// RETURNS username
uint8_t *get_user(uint8_t *filename);

// Gets messages from a client given a client socket
// RETURNS pointer to size uint8_t (char)
uint8_t *get_messages(int16_t socket);

// Copies a given file to the client's directory
// RETURNS 0 on success, -1 on failure
uint8_t store_file(uint8_t *filename, uint8_t *directory);

// Terminates client thread given client address
void terminate_client(uint32_t client_ip, uint16_t client_port);

// Terminates the server and frees memory
void terminate_server(void);

#endif /* SERVER_H */