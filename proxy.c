
/******************************************************************************
* prsoxy.c                                                               *
*                                                                             *
* Description: This file contains the C source code for a video streaming proxy                                          *
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

#include "throughput_connections.h"
#include "command_line.h"

/*#include <openssl/rsa.h>
#include <openssl/crypto.h>
#include <openssl/x509.h>
#include <openssl/pem.h>
#include <openssl/ssl.h>
#include <openssl/err.h>
*/


#include "log/log.h"
//#include "generateResponse.h"

#define RAND_PORT 8088
#define SERV_PORT 8080
#define BUF_SIZE 4096


#define CHK_NULL(x) if ((x)==NULL) {logString("NULL ERROR"); exit (1);}
#define CHK_ERR(err,s) if ((err)==-1) { logString("%s error", s);perror(s); exit(1); }
#define CHK_SSL(err) if ((err)==-1) {  logString("SSL ERROR");exit(2); }
#define FREE(x,s) /*fprintf(stderr,"freeing %s @ %p\n",s,x);*/ free(x);




void sigINThandler(int);

void leave(void){
  //	endLogger();
}

void * Malloc(size_t size, char * name){
  void * loc;
  loc = malloc(size);
  if (loc == NULL){
    //   logString("MALLOC ERROR");
    exit(3);
  }
  // printf("Mallocing %s of size: %x @ %p\n", name, (unsigned int)size, loc);
  return loc;
}

void * Realloc(void * pointer, size_t size, char * name){
  void * loc;
  loc =  realloc(pointer,size);
  if (loc == NULL){
    //   logString("MALLOC ERROR");
    exit(3);
  }
  //  printf("Reallocing %s @ %p of size: %x @ %p\n", name,pointer, (unsigned int)size, loc);
  return loc;
}

int close_socket(int sock)
{
    if (close(sock))
    {
      //        logString("Failed closing socket../");
        return 1;
    }
    return 0;
}





int waitForAction(fd_set master, fd_set * read_fds, int fdmax, struct timeval tv, int fdcont){

  int i;

  *read_fds = master; // copy it

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

int acceptBrowserServerConnectionToStream(int browserListener, fd_set * master, int * fdmax, stream_s *stream, command_line_s *commandLine){
  socklen_t addr_size;

  struct sockaddr_in browser_addr;
  connection_list_s *connection = NULL;

  int browser_sock;
  int server_sock;
  char str[50];
  addr_size = sizeof(browser_addr);

  if(stream == NULL){//new stream!
    struct sockaddr_in client_addr;
    struct sockaddr_in server_addr;

    memcpy(&client_addr.sin_addr, &commandLine->fake_ip, sizeof(struct in_addr));
    memcpy(&server_addr.sin_addr, &commandLine->www_ip, sizeof(struct in_addr));
    client_addr.sin_family = AF_INET;
    server_addr.sin_family = AF_INET;
    client_addr.sin_port = htons(RAND_PORT);
    server_addr.sin_port = htons(SERV_PORT);

    stream = newStream(&client_addr, &server_addr, commandLine->alpha);
  }

  //create new socket to server//CHECK FOR ERRORS
  server_sock = socket(AF_UNSPEC, SOCK_STREAM, SOCK_STREAM);
  bind(server_sock, (struct sockaddr*)&stream->client_addr, addr_size);
  connect(server_sock, (struct sockaddr*)&stream->server_addr, addr_size);

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
  addConnectionToStream(connection, stream);

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
int receive(int fd, fd_set * master, int *fdmax, int listener, char (* buf)[BUF_SIZE]){
  int readret;
  int j;
  if ((readret = recv(fd, *buf, 1, MSG_PEEK)) > 0)//Check if connection closed
    {
      /*if (!FD_ISSET(fd, &write_fds)) {
	memset(*buf,0, BUF_SIZE);
	//	logString("Could not write");
	return -2;
	}*/
      //Http handling//
      if ((readret = recv(fd, *buf, BUF_SIZE, 0)) > 0){
	//	logString("Read data from fd:%d", fd);
	return readret;
      }
      //End gttp handling//
      memset(*buf,0,BUF_SIZE);
    } 
  if (readret == 0){//If connection closed
    //    logString("Connection closed on fd:%d",fd);
    close_socket(fd);
    FD_CLR(fd, master);
  }
  if (readret < 0)
    {
      for(j = listener + 1; j<=*fdmax; j++){
	close_socket(j);
      }
      //     logString("Error reading data from client");
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
  
  tv.tv_sec = 60;
  while (1)
    {
      //wait for a socket to have data to read
      sock = waitForAction(master,&read_fds, fdmax, tv, sockcont);/*blocking*/ 
      if (sock <0) continue;//select timeout
      if (sock == 0){//read through read_fs , start from beginning
	sockcont = 0;
	continue;
      }
      sockcont = sock;
      
      
      //Browser is requesting new connection
      if (sock == browserListener){ //new connection	
	if (stream == NULL){
	 
	}
	if (acceptBrowserServerConnectionToStream(browserListener, &master, &fdmax, stream, &commandLine) < 0)//add connection to stream
	  //add browser sock to read_fs
	  return EXIT_FAILURE;
	//if no current streams 
	
      }

      else {
	//Determine if coming from browser or server
	connection =  getConnectionFromSocket(stream, sock);
	if (connection == NULL){
	  
	}

	if (sock == connection->browser_sock){
	  ret = receive(sock, &master, &fdmax, browserListener, &buf);
	  if (ret > 0)
	    sendResponse(sock, buf, ret);
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
	  ret = receive(sock, &master, &fdmax, browserListener, &buf);
	  if (ret > 0)
	    sendResponse(sock, buf, ret);
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
