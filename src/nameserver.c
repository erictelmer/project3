
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

#define BUFLEN 1500
#define PORT 8000

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

  addrlen = sizeof(addr);
  FD_ZERO(&readfds);

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
  

  return 0;
}
