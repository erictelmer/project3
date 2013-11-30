#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../orderedList/orderedList.h"

//
// main: runs a simple test to check that we get the bitrates
// 

void main(int argc, char *argv[]){


	const char *file = "testXML.xml";
	orderedList *bitrateList = newOrderedList();
	orderedList *blist1 = newOrderedList();
	orderedList *blist2 = newOrderedList();
	orderedList *blist3 = newOrderedList();
	orderedList *blist4 = newOrderedList();
	orderedList *blist5 = newOrderedList();

	int x = parseXML(file, bitrateList);
	if (isOListEmpty(bitrateList) == 1){
		printf("Failed to find bitrate in XML file");
	}
	printf("---- Test 1: parseXML ----\n");
	printf("\nPrinting out bitrates in list from file\n");
	printf("listSize: %d\n", bitrateList->size);
	while(bitrateList->start != NULL){
			printf("bitrate: %d\n", bitrateList->start->num);
			bitrateList->start = bitrateList->start->next;
	}

	printf("\n\n---- Test 2: getBitrate ----\n");
	int y = parseXML(file, blist1);
	int t1 = getBitrate(1500, blist1);
	 y = parseXML(file, blist2);
	int t2 = getBitrate(15, blist2);
	 y = parseXML(file, blist3);
	int t3 = getBitrate(10, blist3);
	 y = parseXML(file, blist4);
	int t4 = getBitrate(15000, blist4);
	 y = parseXML(file, blist5);
	int t5 = getBitrate(1050, blist5);

	printf("1: tput: 1500, maxBR: 1000, br: %d\n", t1);
  printf("2: tput: 15, maxBR: 10, br: %d\n", t2);
  printf("3: tput: 10, maxBR: 6.66, br: %d\n", t3);
  printf("4: tput: 15000, maxBR: 10000, br: %d\n", t4);
  printf("5: tput: 1050, maxBR: 700, br: %d\n", t5);


}
