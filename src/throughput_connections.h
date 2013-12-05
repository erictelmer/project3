/****************************************************************
 *                                                            	*
 * throughput_connections.h                                   	*
 *                                                            	*
 * Description: This file contains the method declarations    	*
 *              and descriptions for functions in 	   	*
 *              throughput_connections.c			*
 *                                                            	*
 ***************************************************************/
#include <assert.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <time.h>

#include "orderedList.h"

#define FILENAMESIZE 64

// Struct for a chunk request
typedef struct ch_through{
  struct timeval time_started;
  struct timeval time_finished;
  unsigned int chunk_size;
  unsigned int bytesLeft;
  char chunk_name[FILENAMESIZE];
  struct ch_through *next;
}chunk_list_s;

// Struct for connections
typedef struct connecs{
  int browser_sock;
  int server_sock;
  chunk_list_s *chunk_throughputs;
  struct connecs *next;
}connection_list_s;

// Struct for a stream
typedef struct strm_s{
  connection_list_s *connections;
  struct sockaddr_in client_addr;
  struct sockaddr_in server_addr;
  char filename[FILENAMESIZE];
  orderedList *available_bitrates;
  float alpha;
  unsigned int current_throughput;
}stream_s;

//
// newStream: creates a new stream struct
// Returns:
//		stream_s: the new stream
//
stream_s *newStream(struct sockaddr_in *client_addr, struct sockaddr_in *server_addr, float alpha);

//
// newChunkList: creates a new chunkList
// Returns:
// 		chunk_list_s: the new chunk
//
chunk_list_s * newChunkList(void);

//
// newConnection: creates a new connection from browser/server
// Parameters:
// 		int browser_sock
// 		int server_sock
// Returns:
// 		connection_list_s
//
connection_list_s *newConnection(int browser_sock, int server_sock);
 
//
// freeChunkList: frees the chunkList
// 	note: there is only ever 1 chunk in the list
// Parameters:
// 		chunk_list_s: chunk to be freed
// Returns:
// 		chunk_list_s: chunkList with freed chunk
//
chunk_list_s *freeChunkList(chunk_list_s *chunkList);

//
// freeConnection: frees the connection_list_s struct
// Parameters:
// 		connection_list_s: connection to be freed
//
void freeConnection(connection_list_s *connection);

//
// freeConnectionList: frees all of the connections in a connection_list_s struct
// Parameters:
// 		connection_list_s: connectionList to be freed
// Returns:
// 		connection_list_s: freed connectionList
//
connection_list_s *freeConnectionList(connection_list_s *connectionList);

//
// freeStream: frees the stream_s struct
// Parameters:
// 		stream_s: stream to be freed
// Returns:
// 		stream_s: freed stream
//
stream_s *freeStream(stream_s *stream);

//
// addChunkToConnections: adds the chunk to the connection list
// Parameters:
// 		chunk_list_s: chunk to be added to connection list
// 		connection_list_s: connection
//
void addChunkToConnections(chunk_list_s *chunkList, connection_list_s *connectionList);

//
// addConnectionToStream: adds the connection to the stream
// Parameters:
// 		connection_list_s: connectionList to be added to the stream
// 		stream_s: stream of connections
//
void addConnectionToStream(connection_list_s *connectionList, stream_s *stream);

//
// removeChunkFromConnections: removes the chunk from the connection list
// Parameters:
// 		chunk_list_s: chunk to be removed from the connection list
// 		connection_list_s: connection list of chunks
//
void removeChunkFromConnections(chunk_list_s *chunkList, connection_list_s *connectionList);

// 
// removeConnectionFromStream: removes the connection from the stream
// Parameters:
// 		connection_list_s: connectionList to be removed from the stream
// 		stream_s: stream of connections
//
void removeConnectionFromStream(connection_list_s *connectionList, stream_s *stream);

//
// getConnectionFromSocket: function to get the connection from a stream list
// 												  based on a socket number
// Parameters:
// 		int sock: socket that connection is on
//		stream_s: list of all of the connections
// Returns:
// 		connection_list_s: connection on the socket
//
connection_list_s *getConnectionFromSocket(stream_s *stream, int sock);
