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


//01 ff 02 ff ff 00
//namelen = 1
//name += 1+1, *name = 2
//namelen += 3, namelen = 4

//name += 3, *name = 0
//namelen += 1, namelen = 5
unsigned char getNameLen(char *name);

char *getEndOfHeader(char *message);

void putID(char *message, unsigned short ID);

unsigned short getID(char *message);

void putQR(char *message, unsigned char QR);

unsigned char getQR(char *message);

void putOPCODE(char *message, unsigned char OPCODE);

unsigned char getOPCODE(char *message);

void putAA(char *message, unsigned char AA);

void putTC(char*message, unsigned char TC);

void putRD(char*message,unsigned char RD);

void putRA(char*message,unsigned char RA);

void putZ(char*message,unsigned char Z);

void putRCODE(char*message,unsigned char RCODE);

unsigned char getRCODE(char *message);

void putQDCOUNT(char *message, unsigned short QDCOUNT);

unsigned short getQDCOUNT(char *message);

void putANCOUNT(char *message, unsigned short ANCOUNT);

unsigned short getANCOUNT(char *message);

void putNSCOUNT(char *message, unsigned short NSCOUNT);

void putARCOUNT(char *message, unsigned short ARCOUNT);

//Question section format

void putQNAME(char *question);

void putQTYPE(char *question, unsigned short QTYPE);

void putQCLASS(char *question, unsigned short QCLASS);

char *getEndOfQuestion(char *question);

//Resource Record format

void putNAME(char *resource);

void putTYPE(char *resource, unsigned short TYPE);

void putCLASS(char *resource, unsigned short CLASS);

void putTTL(char *resource, unsigned int TTL);

void putRDLENGTH(char *resource, unsigned short RDLENGTH);

unsigned short getRDLENGTH(char *resource);

void putRDATA(char *resource, unsigned char *RDATA, unsigned short RDLENGTH);

void getRDATA(char *resource, char *RDATA);

char *getEndOfResource(char *resource);

void fillRequestHeaderTemplate(char *header);

void fillResponseHeaderTemplate(char *header);

//QName is hardcoded
void fillQuestionTemplate(char *question);

//Name is hardcoded
void fillResourceRecordTemplate(char *resource);

int isVideoCsCmuEdu(char *name);







