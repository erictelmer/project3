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

  printf("FDSET: ");
  for(i=0; i<=fdmax; i++){
    if(FD_ISSET(i, read_fds)){
      printf(", %d", i);
    }
  }
  printf("\n");
  printf("Fdmax = %d\n", fdmax);

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

  client_addr.sin_port = htons(ntohs(client_addr.sin_port) + server_sock);

  if (bind(server_sock, (struct sockaddr*)&client_addr, addr_size) < 0) {
    char tmp[80];
    inet_ntop(AF_INET, &(*stream)->client_addr.sin_addr, tmp, 80);
    printf("Bind addr:\n%s\nport:\n%d\n", tmp, ntohs((*stream)->client_addr.sin_port)); 
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

    printf("Closing connection associated with sockets %d,%d\n", connection->server_sock, connection->browser_sock);

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
  
  signal(SIGINT, sigINThandler);
  
  if (parseCommandLine(argc, argv, &commandLine) < 0)
    return -1;

  //  initLogger(commandLine->logfile);

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
      printf("Sockcont = %d\n", sockcont);
      sock = waitForAction(&master,&read_fds, fdmax, tv, sockcont);/*blocking*/
      printf("Wait for action returned, sok = %d\n", sock);
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
        printf("Determening if coming from browser or server\n");
        connection =  getConnectionFromSocket(stream, sock);

        if (connection == NULL){
          printf("NULL connec\n");
          return EXIT_FAILURE;
        }
        printf("Got connections\nBrowserSock = %d, Serversock = %d\n", connection->browser_sock, connection->server_sock);

        if (sock == connection->browser_sock){
          ret = receive(sock, &master, &fdmax, browserListener, &buf, connection, stream);
          printf("Recieved %d bytes from browser\n", ret);
          if (ret > 0)
            printf("Recieved from browser:\n\n\n\n%s\n\n\n\n", buf);
            sendResponse(connection->server_sock, buf, ret);
          //Recieved request from browser
          //Determine if request is for nondata(html,swf,f4m(manifest))
          if (1)/*non-data*/{
            //if beggining of stream fill in stream_s
            //IF NEW CHUNK FILL IN CONNECTION
          }
          else{//data, seg
            //requestedChunk(connection, chunkname);//will start throughput
            
            //modify request for given bitrate
            //send request
          }
        }

        if (sock == connection->server_sock){
          ret = receive(sock, &master, &fdmax, browserListener, &buf, connection, stream);
          printf("Recieved %d bytes from server\n", ret);
          printf("FDSET: ");
          int i;
          for(i=0; i<=fdmax; i++){
            if(FD_ISSET(i, &master)){
              printf(", %d", i);
            }
          }
          printf("\n");

          if (ret > 0)
            printf("Recieved from server:\n\n\n\n%s\n\n\n\n", buf);
            sendResponse(connection->browser_sock, buf, ret);
          //Recieved reply from server
          //if non-data
             //fill in stream
          //if data
            //find chunk in connection
            //edit throughput
            //forward
        }
        

      }//End else
    }//End while(1)
  close_socket(browserListener);
  //endLogger();
  return EXIT_SUCCESS;
}
