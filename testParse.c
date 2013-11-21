#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "orderedList/orderedList.h"

//
// main: runs a simple test to check that we get the bitrates
// 

void main(int argc, char *argv[]){


	const char *file = "testXML.xml";
	orderedList *bitrateList = newOrderedList();

	int x = parseXML(file, bitrateList);
	if (isOListEmpty(bitrateList) == 1){
		printf("Failed to find bitrate in XML file");
	}
	printf("\nPrinting out bitrates in list from file\n");
	printf("listSize: %d\n", bitrateList->size);
	while(bitrateList->start != NULL){
			printf("bitrate: %d\n", bitrateList->start->num);
			bitrateList->start = bitrateList->start->next;
	}
	

}
