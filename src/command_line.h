/********************************************************************************
 *                                                                            	*
 *  command_line.h                                                            	*
 *                                                                            	*
 *  Description: This file contains the method declarations and			*
 *		 description for functions in command_line.c                  	*
 *                                                                            	*
 *******************************************************************************/

#ifndef _COMMAND_LINE_H
#define _COMMAND_LINE_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define CMNDLNSTRLEN 36

typedef struct{
  char logfile[CMNDLNSTRLEN];
  float alpha;
  unsigned short listen_port;
  struct in_addr fake_ip;
  struct in_addr dns_ip;
  unsigned short dns_port;
  struct in_addr www_ip;
}command_line_s;

//
// printHelp: prints out the usage for the proxy 
// 	      with descriptions for each argument
//
int printHelp();

//
// parseCommandLine: parses the command line args into structs
// 		     for use in the proxy
// Parameters:
// 	int argc: the number of arguments
// 	char *argv[]: the command line args
// 	command_line_s *commandLine: the commandLine struct
int parseCommandLine(int argc, char *argv[], command_line_s *commandLine);

#endif
