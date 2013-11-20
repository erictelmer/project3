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

#define CMNDLNSTRLEN 36

struct typedef{
  char log[CMDLNSTRLEN];
  float alpha;
  unsigned short listen_port;
  char fake_ip[CMDLNSTRLEN];
  char dns_ip[CMDLNSTRLEN];
  unsigned short dns_port;
  char www_ip[CMDLNSTRLEN];
}command_line_s;

int printHelp();

int parseCommandLine(int argc, char*argv[], command_line_s *commandLine);


#endif
