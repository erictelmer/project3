/*
 * commandLine.h
 *
 * Command Line parsing functions

 */

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

int printHelp();

int parseCommandLine(int argc, char*argv[], command_line_s *commandLine);


#endif
