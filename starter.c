
/******************************************************************************
* echo_server.c                                                               *
*                                                                             *
* Description: This file contains the C source code for an echo server.  The  *
*              server runs on a hard-coded port and simply write back anything*
*              sent to it by connected clients.  It does not support          *
*              concurrent clients.                                            *
*                                                                             *
* Authors:Eric Telmer <eit@andrew.cmu.edu>                                    *
*                                                                             *
*******************************************************************************/

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

#include "openssl/ssl.h"

/*#include <openssl/rsa.h>
#include <openssl/crypto.h>
#include <openssl/x509.h>
#include <openssl/pem.h>
#include <openssl/ssl.h>
#include <openssl/err.h>
*/


//#include "log.h"
#include "generateResponse.h"

#define ECHO_PORT 8088
#define BUF_SIZE 4096

#define CHK_NULL(x) if ((x)==NULL) {logString("NULL ERROR"); exit (1);}
#define CHK_ERR(err,s) if ((err)==-1) { logString("%s error", s);perror(s); exit(1); }
#define CHK_SSL(err) if ((err)==-1) {  logString("SSL ERROR");exit(2); }
#define FREE(x,s) /*fprintf(stderr,"freeing %s @ %p\n",s,x);*/ free(x);


struct typedef{
  char log[CMDLNSTRLEN];
  float alpha;
  unsigned short listen_port;
  char fake_ip[CMDLNSTRLEN];
  char dns_ip[CMDLNSTRLEN];
  unsigned short dns_port;
  char www_ip[CMDLNSTRLEN];
}command_line_s;


void sigINThandler(int);

void leave(void){
  //	endLogger();
}

void * Malloc(size_t size, char * name){
  void * loc;
  loc = malloc(size);
  if (loc == NULL){
    logString("MALLOC ERROR");
    exit(3);
  }
  // printf("Mallocing %s of size: %x @ %p\n", name, (unsigned int)size, loc);
  return loc;
}

void * Realloc(void * pointer, size_t size, char * name){
  void * loc;
  loc =  realloc(pointer,size);
  if (loc == NULL){
    logString("MALLOC ERROR");
    exit(3);
  }
  //  printf("Reallocing %s @ %p of size: %x @ %p\n", name,pointer, (unsigned int)size, loc);
  return loc;
}

int close_socket(int sock)
{
    if (close(sock))
    {
        logString("Failed closing socket../");
        return 1;
    }
    return 0;
}





int waitForAction(fd_set master, fd_set * read_fds, fd_set *write_fds, int fdmax, struct timeval tv, int fdcont){

  int i;

  *read_fds = master; // copy it
  *write_fds = master;
  tv.tv_sec = 60;
  i = select(fdmax+1, read_fds, write_fds, NULL, &tv);
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

int acceptNewConnection(int listener, fd_set * master, int * fdmax){
  socklen_t cli_size;
  struct sockaddr_in cli_addr;
  int client_sock;
  char str[50];
  cli_size = sizeof(cli_addr);
  if ((client_sock = accept(listener, (struct sockaddr *) &cli_addr,
			    &cli_size)) == -1)
    {
      close(listener);
      logString("Error accepting connection.");
      return EXIT_FAILURE;
    }
  FD_SET(client_sock, master); // add new socket to master set
  if (client_sock > *fdmax) {    // keep track of the max
    *fdmax = client_sock;
  }
  inet_ntop(AF_INET, (void *)(&(cli_addr.sin_addr)), str, 50);
  logString("Accepted connection from %s", str);
  return 0;
}






int receive(int fd, fd_set * master, int *fdmax, fd_set write_fds, int listener, char (* buf)[BUF_SIZE]){
  int readret;
  int j;
  if ((readret = recv(fd, *buf, 1, MSG_PEEK)) > 0)//Check if connection closed
    {
      if (!FD_ISSET(fd, &write_fds)) {
	memset(*buf,0, BUF_SIZE);
	logString("Could not write");
	return -2;
      }
      //Http handling//
      if ((readret = recv(fd, *buf, BUF_SIZE, 0)) > 0){
	logString("Read data from fd:%d", fd);
	return 0;
      }
      //End gttp handling//
      memset(*buf,0,BUF_SIZE);
    } 
  if (readret == 0){//If connection closed
    logString("Connection closed on fd:%d",fd);
    close_socket(fd);
    FD_CLR(fd, master);
  }
  if (readret < 0)
    {
      for(j = listener + 1; j<=*fdmax; j++){
	close_socket(j);
      }
      logString("Error reading data from client");
      FD_ZERO(master);
      FD_SET(listener, master);
      *fdmax = listener;
    }
  return -1;
}


int sendResponse(int fd, char * response, int responselen){
  if (send(fd,response,responselen,0)!=responselen)
    {
      close_socket(fd);
      close_socket(3);
      logString("Error sending to client.");
      return EXIT_FAILURE;
    }
  logString("Sent response to fd:%d", fd);
  return 0;
}

int setupListenerSocket(int * plistener, int port){
  int listener = *plistener;
  struct sockaddr_in addr;
  
  if ((listener = socket(PF_INET, SOCK_STREAM, 0)) == -1)
    {
      logString("Failed creating socket.");
      return -1;
    }
  addr.sin_family = AF_INET;
  addr.sin_port = htons(port);
  addr.sin_addr.s_addr = INADDR_ANY;
  
  /* servers bind sockets to ports---notify the OS they accept connections */
  if (bind(listener, (struct sockaddr *) &addr, sizeof(addr)))
    {
      close_socket(listener);
      logString("Failed binding socket.");
      return -1;
    }
  
  if (listen(listener, 5))
    {
      close_socket(listener);
      logString("Error listening on socket.");
      return -1;
    }
  *plistener = listener;
  return 0;
}



int main(int argc, char* argv[])
{
  
  command_line_s commandLine;

  int browserListener;
 
  fd_set master;
  fd_set read_fds;
  fd_set write_fds;
  int fdmax;
  int sock;
  struct timeval tv;
  
  char buf[BUF_SIZE];
  int ret;
  int responselen = 200;
  char * response;
  
  signal(SIGINT, sigINThandler);
  
  if (parseCommandLine(argc, argv, commandLine) < 0)
    return -1;
  initLogger(logfile);
  initSSL(&ctx);

  tv.tv_sec = 60;
  tv.tv_usec = 0;
  FD_ZERO(&master);
  FD_ZERO(&read_fds);
  FD_ZERO(&write_fds);
  fprintf(stdout, "----- Echo Server -----\n");
  logString("Server Initiated");
  
  /* all networked programs must create a socket */
  if (setupBrowserListenerSocket(&browserListener, HTTPport) < 0)
    return EXIT_FAILURE;
  
  /*Add listerner to master set of fd's*/
  FD_SET(browserListener, &master);
  fdmax = browserListener;
  
  tv.tv_sec = 60;
  int fdcont = 0;
  /* finally, loop waiting for input and then write it back */
  while (1)
    {
      // int fdcont=0;
      sock = waitForAction(master,&read_fds,&write_fds, fdmax, tv, sockcont);/*blocking*/ 
      if (sock <0) continue;
      if (sock == 0){
	sockcont = 0;
	continue;
      }
      sockcont = sock;
      
      //Browser is requesting new connection
      if (sock == browserListener){ //new connection
	if (acceptNewConnection(browserListener, &master, &fdmax) < 0)
	  return EXIT_FAILURE;
	//if no current streams 
	if (stream == NULL){
	  //createNewStream
	  //addConnectionToStream
	}
	else{
	  //addConnectionToStrem
	}
      }

      else {
	//Determine if coming from browser or server
	connection_s *connection =  getConnectionFromSocket(stream_s stream, sock);
	if (connection == NULL){
	  
	}

	if (sock == connection->browser_sock){
	  //Recieved request from browser
	  //Determine if request is for nondata(html,swf,f4m(manifest)) or seg
	  if (1)/*non-data*/{
	    //fill in stream_s
	  }
	  else{//data
	    //requestedChunk(connection, chunkname);//will start throughput
	    
	    //modify request for given bitrate
	    //send request
	  }
	}

	if (sock == connection->server_sock){
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

void sigINThandler(int sig)
{
  close_socket(3);
  //endLogger();
}
