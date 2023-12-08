#include "server.h"
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>

// config.txt defaults
#define DEFAULT_SERVER_PORT 8000
#define DEFAULT_MAX_CONNECTIONS 16
#define DEFAULT_LOGIN_FOLDER "logins.txt"

int main(int argc, char *argv[])
{
  if (argc != 1)
  {
    printf("Usage: ./server\nCheck 'config.txt' for server settings\n");
    exit(EXIT_FAILURE);
  }

  if (access("config.txt", F_OK) != 0) {
    // config.txt does not exist
    config_server(DEFAULT_SERVER_PORT, DEFAULT_MAX_CONNECTIONS, DEFAULT_LOGIN_FOLDER);
  }
  
  start_server();

  exit(EXIT_SUCCESS);
}