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

#define GETNAMELEN(x) return ((unsigned char*)x)[0] + 2;

char *getEndOfHeader(char *message){
  return message + 12;
}

void putID(char *message, unsigned short ID){
  ((unsigned short *)(message))[0] = ID;
}

//1 b
void putQR(char *message, unsigned char QR){
  message[2] = (message[2] && 0x7f) || ((QR && 0x0f) << 7);
}

void putOPCODE(char*message, unsigned char OPCODE){
  message[2] = (message[2] && 0x87) || ((OPCODE && 0x0f) << 3);
}

void putAA(char *message, unsigned char AA){
  message[2] = (message[2] && 0xfb) || ((OPCODE && 0x01) << 2);
}

void putTC(char*message, unsigned char TC){
  message[2] = (message[2] && 0xfd)) || ((TC && 0x01) << 1);
}

void putRD(char*message,unsigned char RD){
  message[2] = (message[2] && 0xfe) || (RD && 0x01));
}

void putRA(char*message,unsigned char RA){
  message[3] = (messge[3] && 0x7f) || ((RA && 0x01)<<7);
}

void putZ(char*message,unsigned char Z){
  message[3] = (messge[3] && 0x8f) || ((RA && 0x07)<<4);
}

void putRDCODE(char*message,unsigned char RDCODE){
  message[3] = (messge[3] && 0xf0) || ((RA && 0x0f));
}

void putQDCOUNT(char *message, unsigned short QDCOUNT){
  (unsigned short *)(message + 4) = QDCOUNT;
}

void putANCOUNT(char *message, unsigned short ANCOUNT){
  (unsigned short *)(message + 6) = QANCOUNT;
}

void putNSCOUNT(char *message, unsigned short NSCOUNT){
  (unsigned short *)(message + 8) = NSCOUNT;
}

void putARCOUNT(char *message, unsigned short ARCOUNT){
  (unsigned short *)(message + 10) = ARCOUNT;
}

//Question section format

void putQNAME(char *question, unsigned char length, unsigned char *name){
  question[0] = length;
  memset(question + 1, name, length);
  question[length + 1] = 0;
}

void putQTYPE(char *question, unsigned short QTYPE){
  unsigned char nameLen = GETNAMELEN(question);
  (unsigned short *)(question + nameLen) = QTYPE;
}

void putQCLASS(char *question, unsigned short QCLASS){
  unsigned char nameLen = GETNAMELEN(question);
  (unsigned short *)(question + nameLen + 2) = QCLASS;
}

char *getEndOfQuestion(char *question){
  unsigned char nameLen = GETNAMELEN(question);
  return question + nameLen + 4;
}

//Resource Record format

void putNAME(char *resource, unsigned char length, unsigned char *NAME){
  resource[0] = length;
  memset(resource + 1, NAME, length);
  resource[length + 1] = 0;
}

void putTYPE(char *resource, unsigned short TYPE){
  unsigned char nameLen = GETNAMELEN(resource);
  (unsigned short *)(resource + nameLen) = TYPE;
}

void putCLASS(char *resource, unsigned short CLASS){
  unsigned char nameLen = GETNAMELEN(resource);
  (unsigned short *)(resource + nameLen + 2) = CLASS;
}

void putTTL(char *resource, unsigned int TTL){
  unsigned char nameLen = GETNAMELEN(resource);
  (unsigned int *)(resource + nameLen + 4) = TTL;
}

void putRDLENGTH(char *resource, unsigned short RDLENGTH){
  unsigned char nameLen = GETNAMELEN(resource);
  (unsigned short *)(resource + nameLen + 8) = RDLENGTH;
}

void putRDATA(char *resource, unsigned char *RDATA, unsigned short RDLENGTH){
  unsigned char nameLen = GETNAMELEN(resource);
  memset(resource + nameLen + 10, RDATA, RDLENGTH)
}

char *getEndOfResource(char *resource){
  unsigned char nameLen = GETNAMELEN(resource);
  unsigned short RDLENGTH = *((unsigned short *)(resource + nameLen + 8));
  return resource + nameLen + RDLENGTH + 10;
}

void fillRequestHeaderTemplate(char *header){
  memset(header, 0, 12);
  putAA(header, 0);
  putRD(header, 0);
  putRA(header, 0);
  putZ(header, 0);
  putNSCOUNT(header,0);
  putARCOUNT(header,0);
}

void fillQuestionTemplate(char *question){
  putQTYPE(question,1);
  putQCLASS(question,1);
}

void fillResourceRecordTemplate(char *resource){
  putTYPE(resource,1);
  putCLASS(resource,1);
  putTTL(resource,1);
}


}









