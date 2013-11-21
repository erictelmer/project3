/********************************************************************
 * hashList.h														*
 *                                                              	*
 * Description: This file contains the parameters for a hashList   	*
 *              and the structures and method calls               	*
 *                                                              	*
 *******************************************************************/
#ifndef _HASHLIST_H_
#define _HASHLIST_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include "orderedList.h"
#include "log.h"

#define SUCCESS 1
#define FAILURE -1
#define HASHSIZE 20
#define LINESIZE 255

//hashNode Structure
typedef struct hNode
{
  int seqNum;//in the uploading context it is the last seq sent
  int ackNum;//in the uploading context it is the last ack received
  int windowSize;
  struct tm timeout;
  char hashp[HASHSIZE];
  unsigned int chunkOffset; //id in both downloading and uploading
  struct sockaddr_in addr;
  struct hNode *next;
  uploadList *upList;
  downloadList *downList;
}hashNode;

//hashList Structure
typedef struct
{
  FILE *fstream; //FILE stream to input or output file
  hashNode *start;
  int size;
}hashList;

//
// putTimeout: sets timeout to the current time
// Parameters:
//		hashNode *node: hasn node
//		struct tm *timeout: current time
// Returns:
//		int: SUCCESS or FAILURE
//
int putTimeout(hashNode *node, struct tm *timeout);

//
// putAddr: sets address of connection into node
// Parameters:
// 		hashNode *node: node
// 		struct sockaddr_in *addr: address of connection
// Returns: 
// 		int: SUCCESS or FAILURE
//
int putAddr(hashNode *node, struct sockaddr_in *addr);

//
// newHashList: creates a new hashList
//
hashList *newHashList();

// 
// newHashNode: creates a new hashNode with chunkHash and id
// Parameters: 
// 		char *chunkHash: chunkHash to be put into node
// 		unsigned int id: id of chunkHash
// Returns:
// 		hashNode* : new hashNode
//
hashNode *newHashNode(char *chunkHash, unsigned int id);

//
// appendNode: appends hashNode to end of list
// Parameters: 
// 		hashList *list: list of hashNodes
// 		hashNode *node: node to be appended
// Returns: 
// 		int: SUCCESS or FAILURE
//
int appendNode(hashList *list, hashNode *node);

//
// findHashNode: looks to see if chunkHash is in the hashNode list
// Parameters:
// 		hashList *list: hashList of nodes
// 		char *chunkHash: chunkHash to be found within list
// Returns: 
// 		hashNode*: node if found, NULL otherwise
//
hashNode *findHashNode(hashList *list, char *chunkHash);

//
// findAddrNode: looks to see if addr is in the HashNode list
// Parameters: 
// 		hashList *list: hashList of nodes
// 		struct sockaddr_in *addr: addr to be found within list
// Returns: 
// 		hashNode*: node if found, NULL otherwise
//
hashNode *findAddrNode(hashList *list, struct sockaddr_in *addr);

//
// removeNode: removes node from list but does not free node
// Parameters: 
// 		hashList *list: hashList of nodes
// 		hashNode *node: node to be removed
// Returns: 
// 		int: SUCCESS or FAILURE
//
int removeNode(hashList *list, hashNode *node);

//
// freeNode: frees the node
// Parameters: 
// 		hashNode *node: node to be freed
// Returns:
// 		int: SUCCESS
//
int freeNode(hashNode *node);

//
// freeList: frees the hashList
// Parameters: 
// 		hashList *list: hashList to be freed
// Returns: 
// 		int: SUCCESS or FAILURE
//
int freeList(hashList *list);

//
// getChunkId: gets ID for chunkHash from masterFile
// Parameters: 
// 		hashNode *node: hashNode
// 		const char *masterFile: masterFile of chunk hashes
//
void getChunkId(hashNode *node, const char *masterFile);

//
// getNextHashFromFile: gets the next hash from a file
// Parameters:
//      FILE *fd: hash chunk file
//      char *chunk_hash: desired hash
// Returns:
//      int: chunk_id upon success
//      	 FAILURE upon failure
//
int getNextHashFromFile(FILE *fd, char *chunk_hash);

//
// hexCharsToBinary: converts char string to binary
// Parameters:
// 		char *str: string to be converted
// 		char *bin: binary string pointer
// 		int strsize: string length
// Returns: 
// 		int: SUCCESS or FAILURE
//
int hexCharsToBinary(char *str, char *bin, int strsize);

#endif
