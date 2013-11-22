


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
  printf("\nusage: ./proxy <log> <alpha> <listen-port> <fake-ip> <dns-ip> <dns-port> <www-ip>\n\n\
log: The file path to which you should log the messages to.\n\n\
alpha: A float in the range [0, 1]. Uses this as the coefficient in your EWMA throughput estimate.\n\n\
listen-port: The TCP port your proxy should listen on for accepting connections from your browser.\n\n\
fake-ip: Your proxy should bind to this IP address for outbound connections to the webservers.\n\n\
dns-ip: IP address of the DNS server.\n\n\
dns-port: UDP port DNS server listens on.\n\n\
www-ip: [optional] Your proxy should accept an optional argument specifying the IP address of the web server from which it should request video chunks.\n");
    return -1;
}



int parseCommandLine(int argc, char*argv[], command_line_s *commandLine){
  
  char input[36];
  int inputInt;

  if (argc < 7 || argc > 8)
    return printHelp();
  
  //Log file
  if (strlen(argv[1]) >= 35)
      printf("Log file too large\n");
  else if (sscanf(argv[1],"%s",commandLine->logfile) < 1)
    return printHelp();

  //alpha
  if (sscanf(argv[2],"%f", &commandLine->alpha) < 1)
    return printHelp();
  if (commandLine->alpha < 0 || commandLine->alpha > 1)
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
  if(inet_pton(AF_INET, input, &commandLine->fake_ip) == 0)
    return printHelp();
  // memset(input, 0, sizeof(input));

  //dns-ip
  if (strlen(argv[5]) >= sizeof(input))
      printf("dns-ip too large\n");
  else if (sscanf(argv[5],"%s",input) < 1){
    return printHelp();
  }  
  if (inet_pton(AF_INET, input, &commandLine->dns_ip) == 0)
    return printHelp();


  //dns_port
  if (sscanf(argv[6],"%d",&inputInt) < 1){
    commandLine->dns_port = htons(inputInt);
    return printHelp();
  }   
  commandLine->dns_port = htons(inputInt);


  
  //www-ip
  if (argc == 8){
    if (strlen(argv[7]) >= sizeof(input))
      printf("www-ip too large\n");
    else if (sscanf(argv[7],"%s",input) < 1){
      return printHelp();
    }
  }
  /*else {
    struct addrinfo hints, *res;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    
    getaddrinfo("video.cs.cmu.edu", "port", &hints, &res);

  }*/
  if(inet_pton(AF_INET, input, &commandLine->www_ip) == 0)
    return printHelp();
      
  return 0;
}
