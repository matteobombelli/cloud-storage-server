#ifndef SERVER_H
#define SERVER_H

#include <stdint.h>
#include <netint/in.h>
#include <pthread.h>

struct ClientData {
    int32_t socket;
    struct sockaddr_in address;
    uint32_t ip;
    uint32_t port;
    int valid;
    pthread_t thread;
};

// Sets up the server if run without config.txt
void config_server();

// Starts a server on a given port reading from config.txt
void start_server();

// Handles internal client threads
void *hande_client(void *arg);

// Checks user credentials against logins file
// RETURNS username
uint8_t *get_user(uint8_t *filename);

// Gets messages from a client given a client socket
// RETURNS pointer to size uint8_t (char)
uint8_t *get_messages(int32_t socket);

// Copies a given file to the user's directory
// RETURNS 0 on success, -1 on failure
uint8_t store_file(struct ClientData, uint8_t *filename);

// Sends a given file to the client
// RETURNS 0 on success, -1 on failure
uint8_t send_file(struct ClientData, uint8_t *filename);

// Deletes a file in the user's directory
// RETURNS 0 on success, -1 on failure
uint8_t send_file(struct ClientData, uint8_t *filename);

// Sends a given file to the client
// RETURNS 0 on success, -1 on failure
uint8_t send_file(uint32_t client_addr, uint8_t *directory);

// Terminates client thread given client address
void terminate_client(uint32_t client_addr);

// Terminates the server and frees memory
void terminate_server(void);

#endif /* SERVER_H */