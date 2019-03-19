#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <netdb.h>
#include <arpa/inet.h>
#include "ftree.h"

int main(int argc, char** argv) {
  
  if (argc != 4){
    printf("Usage: rcopy_client [source] [destination] [server_ip]");
    return (1);
  }
  else{
    fcopy_client(argv[1], argv[2], argv[3], PORT);
    
  }

  return 0;
}