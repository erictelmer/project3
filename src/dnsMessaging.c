#include "dnsMessaging.h"


#define GETNAMELEN(x) return ((unsigned char*)x)[0] + 2;

//01 ff 02 ff ff 00
//namelen = 1
//name += 1+1, *name = 2
//namelen += 3, namelen = 4

//name += 3, *name = 0
//namelen += 1, namelen = 5
unsigned char getNameLen(char *name){
  unsigned char nameLen = name[0] + 1;
  while(name[0] != 0){
    name += name[0] + 1;
    nameLen += name[0] + 1;
  }
  return nameLen;
}

char *getEndOfHeader(char *message){
  return message + 12;
}

void putID(char *message, unsigned short ID){
  ((unsigned short *)(message))[0] = ID;
}

unsigned short getID(char *message){
  return ((unsigned short *)message)[0];
}
//1 b
void putQR(char *message, unsigned char QR){
  message[2] = (message[2] && 0x7f) || ((QR && 0x01) << 7);
}

unsigned char getQR(char *message){
  return (message[2] && 0x80) >> 7;
}

void putOPCODE(char *message, unsigned char OPCODE){
  message[2] = (message[2] && 0x87) || ((OPCODE && 0x0f) << 3);
}

unsigned char getOPCODE(char *message){
  return (message[2] && 0x78) >> 3;
}

void putAA(char *message, unsigned char AA){
  message[2] = (message[2] && 0xfb) || ((AA && 0x01) << 2);
}

void putTC(char*message, unsigned char TC){
  message[2] = (message[2] && 0xfd) || ((TC && 0x01) << 1);
}

void putRD(char*message,unsigned char RD){
  message[2] = (message[2] && 0xfe) || (RD && 0x01);
}

void putRA(char*message,unsigned char RA){
  message[3] = (message[3] && 0x7f) || ((RA && 0x01)<<7);
}

void putZ(char*message,unsigned char Z){
  message[3] = (message[3] && 0x8f) || ((Z && 0x07)<<4);
}

void putRCODE(char*message,unsigned char RCODE){
  message[3] = (message[3] && 0xf0) || ((RCODE && 0x0f));
}

unsigned char get

void putQDCOUNT(char *message, unsigned short QDCOUNT){
  *(unsigned short *)(message + 4) = QDCOUNT;
}

void putANCOUNT(char *message, unsigned short ANCOUNT){
  *(unsigned short *)(message + 6) = ANCOUNT;
}

void putNSCOUNT(char *message, unsigned short NSCOUNT){
  *(unsigned short *)(message + 8) = NSCOUNT;
}

void putARCOUNT(char *message, unsigned short ARCOUNT){
  *(unsigned short *)(message + 10) = ARCOUNT;
}

//Question section format

void putQNAME(char *question, unsigned char length, unsigned char *name){
  question[0] = length;
  memcpy(question + 1, name, length);
  question[length + 1] = 0;
}

void putQTYPE(char *question, unsigned short QTYPE){
  unsigned char nameLen = getNameLen(question);
  *(unsigned short *)(question + nameLen) = QTYPE;
}

void putQCLASS(char *question, unsigned short QCLASS){
  unsigned char nameLen = getNameLen(question);
  *(unsigned short *)(question + nameLen + 2) = QCLASS;
}

char *getEndOfQuestion(char *question){
  unsigned char nameLen = getNameLen(question);
  return question + nameLen + 4;
}

//Resource Record format

void putNAME(char *resource, unsigned char length, unsigned char *NAME){
  resource[0] = length;
  memcpy(resource + 1, NAME, length);
  resource[length + 1] = 0;
}

void putTYPE(char *resource, unsigned short TYPE){
  unsigned char nameLen = getNameLen(resource);
  *(unsigned short *)(resource + nameLen) = TYPE;
}

void putCLASS(char *resource, unsigned short CLASS){
  unsigned char nameLen = getNameLen(resource);
  *(unsigned short *)(resource + nameLen + 2) = CLASS;
}

void putTTL(char *resource, unsigned int TTL){
  unsigned char nameLen = getNameLen(resource);
  *(unsigned int *)(resource + nameLen + 4) = TTL;
}

void putRDLENGTH(char *resource, unsigned short RDLENGTH){
  unsigned char nameLen = getNameLen(resource);
  *(unsigned short *)(resource + nameLen + 8) = RDLENGTH;
}

void putRDATA(char *resource, unsigned char *RDATA, unsigned short RDLENGTH){
  unsigned char nameLen = getNameLen(resource);
  memcpy(resource + nameLen + 10, RDATA, RDLENGTH);
}

char *getEndOfResource(char *resource){
  unsigned char nameLen = getNameLen(resource);
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

int isVideoCsCmuEdu(char *name){
  char cmuName[18] = {0x05, 0x76, 0x69, 0x64, 0x65, 0x6f, 0x02, 0x63, 0x73, 0x03, 0x63, 0x6d, 0x75, 0x03, 0x65, 0x64, 0x75, 0x00};//encoding for video.cs.cmu.edu...trust me!
  if (memcmp(name, cmuName, 18))
    return 1;
  return 0;
}







