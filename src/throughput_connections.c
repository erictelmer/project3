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

#include "throughput_connections.h"

#define CHK_NULL(x,s) if (x == NULL) {printf("Passed func %s NULL\n", s); return;}
#define CHK_NULLR(x,s) if (x == NULL) {printf("Passed func %s NULL\n", s); return NULL;}

stream_s *newStream(struct sockaddr_in *client_addr, struct sockaddr_in *server_addr, float alpha){
  
  stream_s *new = malloc(sizeof(struct strm_s));
  memcpy(&new->client_addr, client_addr, sizeof(struct sockaddr_in)); 
  memcpy(&new->server_addr, server_addr, sizeof(struct sockaddr_in)); 
  new->alpha = alpha;

  memset(new->filename, 0, FILENAMESIZE);
  new->connections = NULL;
  new->available_bitrates = newOrderedList();
  parseXML(new->available_bitrates);
  new->current_throughput = -1;
  
  return new;
}

chunk_list_s *newChunkList(){
  chunk_list_s *new = malloc(sizeof(struct ch_through));

  new->next = NULL;
  
  return new;
}


connection_list_s *newConnection(int browser_sock, int server_sock){
  connection_list_s *new = malloc(sizeof(struct connecs));
  
  new->browser_sock = browser_sock;
  new->server_sock = server_sock;
  
  new->chunk_throughputs = NULL;
  new->next = NULL;

  return new;
}

chunk_list_s *freeChunkList(chunk_list_s *chunkList){
  chunk_list_s *next;
  chunk_list_s *curr = chunkList;
  
  while(curr != NULL){
    next = curr->next;
    free(curr);
    curr = next;
  }

  return NULL;
}

void freeConnection(connection_list_s *connection){
  free(connection);
}

connection_list_s *freeConnectionList(connection_list_s *connectionList){

  connection_list_s * curr = connectionList;
  connection_list_s * next;
  
  
  while(curr != NULL){
    next = curr->next;
    if(curr->chunk_throughputs != NULL)
      freeChunkList(curr->chunk_throughputs);
    freeConnection(curr);
    curr = next;
  }

  return NULL;
}

stream_s *freeStream(stream_s *stream){
  CHK_NULLR(stream, "freeStream");

  freeConnectionList(stream->connections);
  freeOrderedList(stream->available_bitrates);
  
  free(stream);
  
  return NULL;
} 


void addChunkToConnections(chunk_list_s *chunkList, connection_list_s *connectionList){
  CHK_NULL(chunkList, "addChunk:ch"); CHK_NULL(connectionList, "addChunk:cpnn");
  
  chunk_list_s *curr;

  if (connectionList->chunk_throughputs == NULL){//no chunks in list yet
    connectionList->chunk_throughputs = chunkList;
    return;
  } 
  
  curr = connectionList->chunk_throughputs;
  
  while(curr->next != NULL){//set curr = last chunk
    curr = curr->next;
  }
  
  curr->next = chunkList;
  return;
  
}

void addConnectionToStream(connection_list_s *connectionList, stream_s *stream){
  CHK_NULL(connectionList, "addConnec:conn"); CHK_NULL(stream, "addConnec:s");

  connection_list_s *curr;

  if (stream->connections == NULL){//no connections in list yet
    stream->connections = connectionList;
    return;
  }
  
  curr = stream->connections;

  while(curr->next != NULL){//set cur last connection
    curr = curr->next;
  }

  curr->next = connectionList;
  return;

}

void removeChunkFromConnections(chunk_list_s *chunkList, connection_list_s *connectionList){
  CHK_NULL(chunkList, "rmChnk:ch"); CHK_NULL(connectionList,"rmChnk:con");
  
  chunk_list_s *curr;
	connectionList->chunk_throughputs = NULL;
/*
  if (connectionList->chunk_throughputs == NULL){//no chunks in list
    return;
  } 
  
  //Check if first node is chunk
  curr = connectionList->chunk_throughputs;
  if (curr == chunkList){
    connectionList->chunk_throughputs = curr->next;
    return;
  }

  //find either to last node, or node before desired one
  while(curr->next != NULL && curr->next != chunkList){
    curr = curr->next;
  }
  //if node is last, we didn't find it
  if (curr->next == NULL) return;

  //if next node is desired one
  if(curr->next == chunkList){
    curr->next = curr->next->next;
  }
  */
  return;
}

void removeConnectionFromStream(connection_list_s *connectionList, stream_s *stream){
  CHK_NULL(stream,"rmConnec:s"); CHK_NULL(connectionList,"rmConnec:c");
  
  connection_list_s *curr;

  if (stream->connections == NULL){//no connections in list
    return;
  } 
  
  //Check if first node is connection
  curr = stream->connections;
  if (curr == connectionList){
    stream->connections = curr->next;
    return;
  }

  //find either to last node, or node before desired one
  while((curr->next != NULL) && (curr->next != connectionList)){
    curr = curr->next;
  }
  //if node is last, we didn't find it
  if (curr->next == NULL) return;

  //if next node is desired one
  if(curr->next == connectionList){
    curr->next = curr->next->next;
  }
  
  return;
}


connection_list_s *getConnectionFromSocket(stream_s *stream, int sock){
  CHK_NULLR(stream,"getConnecfrmS:s"); CHK_NULLR(stream->connections,"getConnecFrmS:c");

  connection_list_s *curr = stream->connections;

  while((curr->browser_sock != sock) && (curr->server_sock != sock)){
    curr = curr->next;
    if (curr == NULL)
      return NULL;
  }
  return curr;

}


