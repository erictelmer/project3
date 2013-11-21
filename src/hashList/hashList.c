 /*******************************************************************
 * hashList.c                                                   	*
 *                                                             		*
 * Description: This file contains the functions for a hashList.	*
 *                                                              	*
 *******************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include "hashList.h"
#include "log.h"

//Temp Function -- used for debugging and printing bytes
void print_bytes(const void *object, size_t size)
{
  size_t i;

  printf("[ ");
  for(i = 0; i < size; i++)
  {
    printf("%02x ", ((const unsigned char *) object)[i] & 0xff);
  }
  printf("]\n");
}

int putTimeout(hashNode *node, struct tm *timeout){
	//Log(log, "Method call: putTimeout\n");

	if (node == NULL){
		//Log(log, "node is NULL. Returning FAILURE\n");
		return FAILURE;
	}
	if (timeout == NULL){
		//Log(log, "timeout is NULL. Returning FAILURE\n");
		return FAILURE;
	}

	memcpy(&(node->timeout), timeout, sizeof(struct tm));
	return SUCCESS;
}

int putAddr(hashNode *node, struct sockaddr_in *addr){

	if ((node == NULL) || (addr == NULL)) return FAILURE;

	memcpy(&(node->addr), addr, sizeof(struct sockaddr_in));
	return SUCCESS;
}

hashList *newHashList(){
  hashList *new = malloc(sizeof(hashList));
  new->start = NULL;
  new->size = 0;
  return new;
}

hashNode *newHashNode(char *chunkHash, unsigned int id){

  hashNode *new = malloc(sizeof(hashNode));

  if (new == NULL)
		return NULL;

  new->next = NULL;
  new->seqNum = -1;
  new->ackNum = -1;
  new->chunkOffset = id;
  // new->timeout = 0;
  new->windowSize = 8;
  new->upList = NULL;
  new->downList = NULL;
  //do addr stuff?

  if (chunkHash != NULL)
    memcpy(new->hashp, chunkHash, HASHSIZE);

  return new;
}

int appendNode(hashList *list, hashNode *node){
  
  if (findHashNode(list, node->hashp) == node)
    return SUCCESS;
		
  hashNode *x = list->start;
  
  if (x == NULL){
    list->start = node;
    list->size++;
    node->next = NULL;
    return SUCCESS;
  }
  
  while(x->next != NULL){
    x = x->next;
  }
  
  if (x->next != NULL) 
    return FAILURE;
  
  x->next = node;
  list->size++;
  
  return SUCCESS;
}

hashNode *findHashNode(hashList *list, char *chunkHash){

  hashNode *x = list->start;

  if (x == NULL)
	return NULL;
 
  if (memcmp(&x->hashp,chunkHash,HASHSIZE) == 0)
    return x;

  while(x->next != NULL){
    x = x->next;
    
    if (memcmp(&x->hashp,chunkHash,HASHSIZE) == 0)
      return x;
  }

  return NULL;
}

hashNode *findAddrNode(hashList *list, struct sockaddr_in *addr){
  hashNode *x = list->start;

  if (x == NULL) return NULL;
  

  if (((memcmp(&x->addr.sin_addr,&addr->sin_addr,sizeof(struct in_addr)) == 0) &&
       memcmp(&x->addr.sin_port,&addr->sin_port,sizeof(unsigned short)) == 0)){
		  return x;
  }

  while (x->next != NULL){
    x = x->next; 
    
    if (((memcmp(&x->addr.sin_addr,&addr->sin_addr,sizeof(struct in_addr)) == 0) &&
	 memcmp(&x->addr.sin_port,&addr->sin_port,sizeof(unsigned short)) == 0))
      return x;
  }
  return NULL;
}

int removeNode(hashList *list, hashNode *node){
  hashNode *x = list->start;

  if (x == NULL) return FAILURE;


  if (x == node){
    list->start = x->next;
    list->size--;
    return SUCCESS;
  }

  while(x->next != NULL){
    if (x->next == node){
      x->next = x->next->next;
      list->size--;
      return SUCCESS;
    }

    x = x->next;
  }

  return FAILURE;
}

int freeNode(hashNode *node){
  free(node);
  return SUCCESS;
}
 
int freeList(hashList *list){
  hashNode *x = list->start;
  hashNode *next;
  int i = 0;
  int size = list->size;

  free(list);
  while(x != NULL){
    printf("x = %p\n",x);
    next = x->next;
    freeNode(x);
    x = next;
    i++;
  }
  if (i != size)
    return FAILURE;
  return SUCCESS;
}

int getNextHashFromFile(FILE *fd, char *chunk_hash)
{
  char *line = NULL;
  size_t len = 0;
  ssize_t read;
  char *index;
  unsigned int id;
  // printf("Size long:%d, size int:%d\n",sizeof(partialHash),sizeof(i));
  //Log(log, "Getting next hash from file: hash = %s\n", chunk_hash);

  if ((read = getline(&line, &len, fd)) < 0){
    free(line);
    return FAILURE;
  }
  sscanf(line,"%d", &id);
  //The hash starts after ' '; copy current hash into chunk_hash
  index = strchr((const char *)line, ' ') + 1;
  hexCharsToBinary(index, chunk_hash, 2*HASHSIZE);

  //Log(log, "Copying hash into chunk_hash: hash = %s", chunk_hash);

  free(line);
  return id;

}

int hexCharsToBinary(char *str, char *bin, int strsize){
  int i;
  
  if (strsize%2 != 0) return FAILURE;
  memset(bin,0,strsize/2);
  for(i = 0;i < strsize; i++){
     bin[i/2] &= (0xff << (i%2));
    if(str[i] >= '0' && str[i] <= '9'){
      bin[i/2] |= (str[i] - '0') << (((i + 1) % 2)*4);
    }
    else if(str[i] >= 'a' && str[i] <= 'f'){
      bin[i/2] |= (str[i] - 'a' + 10)  << (((i + 1) % 2)*4);
     }
    else
      return FAILURE;
  }
  return SUCCESS;
}
