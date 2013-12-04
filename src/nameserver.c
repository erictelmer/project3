
#include <netinet/in.h>
#include <netinet/ip.h>                          
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <signal.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/types.h>

#include "dnsMessaging.h"

#define BUFLEN 1500
#define PORT 8000
#define IPLEN 36
#define PATHLEN 64

typedef struct ipstruct{
  char ipstr[IPLEN];
  struct ipstruct *next;
}ip_s;


typedef struct cline{
  int r;
  char log[PATHLEN];
  char ipstr[IPLEN];
  unsigned short port;
  char servers[PATHLEN];
  char LSAs[PATHLEN];
}command_line_s;

void printHelp(){
  printf("Improper usage\n");
}

int parseCommandLine(int argc, char *argv[], command_line_s *commandLine){
  //Check NULLS
  int r;
  if ((argc < 6) || (argc > 7)){
    printHelp();
    return -1;
  }

  if (strcmp(argv[1], "-r") == 0){
    commandLine->r = 1;
    r = 1;
    if (argc != 7){
      printHelp();
      return -1;
    }
  }
  else{
    if (argc != 6){
      printHelp();
      return -1;
    }
    commandLine->r = 0;
    r = 0;
  }
  strncpy(commandLine->log, argv[1 + r], PATHLEN);
  strncpy(commandLine->ipstr, argv[2 + r], IPLEN);
  commandLine->port = atoi(argv[3 + r]);
  strncpy(commandLine->servers, argv[4 + r], PATHLEN);
  strncpy(commandLine->LSAs, argv[5 + r], PATHLEN);
  return 0;
}

ip_s *newIp_s(){
  ip_s * new = malloc(sizeof(struct ipstruct));
  new->next = NULL;
  memset(new->ipstr, 0, IPLEN);
  return new;
}

void freeIps(ip_s *ips){
  ip_s *next;
  
  while(ips != NULL){
    next = ips->next;
    free(ips);
    ips = next;
  }
}

int getIpsFromFile(char *filename, ip_s *ips){
  FILE *file;
  char line[IPLEN];
  int readbytes;
  ip_s *prev = ips;

  if ((filename == NULL) || (ips==NULL)){
    printf("Recieved NULL\n");
    return -1;
  }

  file = fopen(filename, "r");
  if (file == NULL){
    printf("file NULL\n");
    return -1;
  }
  
  while(fgets(line, sizeof(line), file) != NULL)
    {
      strncpy(ips->ipstr, line, IPLEN);
      if (ips->next == NULL)
	ips->next = newIp_s();
      prev = ips;
      ips = ips->next;
    }
  //if we didn't read any ips
  if (prev == ips)
    return -1;
  //the last node won't have anything in it
  freeIps(ips);
  prev->next = NULL;
  return 0;
}

int main(int argc, char * argv[])
{

  int listener;
  struct sockaddr_in addr;
  struct sockaddr_in from;
  socklen_t addrlen;

  fd_set readfds;
  int nfds;
  struct timeval tv;

  char buf[BUFLEN];
  int readret;

  ip_s *ips = newIp_s();
  command_line_s commandLine;
  /*if (parseCommandLine(argc, argv, &commandLine) < 0)
    return -1;
  if(getIpsFromFile(commandLine.servers, ips) < 0)
  return -1;*/
  
  addrlen = sizeof(addr);
  FD_ZERO(&readfds);

  if ((sizeof(int) != 4) || (sizeof(short) != 2) || (sizeof(void*) != 8)){
    printf("Unexpected sizes...UHOH\n");
  }

  if ((listener = socket(AF_INET, SOCK_DGRAM, 0)) < 0){
    perror("socket");
    exit(4);
  }

  bzero(&addr, addrlen);
  addr.sin_family = AF_INET;
  addr.sin_port = htons(9000);
  addr.sin_addr.s_addr = htonl(INADDR_ANY);

  if(bind(listener, (struct sockaddr *)&addr, addrlen) < 0){
    perror("bind");
    exit(4);
  }

  //printf("Server Initiated\nBound2: %s\n%d\n", inet_ntoa(addr.sin_addr), ntohs(addr.sin_port));
  //flush(stdout);

  while(1) {
    
    FD_SET(listener, &readfds);
    tv.tv_sec = 2;

    nfds = select(listener+1, &readfds, NULL, NULL, &tv);
    if (nfds == 0){
      printf("I'm waiting...\n");
      fflush(stdout);
      continue;
    }
    
    printf("About to recieve\n");
    readret = recvfrom(listener, buf, BUFLEN, 0, (struct sockaddr *) &from, &addrlen);
    printf("Recieved\n");

    if (readret > 0){
      printf("\nRecieved:\n%s", buf);
    }

  }
  
  freeIps(ips);

  return 0;
}
