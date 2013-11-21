 /***********************************************************************
 * orderedList.c                                                       	*
 *                                                             		    *
 * Description: This file contains the functions for an orderedList.	*
 *                                                                  	*
 ***********************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include "orderedList.h"


/* Function to parse XML for available bitrates */
// takes in an XML file and an empty orderedList
int parseXML(const char *file, orderedList *list){
	FILE *fp;

		if (file == NULL)
		{
				printf("XML File is NULL\n");
				return -1;
		}
		if (list == NULL)
		{
				printf("Bitrate List is null");
				return -1;
		}

		fp = fopen(file, "r");
		char line[1024];
		char *num;
		while (fgets(line, sizeof(line), fp)){
			if (strstr(line, "bitrate=")) //found bitrate in a line
				{
					num = strchr(line, '"'); //num = "###"
					num = num + 1;					 //num = ###"
					num = strtok(num, "\""); //num = ###
					addNum(list, atoi(num));			 //add bitrate to bitrateList
				}
			}

		return 1;
}

// Function to find the best bitrate based on throughput
// Takes in the bitrateList and calculated throughput
int getBitrate(int tput, orderedList *list){

	orderedList *blist = list;
	//calculate highest bitrate connection can support
	int maxBR = tput/1.5;
	int br = 0;

	while(blist->start != NULL && blist->start->num <= maxBR){	
			if(blist->start->num > br){
				br = blist->start->num;
				blist->start = blist->start->next;
			}
	}

	return br;

}


int getTimeoutPacket(uploadList *list, double secs){

  orderedNode *node;
  time_t timeNow;
  time(&timeNow);
  node = list->sent->start;
  
  while(node != NULL){
    
    if (difftime(timeNow, node->timeSent) >= secs)
      return node->num;
    
    node = node->next;
  }
  
  return -1;
}

int receivedACK(uploadList *list, int ack){
	int min = list->sent->start->num;
	int i;
	
	if (ack == min - 1){
		list->dup++;
		
		if (list->dup == 3)
			return LOST;
			
		return OK;
	}
	
	
	for(i = min; i <= ack; i++){
		removeNum(list->sent,i);
	}
	
	list->dup = 0;
	return OK;
}

void sentDATA(uploadList *list, int seq){
	addNum(list->sent, seq);
}

int receivedDATA(downloadList *list, int seq){
	int i;
	
	if(seq == list->maxRecv + 1){
		list->maxRecv = seq;
	} else if (seq > list->maxRecv + 1){
		for (i=list->maxRecv + 1;i<seq;i++){
			addNum(list->missing, i);
		}
		
		list->maxRecv = seq;
	} else {
		removeNum(list->missing, seq);
	}

  return isOListEmpty(list->missing) ? list->maxRecv : list->missing->start->num - 1;
}

int isOListEmpty(orderedList *list){
	if (list->start == NULL)
		return 1;
	else
		return 0;
}

downloadList * newDownloadList(){
	downloadList * new = malloc(sizeof(downloadList));
	new->maxRecv = 0;
	new->missing = newOrderedList();
	return new;
}

uploadList * newUploadList(){
	uploadList * new = malloc(sizeof(uploadList));
	new->dup = 0;
	new->sent = newOrderedList();
	return new;
}

orderedList *newOrderedList(){
	orderedList *list = malloc(sizeof(orderedList));
	list->start = NULL;
	list->size = 0;
	return list;
}

orderedNode *newOrderedNode(int num){
	orderedNode *node = malloc(sizeof(orderedNode));
	node->num = num;
	// node->count = 0;
	time(&node->timeSent);
	node->next = NULL;
	return node;
}

void freeOrderedNode(orderedNode *node){
	free(node);
}

void removeNum(orderedList *list, int num){
	orderedNode *x;
	orderedNode *tmp;
	if (list->size == 0)
		return;
		
	x = list->start;
	if (x == NULL) return;
	
	if (x->num == num){
		list->start = x->next;
		freeOrderedNode(x);
		list->size--;
		return ;
	}

	while(x->next != NULL){
		if(x->next->num == num){
			tmp = x->next;
			x->next = x->next->next;  
			freeOrderedNode(tmp);
			list->size--;
			return;
		}

		x = x->next;
	}
	
	return;
}

void addNum(orderedList *list, int num){
	orderedNode *x;
	orderedNode *new;
	
	if (list->start == NULL){
		//if list is empty
		list->start = newOrderedNode(num);
		list->size++;
		return;
	}
	
	if (num < list->start->num){
		//if num is the new min
		x = newOrderedNode(num);
		x->next = list->start;
		list->start = x;
		list->size++;
		return;
	}

	if (num == list->start->num){
	  time(&list->start->timeSent);
	  return;
	}

	x = list->start;

	while(x->next != NULL){
		if (num < x->next->num){
			//if num < the next num it belongs here
			new = newOrderedNode(num);
			new->next = x->next;
			x->next = new;
			list->size++;
			return;
		}

		if (num == x->next->num){
		  //if this num is already in list, update the timeout
		  time(&x->next->timeSent);
		  return;
		}
		
		x = x->next;
	}

	//x is the last node
	new = newOrderedNode(num);
	x->next = new;
	list->size++;
}

void freeOrderedList(orderedList *list){
	orderedNode *ahead;
	orderedNode *dead;
	
	if (list == NULL) return;
	
	ahead = list->start;
	
	while(ahead != NULL){
		dead = ahead;
		ahead = ahead->next;
		freeOrderedNode(dead);
	}
	
	free(list);
}

void freeDownloadList(downloadList * list){
	if (list == NULL) return;
	freeOrderedList(list->missing);
	free(list);
}

void freeUploadList(uploadList * list){
	if (list == NULL) return;
	freeOrderedList(list->sent);
	free(list);
}
