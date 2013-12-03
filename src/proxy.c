/******************************************************************************
* proxy.c                                                               *
*                                                                             *
* Description: This file contains the C source code for a video streaming proxy                                          *
*                                                                             *
* Authors:Eric Telmer <eit@andrew.cmu.edu>                                    *
*                                                                             *
*******************************************************************************/

#include "proxy.h"

/* 

Everything commented out below is in the proxy.h file
proxy.c should still compile
To be removed before final submission

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

#include "proxy.h"

#include <openssl/rsa.h>
#include <openssl/crypto.h>
#include <openssl/x509.h>
#include <openssl/pem.h>
#include <openssl/ssl.h>
#include <openssl/err.h>

#include "log.h"
#include "generateResponse.h"


#define CHK_NULL(x) if ((x)==NULL) {logString("NULL ERROR"); exit (1);}
#define CHK_ERR(err,s) if ((err)==-1) { logString("%s error", s);perror(s); exit(1); }
#define CHK_SSL(err) if ((err)==-1) {  logString("SSL ERROR");exit(2); }
#define FREE(x,s) //fprintf(stderr,"freeing %s @ %p\n",s,x); free(x);
*/

static FILE *log, *p_log, *dns_log;
int port_offset;


int close_socket(int sock){
  if (close(sock)){
    log_msg(log, "Failed closing socket..\n");
        return 1;
  }
  return 0;
}

void sigINThandler(int sig){
	close_socket(3);
}

void leave(void){
  //endLogger();
}

void * Malloc(size_t size, char *name){
  void *loc;
  loc = malloc(size);
  if (loc == NULL){
    log_msg(log, "Error: Malloc on %s\n", name);
    exit(3);
  }
  // printf("Mallocing %s of size: %x @ %p\n", name, (unsigned int)size, loc);
  return loc;
}

void * Realloc(void *pointer, size_t size, char *name){
  void *loc;
  loc =  realloc(pointer,size);
  if (loc == NULL){
    log_msg(log, "Error: Realloc on %s\n", name);
    exit(3);
  }
  //  printf("Reallocing %s @ %p of size: %x @ %p\n", name,pointer, (unsigned int)size, loc);
  return loc;
}

int waitForAction(fd_set *master, fd_set *read_fds, int fdmax, struct timeval tv, int fdcont){

  int i;

  *read_fds = *master; // copy it


  tv.tv_sec = 60;
  i = select(fdmax+1, read_fds, NULL, NULL, &tv);
  if (i == -1) {
    perror("select");
    exit(4);
  }
  if (i == 0){
    printf("I'm waiting...\n");
    return -1;
  }
  //L1
  for(; fdcont<=fdmax; fdcont++){
    if (FD_ISSET(fdcont, read_fds)){ //socket ready
      return fdcont;
    }//End fd isset
  }//End for loop
  return 0;
}

int acceptBrowserServerConnectionToStream(int browserListener, fd_set * master, int * fdmax, stream_s **stream, command_line_s *commandLine){
  socklen_t addr_size;

  struct sockaddr_in browser_addr;
  connection_list_s *connection = NULL;

  int browser_sock;
  int server_sock;
  struct sockaddr_in client_addr;
  struct sockaddr_in server_addr;

  char str[50];
  addr_size = sizeof(browser_addr);

  printf("Attempting to accept connection\n");
  //  Log(p_log,"Attempting to accept connection\n");

  if(*stream == NULL){//new stream!
    memcpy(&client_addr.sin_addr, &commandLine->fake_ip, sizeof(struct in_addr));
    memcpy(&server_addr.sin_addr, &commandLine->www_ip, sizeof(struct in_addr));
    client_addr.sin_family = AF_INET;
    server_addr.sin_family = AF_INET;
    client_addr.sin_port = htons(RAND_PORT);
    server_addr.sin_port = htons(SERV_PORT);

    memset(client_addr.sin_zero, '\0', sizeof(client_addr.sin_zero));
    memset(server_addr.sin_zero, '\0', sizeof(server_addr.sin_zero));

    *stream = newStream(&client_addr, &server_addr, commandLine->alpha);
  }
  else{
    memcpy(&client_addr, &(*stream)->client_addr, addr_size);
    memcpy(&server_addr, &(*stream)->server_addr, addr_size);
  }

  //create new socket to server//CHECK FOR ERRORS
  if ((server_sock = socket(PF_INET, SOCK_STREAM, 0)) < 0) printf("socket failed\n");

  //change port number to unoccupied one, client_addr should be copied from stream
  client_addr.sin_port = htons(ntohs(client_addr.sin_port) + port_offset);
  INC_POFFSET();

  if (bind(server_sock, (struct sockaddr*)&client_addr, addr_size) < 0) {
    char tmp[80];
    inet_ntop(AF_INET, &client_addr.sin_addr, tmp, 80);
    printf("Socket: %d\nBind addr: %s\nport: %d\n", server_sock, tmp, ntohs(client_addr.sin_port)); 
    printf("Bind failed\n");
    perror("bind"); 
    exit(EXIT_FAILURE);
    
  }

  if (connect(server_sock, (struct sockaddr*)&(*stream)->server_addr, addr_size)) printf("Connect failed\n");

  //accept new socket to browser
  if ((browser_sock = accept(browserListener, (struct sockaddr *) &browser_addr,
                            &addr_size)) == -1)
    {
      close(browserListener);
      //      logString("Error accepting connection.");
      return EXIT_FAILURE;
    }
  
  //create new connection adn add it to stream
  connection = newConnection(browser_sock, server_sock);
  addConnectionToStream(connection, *stream);
  printf("Added connections on seckets b:%d,s:%d\n",connection->browser_sock, connection->server_sock); 


  //Add socket to browser to fdset
  FD_SET(browser_sock, master); // add new socket to master set
  if (browser_sock > *fdmax) {    // keep track of the max
    *fdmax = browser_sock;
  }

  //Add socket to server to master
  FD_SET(server_sock, master); // add new socket to master set
  if (server_sock > *fdmax) {    // keep track of the max
    *fdmax = server_sock;
  }

  inet_ntop(AF_INET, (void *)(&(browser_addr.sin_addr)), str, 50);
  //  logString("Accepted connection from %s", str);
  return 0;
}

//returns bytes read
int receive(int fd, fd_set * master, int *fdmax, int listener, char (* buf)[BUF_SIZE], connection_list_s *connection, stream_s *stream){
  int readret;
  int i;
  if ((readret = recv(fd, *buf, 1, MSG_PEEK)) > 0)//Check if connection closed
    {
      /*if (!FD_ISSET(fd, &write_fds)) {
        memset(*buf,0, BUF_SIZE);
        //        logString("Could not write");
        return -2;
        }*/
      //Http handling//
      if ((readret = recv(fd, *buf, BUF_SIZE, 0)) > 0){
        //logString("Read data from fd:%d", fd);
        return readret;
      }
      //End gttp handling//
      memset(*buf,0,BUF_SIZE);
    } 
 
  if (readret <= 0){//If connection closed
    //    logString("Connection closed on fd:%d",fd);
    close_socket(connection->server_sock);
    close_socket(connection->browser_sock);
    FD_CLR(connection->server_sock, master);
    FD_CLR(connection->browser_sock, master);

    printf("Closing connection associated with sockets %d,%d because of %d\n", connection->server_sock, connection->browser_sock, fd);

    removeConnectionFromStream(connection, stream);
    freeConnection(connection);

    printf("FDSET: ");
    for(i=0; i<=*fdmax; i++){
      if(FD_ISSET(i, master)){
        printf(", %d", i);
      }
    }
    printf("\n");

    //change fdmax
  }
  return -1;
}


int sendResponse(int fd, char * response, int responselen){
  if (send(fd,response,responselen,0)!=responselen)
    {
      close_socket(fd);
      close_socket(3);
      printf("Send Failed\n");
      //     logString("Error sending to client.");
      return EXIT_FAILURE;
    }
  //  logString("Sent response to fd:%d", fd);
  return 0;
}

int setupBrowserListenerSocket(int * plistener, unsigned short port){
  int listener;
  struct sockaddr_in addr;
  
  if ((listener = socket(PF_INET, SOCK_STREAM, 0)) == -1)
    {
      //      logString("Failed creating socket.");
      return -1;
    }
  addr.sin_family = AF_INET;
  addr.sin_port = htons(port);
  addr.sin_addr.s_addr = INADDR_ANY;
 
  printf("Binding to port:%d\n", port);
 
  /* servers bind sockets to ports---notify the OS they accept connections */
  if (bind(listener, (struct sockaddr *) &addr, sizeof(addr)))
    {
      close_socket(listener);
      //     logString("Failed binding socket.");
      return -1;
    }
  
  if (listen(listener, 5))
    {
      close_socket(listener);
      //     logString("Error listening on socket.");
      return -1;
    }
  *plistener = listener;
  return 0;
}

//bufstrlen length of string in buf
//str string to insert into buf
//index: place in buf to replace
//deleteLen: chars to write over in buf
//replaceLen: num chars to write from str
int replaceString(char *buf, unsigned int bufstrlen, char *str, unsigned int index, unsigned int deleteLen, unsigned int replaceLen)
{
  char *endBuf = malloc(sizeof(char) * bufstrlen);
  memcpy(endBuf, buf + index + deleteLen, bufstrlen + 1 - index - deleteLen);
  memcpy(buf + index, str, replaceLen);
  memcpy(buf + index + replaceLen - deleteLen + 1, endBuf, bufstrlen + 1 - index - deleteLen);
  free(endBuf);
  return 0;
}


//Call before sending request
int startChunk(connection_list_s *connection, char *chunkName){
  //Check for NULL
  chunk_list_s *chunk = newChunkList();
  addChunkToConnections(chunk, connection);

  //+1 to include NULL char
  memcpy(chunk->chunk_name, chunkName, strlen(chunkName) + 1);

  time(&chunk->time_started);
  return 0;
}

//assuming only 1 chunk active at once
int finishChunk(stream_s *stream, connection_list_s *connection, unsigned int chunkSize){
  chunk_list_s *chunk = connection->chunk_throughputs;
  double duration;
  double throughput;
  float alpha = stream->alpha;
  time(&chunk->time_finished);
  duration = difftime(chunk->time_finished, chunk->time_started);
  throughput = (chunkSize / duration) * (8.0 / 1000);
  stream->current_throughput = (throughput * alpha) + ((1 - alpha) * stream->current_throughput); 
  //log
  removeChunkFromConnections(chunk, connection);
  //beware of freeing the entire list if concurrent chunks
  freeChunkList(chunk);
  return 1;
}


int main(int argc, char* argv[])
{
  
  command_line_s commandLine;
  stream_s *stream = NULL;
  connection_list_s *connection;

  int browserListener;
 
  fd_set master;
  fd_set read_fds;

  int fdmax;
  int sock;
  struct timeval tv;
  
  char buf[BUF_SIZE];
  int ret;
  int responselen = 200;
  char * response;

  int sockcont=0;

  port_offset = 0;
  signal(SIGINT, sigINThandler);
  
  if (parseCommandLine(argc, argv, &commandLine) < 0)
    return -1;

  p_log = open_log(p_log, commandLine.logfile);
  
  tv.tv_sec = 60;
  tv.tv_usec = 0;
  FD_ZERO(&master);
  FD_ZERO(&read_fds);

  //  logString("Server Initiated");
  
  /* set up socket to listen for incoming connections from the browser*/
  if (setupBrowserListenerSocket(&browserListener, commandLine.listen_port) < 0)
    return EXIT_FAILURE;
  
  /*Add browser listerner to master set of fd's*/
  FD_SET(browserListener, &master);
  fdmax = browserListener;

  printf("Browser listener: %d\n", browserListener);
  
  tv.tv_sec = 60;
  while (1)
    {
      //wait for a socket to have data to read
      sock = waitForAction(&master,&read_fds, fdmax, tv, sockcont);/*blocking*/
      if (sock <0) continue;//select timeout
      if (sock == 0){//read through read_fs , start from beginning
        sockcont = 0;
        continue;
      }
      sockcont = sock + 1;
      
      
      //Browser is requesting new connection
      if (sock == browserListener){ //new connection        
        if (stream == NULL){
         
        }
        if (acceptBrowserServerConnectionToStream(browserListener, &master, &fdmax, &stream, &commandLine) < 0)//add connection to stream
          //add browser sock to read_fs
          return EXIT_FAILURE;
        //if no current streams 
        
      }

      else {
        //Determine if coming from browser or server
        connection =  getConnectionFromSocket(stream, sock);

        if (connection == NULL){
          printf("NULL connec\n");
          return EXIT_FAILURE;
        }

        if (sock == connection->browser_sock){
	  memset(buf, 0, BUF_SIZE);
          ret = receive(sock, &master, &fdmax, browserListener, &buf, connection, stream);

	  int x;
	  char header[100];
	  x = strcspn(buf, "\n");
	  if (x > 99) x = 99;
	  memcpy(header, buf, x);
	  header[x] = '\0';
	  printf("Recieved  request from browser:\n%s\n\n", header);
          if (ret > 0){
	    if (1){//Get request is /, /swfobject.js, /strobeMediaPlayback forward unchanged
	      if (0)
		printf("Recieved from browswer\n%s\n", buf); 
	      sendResponse(connection->server_sock, buf, ret);
	    }
	    if (1)/*Get request is .f4m, either:
		    1. send request for .f4m, saved it, then send GET ..._nolist.f4m and forward that
		    2. send request for both and decide which one to forward back by parsing*/{
	    }
	    if (1)/*Get request is for /vod/bitratesegFrag modify it for current throughput
		    and start a new chunk*/
	      {
		
	      }
	  }
	  
        }
	
        if (sock == connection->server_sock){
	  memset(buf, 0,BUF_SIZE);
          ret = receive(sock, &master, &fdmax, browserListener, &buf, connection, stream);
	  

	  //Use close to avoid streamlining 
	  char *p, *type, *len;
	  char close[] = "close     ";
	  char contentType[] = "Content-Type: ";
	  char header[100];
	  memset(header, '\0', 100);
	  int x;
	  if (memcmp(buf, "HTTP/1.1", strlen("HTTP/1.1")) == 0){
	    if ((p = strstr(buf, "Connection: ")) != NULL){
	      p = p + strlen("Connection: ");
	      memcpy(p, close, strlen(close));
	    }
	  }
	  
	  
	  // printf("Recieved from server\n%s\n",buf);
	  // p is a pointer to Content-Type
	  p = strstr(buf, contentType);
	  if (p != NULL){
	    x = strcspn(p, "\n");
	    if (x > 99) x = 99;
	    memcpy(header, p, x);
	    header[x] = '\0';
	    fflush(stdout);

	    if ((type = strstr(header, "Content-Type: ")) != NULL){ //found Content-Type
		type = type + strlen("Content-Type: ");
	    }

	    //if video/f4f then we need to get content length
	    if( strstr(type, "video/f4f") != NULL){
		if ((len = strstr(buf, "Content-Length: ")) != NULL){
		   len = len + strlen("Content-Length: ");
		}
	    }

	    if (strstr(header, "text/xml") != NULL){
		//iniitialize throughput to be lowest one
	      printf("%s\n", buf);
	    }
	  }
	  
	  
          if (ret > 0){
	    if (1)
	      printf("Recieved %d:\n%s\n",ret, buf);
            sendResponse(connection->browser_sock, buf, ret);
	    if(1)/*Content type is one of the following forward untouched:
		  text/html, application/javascript, application/x-shockwave-flash*/
	      {
		
	      }
	    if(1)/*Content type is text/html decide what to do based on decision above */
	      {
	      }
	    if(1)/*Content type is video/f4f, get content length to detrmine when the chunk is done?
		   measure header length until /n/r/n/r and then subreact that from ret
		   to see how much data you got
		  and forward*/
	      {
	      }
	    if(1)/*No content length, forward bytes, if == content length, finish chunk */
	  }
        }

      }//End else
    }//End while(1)

  close_socket(browserListener);
  //endLogger();
  return EXIT_SUCCESS;
}
