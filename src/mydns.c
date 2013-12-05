#include "mydns.h"

#define REQSIZE 2000

static struct sockaddr_in dnsaddr;
static int sock;

int init_mydns(const char *dns_ip, unsigned int dns_port){
  printf("Dns ip: %s\nDns port: %d\n", dns_ip, dns_port);
  socklen_t addrlen = sizeof(struct sockaddr_in);

  if ((sock = socket(AF_INET, SOCK_DGRAM, 0)) < 0){
    perror("socket");
    return -1;
  }

  bzero(&dnsaddr, addrlen);
  dnsaddr.sin_family = AF_INET;
  dnsaddr.sin_port = htons(dns_port);
  inet_pton(AF_INET, dns_ip, &dnsaddr.sin_addr);

  if(bind(sock, (struct sockaddr *)&dnsaddr, addrlen) < 0){
    perror("bind");
    return -1;
  }

  return 0;
}

int resolve(const char *node, const char *service,
            const struct addrinfo *hints, struct addrinfo **res){
  char req[REQSIZE];
  char cmuName[18] = {0x05, 0x76, 0x69, 0x64, 0x65, 0x6f, 0x02, 0x63, 0x73, 0x03, 0x63, 0x6d, 0x75, 0x03, 0x65, 0x64, 0x75, 0x00};
  unsigned short port;
  unsigned short ID = 42;
  char *question;
  char *answer;
  char ipbits[4];
  unsigned int reqlen;
  struct sockaddr_in from;
  struct addrinfo myhints;
  socklen_t addrlen = sizeof(from);


  if (strcmp(node, "video.cs.cmu.edu") != 0){
    printf("WHAT ARE YOU TRYING TO DO TO ME!!!\n");
  }
  
  port = atoi(service);
  *res = malloc(sizeof(struct addrinfo));
  
  memset(req, 0, REQSIZE);
  fillRequestHeaderTemplate(req);
  putID(req, ID);
  question = getEndOfHeader(req);
  fillQuestionTemplate(question);

  reqlen = getEndOfQuestion(question) - req;

  sendto(sock, req, reqlen, 0, (const struct sockaddr *)&dnsaddr, addrlen);

  memset(req, 0 , REQSIZE);

  reqlen = recvfrom(sock, req, REQSIZE, 0, (struct sockaddr *) &from, &addrlen);

  if (reqlen <= 0)
    return -1;

  if (getID(req) != ID){
    printf("Not == IDs\n");
    return -1;
  }
  
  if (getQR(req) != 1){
    printf("Recieved Request?!\n");
    return -1;
  }

  if (getANCOUNT(req) != 1){
    printf("Recieved response with numAnsweres != 1\n");
    return -1;
  }

  answer = getEndOfHeader(req);
  if (!isVideoCsCmuEdu(answer)){
    printf("Recieved reply with name != video.cs.cmu.edu\n");
    return -1;
  }

  if (getRDLENGTH(answer) != 4){
    printf("RDLENGTH != 4\n");
    return -1;
  }

  getRDATA(answer, ipbits);

  char myipstr[36];
  inet_ntop(AF_INET, &ipbits, myipstr, addrlen);

  memset(&myhints, 0, sizeof(struct addrinfo));
  //myhints.ai_flags = AI_PASSIVE;
  myhints.ai_family = AF_INET;
  myhints.ai_socktype = SOCK_STREAM;

  getaddrinfo(myipstr, service, &myhints, res);

  return 0;
}
