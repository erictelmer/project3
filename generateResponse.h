

#include <netinet/in.h>
#include <netinet/ip.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <signal.h>
#include <stdarg.h>
#include <time.h>
#include "log.h"

#define BUF_SIZE 4096
#define FREE(x,s) /*fprintf(stderr,"freeing %s @ %p\n",s,x);*/ free(x);

#define NUM_FILE_TYPES 3
char * filetypes[NUM_FILE_TYPES][2] = {{".html", "text/html"},{".png", "image/png"},{".css","text/css"}};

extern char wwwfolder[30];

int generateResponse(char readbuf[BUF_SIZE],char ** response, int * responselen);
int parseRequest(char readbuf[BUF_SIZE], char ** request, char ** resource, char ** httpversion);
int generateError(char ** response, int * responselen, int errorNum, char * message);
int handlePost(char ** response, int * responselen, char ** resource);
int handleHead(char ** response, int * responselen, char ** resource);
int handleGet(char ** response, int * responselen, char ** resource);
int handleTest(void);
int generateResponseToResource(char ** response, int * responselen,int status, char * statMsg, char * resource, int sendContent);
int makeHeader(char ** pheader , int * pheadsize/*bytes*/,int status, char * statMsg, char * contenttype, int contentlen/*bytes*/, int * offset/*NULL char*/);
int addLineToString(char ** pstring, int * pstringsize/*bytes*/, int * poffset,const char * line, ...);
void getFilename(char * resource, char ** filename);
void getTimeString(char * timebuf, int bufsize);
int fileSize(FILE * rfile);

char * contentType(char * file, char * contenttype);



int generateResponse(char readbuf[BUF_SIZE],char ** response, int * responselen){
  char * request = NULL;
  char * httpversion = NULL;
  char * resource = NULL;//Change to be dynamically allocated
  
  if(parseRequest(readbuf, &request, &resource, &httpversion) == -1){
    generateError(response, responselen, 501, "Not Implemented");
    FREE(httpversion,"httpversion");
    FREE(resource,"resource");
    FREE(request, "request");
    return 0;
  }
  logString("Recieved Req:%s|Res:%s|Http:%s|\n", request, resource, httpversion);
  if (strcmp(httpversion, "HTTP/1.1")){
    generateError(response, responselen, 505, "Http Version not supported");
    logString("Http version not supported");
    FREE(httpversion,"httpversion");
    FREE(resource,"resource");
    FREE(request, "request");
    return 0;
  }
  FREE(httpversion,"httpversion");
  if (!strcmp(request,"GET")){
    FREE(request,"request");
    handleGet(response, responselen, &resource);
    FREE(resource,"resource");
  }
  else if (!strcmp(request,"HEAD")){
    FREE(request,"request");
    handleHead(response, responselen, &resource);
    FREE(resource,"resource");
  }
  else if (!strcmp(request,"POST")){
    FREE(request,"request");
    handlePost(response, responselen,&resource);
    FREE(resource,"resource");
  }
  else if (!strcmp(request, "TEST")){
    FREE(request,"request");
    handleTest();
    FREE(resource,"resource");
  }
  else{
    generateError(response, responselen, 501,"Not Implemented");
    logString("Invalid Request: %s", request);
    FREE(resource,"resource");
    FREE(request,"request");
  }
  return 0;
}

int parseRequest(char readbuf[BUF_SIZE], char ** request, char ** resource, char ** httpversion){
  char * index;

  unsigned char reqsize = sizeof(char) * 5;
  unsigned char httpsize = sizeof(char) * 15;
  unsigned char ressize = sizeof(char) * 100;
  *httpversion = malloc(httpsize);
  *resource = malloc(ressize);
  *request = malloc(reqsize);

  index = strtok(readbuf," \n\r");
  if (index==NULL){
    logString("No request found");
    return -1;
  }
  strncpy((*request), index, reqsize);
  (*request)[reqsize-1] = (char)(0);
  if (strcmp(index, (*request))){
    logString("Request too large:%s|", (*request));
    return -1;
  }
  index = strtok(NULL," \n\r");
  if (index==NULL){
    logString("No resource found");
    return -1; 
  }
  strncpy((*resource), index, ressize);
  (*resource)[ressize-1] = (char)(0);
  if (strcmp(index, (*resource))){
    logString("Resource too large:%s|", (*resource));
    return -1;
  }
  index = strtok(NULL, " \n\r");
  
  if (index == NULL){
    logString("Did not read valid httpversion");
    return -1;
  }
  strncpy((*httpversion), index, httpsize);
  (*httpversion)[httpsize-1] = (char)(0);
  if (strcmp(index, (*httpversion))){
    logString("Http too large:%s|", (*httpversion));
    return -1;
  }
  return 0;
}

int addLineToString(char ** pstring, int * pstringsize/*bytes*/, int * poffset,const char * line, ...){
  va_list args;
  va_start(args, line);  
  int writtenChars;
  int stringsize = *pstringsize;
  char * string = *pstring;
  int offset = *poffset;
  int sizeLeft = stringsize - offset;//0 1 2 3 

  writtenChars = vsnprintf(string + offset,sizeLeft ,line,args);
  while ((writtenChars >= sizeLeft) || (writtenChars < 0)){//Check for NULL
    if (writtenChars< 0) {
      return -1;
    }
    stringsize *= 2;
    string = realloc(string,stringsize);
    sizeLeft = stringsize - offset;
    writtenChars = vsnprintf(string + offset,sizeLeft ,line, args);
  }
  va_end(args);
  offset += writtenChars;
  *pstringsize = stringsize;
  *poffset = offset;
  *pstring = string;
  return 0;
}

int makeHeader(char ** pheader , int * pheadsize/*bytes*/,int status, char * statMsg, char * contenttype, int contentlen/*bytes*/, int * offset/*NULL char*/){
  char timebuf[50];
  // int ret;
  getTimeString(timebuf, 50);
  
  addLineToString(pheader,pheadsize, offset, "HTTP/1.1 %d %s\r\nDate: %s\r\n", status,statMsg,timebuf);

  addLineToString(pheader,pheadsize,offset, "Server: Liso/1.0\r\n");

  if ((contenttype != NULL) && (contentlen != 0)){
    addLineToString(pheader,pheadsize,offset,"Content-Type: %s\r\nContent-Length: %d\r\n", contenttype,contentlen);
  }

  addLineToString(pheader,pheadsize,offset, "Connection: keep-alive\r\n\r\n", contentlen);
  return 0;
}

int generateResponseToResource(char ** response, int * responselen,int status, char * statMsg, char * resource, int sendContent){
  int ret;
  char * filename;
  FILE * rfile;
  //int sendsize;
  int contentlen;
  char contenttype[20];
  int bodySize = *responselen;
  char * body = *response;
  int offset = 0;

  filename = malloc(12);//FREE!
  getFilename(resource,&filename);
  contentType(filename, contenttype);
  //Open File
  if ((rfile = fopen(filename,"r")) == NULL) {
    logString("Resource not found: %s", filename);
    generateError(&body, &bodySize, 404, "Not found");
    FREE(filename,"filename");
    FREE(body,"body");
    return -1;
  }
  contentlen = fileSize(rfile);
  FREE(filename,"filename");
  //Put http header into body
  makeHeader(&body, &bodySize, status, statMsg, contenttype, contentlen, &offset);
  if (sendContent){
    //If not enough room in body, make larger
    if (bodySize < (offset + contentlen)){
      bodySize = offset + contentlen;
      body = realloc(body, bodySize);
    }
    //read requested content into body
    if ((ret = fread(body + offset, sizeof(char), contentlen, rfile)) !=contentlen){
      logString("Resource read error");
      generateError(&body,&bodySize , 500,"Internal Server Error");
      FREE(body,"body");
      return -1;
    }
    offset = offset + contentlen - 1;
  }
  fclose(rfile);
  //Send everything
  offset = offset + 1;
  *responselen = offset;
  *response = body;
  return 0;
}

int handleTest(void){
  int strSize = 8;
  char * testString = malloc(strSize);
  int offset = 0;
  char contenttype[20];
  printf("Adding  Line %s\n", "Four");
  addLineToString(&testString, &strSize, &offset, "Four");
  printf("String = %s\n",testString);
  addLineToString(&testString, &strSize,&offset,"plus Four");
  printf("String= %s, size = %d\n", testString, strSize);
  FREE(testString,"test");

  contentType("blah.png", contenttype);
  printf("Content = %s\n", contenttype);
  return 0;
}

int handleGet(char ** response, int * responselen, char ** resource){
 
  generateResponseToResource(response, responselen,200,"OK", *resource, 1);
  logString("Sent data");
  return 0;
}

int handleHead(char ** response, int * responselen, char ** resource){
  
  generateResponseToResource(response, responselen,200,"OK", *resource, 0);
  
  // sendError(fd, 501, "Method unimplemented");
  return 0;
}

int handlePost(char ** response, int * responselen, char ** resource){
  int bodySize = *responselen;
  int offset = 0;
  char *body = *response;
  
  makeHeader(&body, &bodySize, 200,"OK",NULL,0, &offset);
  
  offset += 1;
  *response = body;
  *responselen=offset;
  
  //sendError(fd, 501, "Method unimplemented");
  return 0;
}  

int generateError(char ** response, int * responselen, int errorNum, char * message)
{
  char timebuf [80];
  char http[256];
  int httplen;
  int offset=0;

  getTimeString(timebuf,80);

  httplen = sprintf(http,  "<head>\n<title>Error response</title>\n</head>\n<body>\n<h1>Error response</h1>\n<p>Error code %d.\n<p>Message: %s.\n<p>Errrrrr\n</body>\n\r\n", errorNum, message);

  addLineToString(response,responselen,&offset,"HTTP/1.1 %d %s\r\n\
Date: %s\r\n",errorNum, message, timebuf);

  addLineToString(response,responselen,&offset, "Server: Liso/1.0\r\n");
  addLineToString(response,responselen,&offset, "Content-Type: text/html\r\n");
  addLineToString(response,responselen,&offset,"Content-Length: %d\r\n", httplen);
  addLineToString(response,responselen,&offset, "Connection: keep-alive\r\n\r\n"); 
  addLineToString(response,responselen,&offset,http);
  //printf("SENDING to %d\n%s",fd, sendbuf);
  offset += 1;
  *responselen = offset;
  return 0;
}


char * contentType(char * file, char * contenttype){
  char * index = NULL;
  int i;
  index = strrchr(file, (int)('.'));//Get pointer to last period
  for (i=0;i<NUM_FILE_TYPES;i++){
    if(!strcmp(index, filetypes[i][0])){
      index = filetypes[i][1];
      strcpy(contenttype, index);
      return index;
    }
  }
  return NULL;
}

int fileSize(FILE * rfile){
  int contentlen;
  fseek(rfile, 0, SEEK_END);
  contentlen = ftell(rfile);
  fseek(rfile, 0, SEEK_SET);
  return contentlen;
}

void getTimeString(char * timebuf, int bufsize){
  time_t rawtime;
  struct tm * timeinfo;
  time(&rawtime);
  timeinfo = gmtime(&rawtime);
  strftime(timebuf, bufsize, "%a, %d %b %G %T GST", timeinfo);
  return;
}

void getFilename(char * resource, char ** filename){
  if ((!strcmp(resource,"/")) || (!strcmp(resource,""))){
    (resource) = "/index.html";
  }
  *filename = realloc(*filename,strlen(resource) +strlen(wwwfolder) );//FREE
  strcpy(*filename, wwwfolder);
  strcat(*filename,(resource));
  return;
}
