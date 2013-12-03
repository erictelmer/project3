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

#include "../orderedList/orderedList.h"

#define FILENAMESIZE 64

typedef struct ch_through{
  time_t time_started;
  time_t time_finished;
  unsigned int chunk_size;
  char chunk_name[FILENAMESIZE];
  struct ch_through *next;
}chunk_list_s;

typedef struct connecs{
  int browser_sock;
  int server_sock;
  chunk_list_s *chunk_throughputs;
  struct connecs *next;
}connection_list_s;

typedef struct strm_s{
  connection_list_s *connections;
  struct sockaddr_in client_addr;
  struct sockaddr_in server_addr;
  char filename[FILENAMESIZE];
  orderedList *available_bitrates;
  float alpha;
  unsigned int current_throughput;
}stream_s;

//new funcs

stream_s *newStream(struct sockaddr_in *client_addr, struct sockaddr_in *server_addr, float alpha);

chunk_list_s * newChunkList(void);

connection_list_s *newConnection(int browser_sock, int server_sock);
 
chunk_list_s *freeChunkList(chunk_list_s *chunkList);

void freeConnection(connection_list_s *connection);

connection_list_s *freeConnectionList(connection_list_s *connectionList);

stream_s *freeStream(stream_s *stream);

void addChunkToConnections(chunk_list_s *chunkList, connection_list_s *connectionList);

void addConnectionToStream(connection_list_s *connectionList, stream_s *stream);

void removeChunkFromConnections(chunk_list_s *chunkList, connection_list_s *connectionList);

void removeConnectionFromStream(connection_list_s *connectionList, stream_s *stream);

connection_list_s *getConnectionFromSocket(stream_s *stream, int sock);
