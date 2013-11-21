


#include <assert.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "command_line.h"


int printHelp(){
  printf("usage: ./lisod <HTTP port> <HTTPS port> <log file> <lock file> <www folder> <CGI folder or script name> <private key file> <certificate file>\nHTTP port\n  – the port for the HTTP (or echo) server to listen on\nHTTPS port\n  – the port for the HTTPS server to listen on\nlog file\n  – file to send log messages to (debug, info, error)\nlock file\n  – file to lock on when becoming a daemon process\nwww folder\n  – folder containing a tree to serve as the root of a website\nCGI folder or script name\n  – folder containing CGI programs—each file should be executable;\n  if a file it should be a script where you redirect all /cgi/* URIs to\nprivate key file\n  – private key file path\ncertificate file\n  – certificate file path\n");
  return -1;
}



int parseCommandLine(int argc, char*argv[], command_line_s *commandLine){
  
  char input[36];
  int inputInt;

  if (argc != 8)
    return printHelp();
  
  //Log file
  if (strlen(argv[1]) >= 35)
      printf("Log file too large\n");
  else if (sscanf(argv[1],"%s",commandLine->logfile) < 1)
    return printHelp();

  //alpha
  if (sscanf(argv[2],"%f", &commandLine->alpha) < 1)
    return printHelp();

  //listen-port
  if (sscanf(argv[3],"%d",&inputInt) < 1){
    return printHelp();
  }
  commandLine->listen_port = htons(inputInt);


  //fake-ip
  if (strlen(argv[4]) >= sizeof(input))
      printf("fake-ip too large\n");
  else if (sscanf(argv[4],"%s",input) < 1){
    return printHelp();
  }
  inet_pton(AF_INET, input, &commandLine->fake_ip);
  // memset(input, 0, sizeof(input));

  //dns-ip
  if (strlen(argv[5]) >= sizeof(input))
      printf("dns-ip too large\n");
  else if (sscanf(argv[5],"%s",input) < 1){
    return printHelp();
  }  
  printf("FASKEIP: %s\n" ,input);
  inet_pton(AF_INET, input, &commandLine->dns_ip);


  //dns_port
  if (sscanf(argv[6],"%d",&inputInt) < 1){
    commandLine->dns_port = htons(inputInt);
    return printHelp();
  }   
  commandLine->dns_port = htons(inputInt);
  
  //www-ip
  if (strlen(argv[7]) >= sizeof(input))
    printf("www-ip too large\n");
  else if (sscanf(argv[7],"%s",input) < 1){
    return printHelp();
  }
  inet_pton(AF_INET, input, &commandLine->www_ip);
      
  return 0;
}
