/******************************************************************************
*                                                                             *
* proxy.h                                                                     *
*                                                                             *
* Description: This file contains the method declarations and 								*
*							 descriptions for a video streaming proxy based									*
*							 on the handout for Project 3.           												*
*                                                                             *
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
#include "log.h"

//#include <openssl/rsa.h>
//#include <openssl/crypto.h>
//#include <openssl/x509.h>
//#include <openssl/pem.h>
//#include <openssl/ssl.h>
//#include <openssl/err.h>
//#include "generateResponse.h"

#define RAND_PORT 8088
#define SERV_PORT 8080
#define BUF_SIZE 4096

#define CHK_NULL(x) if ((x)==NULL) {logString("NULL ERROR"); exit (1);}
#define CHK_ERR(err,s) if ((err)==-1) { logString("%s error", s);perror(s); exit(1); }
#define CHK_SSL(err) if ((err)==-1) {  logString("SSL ERROR");exit(2); }
#define FREE(x,s) /*fprintf(stderr,"freeing %s @ %p\n",s,x);*/ free(x);

//
// close_socket: closes the socket
// Parameters:
// 		int sock: socket to be closed
// Returns:
// 		0 on SUCCESS and 1 on FAILURE
// 		
int close_socket(int sock);

//
// sigINThandler: closes sig 3 
// Parameters:
// 		int sig: 
//
void sigINThandler(int sig);

//
// leave: 
//
void leave();

//
// Malloc: malloc's memory of size
// Parameters:
// 		size_t size
// 		char *name
//
void * Malloc(size_t size, char *name);

//
// Realloc: realloc's memory of size
// Parameters:
// 		void *pointer
// 		size_t size
// 		char *name
//
void * Realloc(void *pointer, size_t size, char *name);

//
// waitForAction: 
// Parameters:
// 		fd_set master
// 		fd_set *read_fds
// 		int fdmax
// 		struct timeval tv
// 		int fdcont
// Returns:
// 		
//
int waitForAction(fd_set *master, 
		  fd_set *read_fds,
		  int fdmax,
		  struct timeval tv,
		  int fdcont);

// 
// acceptBrowserServerConnectionToStream: 
// Parameters:
//
// Returns:
//
//
int acceptBrowserServerConnectionToStream(int browserListener, fd_set *master, int *fdmax, stream_s **stream, command_line_s *commandLine);

//
// receive: 
// Parameters:
// Returns:
//
//
int receive(int fd, fd_set * master, int *fdmax, int listener, char (* buf)[BUF_SIZE], connection_list_s *connection, stream_s *stream);


//
// sendResponse: 
// Parameters:
// Returns:
//
//
int sendResponse(int fd, char *response, int responselen);

//
// setupBrowserListenerSocket
// Parameters:
// Returns:
//
int setupBrowserListenerSocket(int *plistener, unsigned short port);
