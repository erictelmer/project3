


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
  if (argc != 8)
    return printHelp();
  //1 log          string
  //2 alpha        float
  //3 listen_port  int
  //4 fake_ip      string
  //5 dns_ip       string
  //6 dns_port     int
  //7 www_ip       string

  if (strlen(argv[1]) >= strlen(commandLine->logfile))
      printf("Log file too large\n");
  else if (sscanf(argv[3],"%s",commandLine->logfile) < 1)
    return printHelp();

  if (sscanf(argv[4],"%f", &commandLine->alpha) < 1)
    return printHelp();

  else if (sscanf(argv[5],"%d",&commandLine->listen_port) < 1)
    return printHelp();

  if (strlen(argv[6]) >= strlen(commandLine->fake_ip))
      printf("fake-ip too large\n");
  else if (sscanf(argv[6],"%s",commandLine->fake_ip) < 1)
    return printHelp();

  if (strlen(argv[7]) >= strlen(commandLine->dns_ip))
      printf("dns-ip too large\n");
  else if (sscanf(argv[7],"%s",commansLine->dns_ip) < 1)
    return printHelp();
  
  else if (sscanf(argv[5],"%d",&commandLine->dns_port) < 1)
    return printHelp(); 

  if (strlen(argv[8]) >= strlen(commandLine->www_ip))
      printf("www-ip too large\n");
  else if (sscanf(argv[8],"%s",commandLine->www_ip) < 1)
    return printHelp();
  
  return 0;
}
