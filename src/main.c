#include "server.h"
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>

int main(int argc, char *argv[])
{
  if (argc != 1)
  {
    printf("Usage: ./server\nCheck 'config.txt' for server settings\n");
    exit(EXIT_FAILURE);
  }

  if (access("config.txt", F_OK) != 0) {
    // config.txt does not exist
    config_server();
  }
  
  start_server();

  exit(EXIT_SUCCESS);
}