
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

//get Ip by num, starting at 1
ip_s *getIpNum(ip_s *ips, int num){
  int x = 1;
  while((ips != NULL) && (x != num)){
    ips = ips->next;
    x++;
  }
  return ips;
}

int getIpsFromFile(char *filename, ip_s *ips, int *numIps){
  FILE *file;
  char line[IPLEN];
  int readbytes;
  ip_s *prev = ips;

  *numIps = 0;

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
      *numIps = *numIps + 1;
      //remove \n
      line[strlen(line) - 1] = '\0';
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

  int numIps;
  int roundRobin = 1;
  /*if (parseCommandLine(argc, argv, &commandLine) < 0)
    return -1;
  if(getIpsFromFile(commandLine.servers, ips) < 0)
  return -1;*/
  
  addrlen = sizeof(addr);
  FD_ZERO(&readfds);
  parseCommandLine(argc, argv, &commandLine);
  getIpsFromFile(commandLine.servers, ips, &numIps);

  printf("Ip1: %s\n", ips->ipstr);
  printf("Ip2: %s\n", ips->next->ipstr);

  if ((sizeof(int) != 4) || (sizeof(short) != 2) || (sizeof(void*) != 8)){
    printf("Unexpected sizes...UHOH\n");
  }

  if ((listener = socket(AF_INET, SOCK_DGRAM, 0)) < 0){
    perror("socket");
    exit(4);
  }

  char myipstr[IPLEN];

  bzero(&addr, addrlen);
  addr.sin_family = AF_INET;
  addr.sin_port = htons(commandLine.port);
  inet_pton(AF_INET, commandLine.ipstr, &addr.sin_addr);

  addr.sin_addr.s_addr = htonl(INADDR_ANY);

  inet_ntop(AF_INET, &addr.sin_addr, myipstr, 36);
  printf("Ip: %s\n", myipstr);
  printf("Port: %d\n", ntohs(addr.sin_port));

  if(bind(listener, (struct sockaddr *)&addr, addrlen) < 0){
    perror("bind");
    exit(4);
  }

  //printf("Server Initiated\nBound2: %s\n%d\n", inet_ntop(addr.sin_addr), ntohs(addr.sin_port));
  //flush(stdout);

  while(1) {
    
    FD_SET(listener, &readfds);
    tv.tv_sec = 20;

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
      unsigned short ID = getID(buf);
      char *question;
      char *resource;
      char ipstr[IPLEN];
      ip_s *iptosend;
      char ipbits[4];
      unsigned short RDLENGTH = 4;
      unsigned char RCODE = 0;
      unsigned int sendlen = 0;
      question = getEndOfHeader(buf);
      if (getQR(buf) != 0){
	//send RCODE 3?
	continue;
      }
      else if (!isVideoCsCmuEdu(question)){
	//send RCODE 3?
	RCODE = 3;
	memset(buf, 0, BUFLEN);
	fillResponseHeaderTemplate(buf);
	putID(buf, ID);
	putANCOUNT(buf, 0);
	putRCODE(buf, RCODE);
	//continue;
      }
      else{
	//Check OPCODE?
	//Check QDCOUNT?

	//ROUND ROBIN
	//roundRobin > 0
	iptosend = getIpNum(ips, roundRobin);
	roundRobin++;
	if (roundRobin > numIps)
	  roundRobin = 1;
	inet_pton(AF_INET, iptosend->ipstr, ipbits);
	
	memset(buf, 0, BUFLEN);
	fillResponseHeaderTemplate(buf);
	putID(buf, ID);
	//putRCODE
	resource = getEndOfHeader(buf);
	fillResourceRecordTemplate(resource);
	putRDLENGTH(resource, RDLENGTH);
	putRDATA(resource, ipbits, RDLENGTH);
	sendlen = getEndOfResource(resource) - buf;
      }
      sendto(listener, buf, sendlen, 0, (const struct sockaddr *)&from, addrlen);
      //SEND
    }

  }
  
  freeIps(ips);

  return 0;
}
