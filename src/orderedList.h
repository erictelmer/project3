/**********************************************************************
 * orderedList.h                                                      *
 *                                                                  	*
 * Description: This file contains the parameters for an orderedList	*
 *              and the structures and method calls                 	*
 *                                                                  	*
 *********************************************************************/
#ifndef _ORDEREDLIST_H_
#define _ORDEREDLIST_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define LOST -1
#define OK 1

//orderedNode Structure
typedef struct oNode
{
  int num;
  time_t timeSent; //Upload use only
  struct oNode *next;
}orderedNode;

//orderedList Structure
typedef struct oList
{
  int size;
  orderedNode *start;
}orderedList;

//downloadList Structure
typedef struct downL
{
  int maxRecv;
  orderedList *missing;
}downloadList;

//uploadList Structure
typedef struct upL
{
  orderedList *sent;
  int dup;
}uploadList;


//
// parseXML: parses the XML for available bitrates
// Parameters:
// 		char* file: buffer containing the XML file
// 		orderedList *list: empty orderedList
// Returns:
// 		-1 on ERROR, 1 on SUCCESS
//
int parseXML(const char *file, orderedList *list);

// 
// getBitrate: function to find the best bitrate based on throughput
// Parameters:
// 		int tput: max tput
// 		orderedList *list: bitrates list
// Returns:
// 		int: max bitrate available
int getBitrate(int tput, orderedList *list);
				

//
// getTimeoutPacket: gets the seqNum of a packet that has timed out
// Parameters:
// 		uploadList *list
// 		double secs: current time
// Returns:
// 		int: seqNum of packet if timed out, otherwise -1
//
int getTimeoutPacket(uploadList *list, double secs);

//
// receivedACK: returns LOST if 3 duplicated ACKSs, otherwise ok
// Parameters: 
// 		uploadList *list
// 		int ack: acknowledgement number
// 	Returns:
// 		int: LOST if 3 duplicate ACKS, otherwise OK
//
int receivedACK(uploadList *list, int ack);

//
// sendDATA: sends the data from seq
// Parameters:
// 		uploadList *list
// 		int seq: sequence number of data to send
//
void sentDATA(uploadList *list, int seq);

//
// receivedDATA: sends an acknowledgement to sender
// Parameters:
// 		downloadList *list
// 		int seq: sequence number to send acknowledgement
// Returns: 
// 		int: acknowledgement number
//
int receivedDATA(downloadList *list, int seq); 

//
// isOListEmpty: checks if an ordered list is empty
// Parameters:
// 		orderedList *list
// Returns:
// 		1 if NULL, 0 otherwise
//
int isOListEmpty(orderedList *list);

//
// newDownlostList: creates a new download list
// Returns:
// 		an empty downloadList
//
downloadList * newDownloadList();

//
// newUploadList: creates a new upload list
// Returns:
// 		an empty uploadList
//
uploadList * newUploadList();

//
// newOrderedList: creates a new ordered list
// Returns:
// 		an empty orderedList
//
orderedList *newOrderedList();

//
// newOrderedNode: creates a new ordered node
// Parameters:
// 		int num: number of the ordered node
// Returns:
// 		an orderedNode with number num
//
orderedNode *newOrderedNode(int num);

//
// freeOrderedNode: frees the given ordered node
// Parameters:
// 		orderedNode *node: node to be freed
//
void freeOrderedNode(orderedNode *node);

//
// removeNum: removes the ordered node of num from the list
// Parameters:
// 		orderedList *list
// 		int num: node to be removed
void removeNum(orderedList *list, int num);

//
// addNum: adds the number to the orderedList
// Parameters:
// 		orderedList *list
// 		int num: number to be added to the list
void addNum(orderedList *list, int num);

//
// freeOrderedList: frees an orderedList
// Parameters: 
// 		orderedList *list
//
void freeOrderedList(orderedList *list);

//
// freeDownloadList: frees a downloadList
// Parameters: 
// 		download List *list
//
void freeDownloadList(downloadList *list);

// freeUploadList: frees an uploadList
// Parameters:
// 		upload List *list
void freeUploadList(uploadList *list);

#endif
