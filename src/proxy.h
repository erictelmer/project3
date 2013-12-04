/******************************************************************************
*                                                                             *
* proxy.h                                                                     *
*                                                                             *
* Description: This file contains the method declarations and 								*
*							 descriptions for a video streaming proxy based									*
*							 on the handout for Project 3.           												*
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
#include <sys/time.h>

#include "throughput_connections.h"
#include "command_line.h"
#include "log.h"

#define RAND_PORT 8088
#define SERV_PORT 8080
#define BUF_SIZE 200000

#define CHK_NULL(x) if ((x)==NULL) {logString("NULL ERROR"); exit (1);}
#define CHK_ERR(err,s) if ((err)==-1) { logString("%s error", s);perror(s); exit(1); }
#define CHK_SSL(err) if ((err)==-1) {  logString("SSL ERROR");exit(2); }
#define FREE(x,s) /*fprintf(stderr,"freeing %s @ %p\n",s,x);*/ free(x);
#define INC_POFFSET() port_offset++; if (port_offset > 1024) port_offset = 0;

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
// Parameters
// 		int sig: 
//
void sigINThandler(int sig);

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
// waitForAction: waits for action
// Parameters:
// 		fd_set master
// 		fd_set *read_fds
// 		int fdmax
// 		struct timeval tv
// 		int fdcont
// Returns:
// 		-1 if waiting, 0 if done waiting 		
//
int waitForAction(fd_set *master, 
		  fd_set *read_fds,
		  int fdmax,
		  struct timeval tv,
		  int fdcont);

// 
// acceptBrowserServerConnectionToStream: accepts browser server connection and adds it to the stream
// Parameters:
// 		int browserListener
// 		fd_set *master
// 		int *fdmax
// 		stream_s **stream
// 		command_line_s *commandLine
// Returns:
// 		0 on success
//
int acceptBrowserServerConnectionToStream(int browserListener, fd_set *master, int *fdmax, stream_s **stream, command_line_s *commandLine);

//
// receive: receives a stream of bytes
// Parameters:
// 		int fd
// 		fd_set *master
// 		int listener
// 		char (*buf)[BUF_SIZE]
// 		connection_list_s *connection
// 		stream_s *stream
// Returns:
// 		int: number of bytes read
//
int receive(int fd, fd_set *master, int *fdmax, int listener, char (* buf)[BUF_SIZE], connection_list_s *connection, stream_s *stream);


//
// sendResponse: sends a response to the client
// Parameters:
// 		int fd
// 		char *response
// 		int responselen
// Returns:
// 		0 on success
//
int sendResponse(int fd, char *response, int responselen);

//
// setupBrowserListenerSocket: sets up the browser listener socket for the connection
// Parameters:
// 		int *plistener
//		unsigned short port
// Returns:
// 		-1 on error, 0 on success
//
int setupBrowserListenerSocket(int *plistener, unsigned short port);


//
// replaceString: replaces a specific string in the buffer exactly where it belongs
// Parameters:
//		char *buf: buffer
//		unsigned int bufstrlen: length of the string in the buffer
//		char *str: string to insert into buffer
//		unsigned int index: place in the buffer to replace the string
//		unsigned int deleteLen: length of chars to write over in buffer
//		unsigned int replaceLen: length of chars to write from the string
// Returns:
// 		length of the new buffer
//
int replaceString(char *buf, unsigned int bufstrlen, char *str, unsigned int index, unsigned int deleteLen, unsigned int replaceLen);

// 
// startChunk: sets the time that a chunk is started
// 						 Called before sending a request
// Parameters:
// 		connection_list_s *connection
// 		char *chunkName
// Returns:
// 		0 on success
//
int startChunk(connection_list_s *connection, char *chunkName);

//
// finishChunk: finishes a chunk and calculates the current tpug
// Parameters:
// 		stream_s *stream
// 		connection_list_s *connection
// 		command_line_s commandLine
// Returns:
// 		0 on success
//
int finishChunk(stream_s *stream, connection_list_s *connection, command_line_s commandLine);
