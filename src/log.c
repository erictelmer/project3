/******************************************************************************
 *                                                              							*
 *  log.c                                                       							*
 *                                                              							*
 *  Description: This file contains the description and methods for						*
 *  						 creating a log file and writing to it with the expected			*
 *  						 format as specified in the handout for Project 3							* *																																						 *
 *																																						*
 *  Code reused and modified from Lauren's Project 1													*
 *                                                              							*
 *****************************************************************************/

#include "log.h"

FILE *open_log(FILE *log, const char *path){
	log = fopen(path, "w");

	if (log == NULL){
		fprintf(stdout, "Error creating logfile. \n");
		exit(EXIT_FAILURE);
	}

	/*Set buffering for file to be new line*/
	setvbuf(log, NULL, _IOFBF, 0);
	return log;
}

void log_proxy(FILE *log, const char *message, ...){
  time_t rawtime;
  struct tm *timeinfo;
  char buffer[80];

  time(&rawtime);

  //Print the current time in seconds since the epoch
  fprintf(log, "%llu\t", (uintmax_t)&rawtime);
  fflush(log);
  va_list arg_point;
  va_start(arg_point, message);
  vfprintf(log, message, arg_point);
  va_end(arg_point);
}

void log_dns(FILE *log, const char *message, ...){
	time_t rawtime; 
	struct tm *timeinfo;
  char buffer[80];

  time(&rawtime);

  //Print the current time in seconds since the epoch
  fprintf(log, "%llu\t", (uintmax_t)&rawtime);
  fflush(log);
  va_list arg_point;
  va_start(arg_point, message);
  vfprintf(log, message, arg_point);
  va_end(arg_point);
}

void log_msg(FILE *log, const char *message, ...){
	time_t rawtime;
	struct tm *timeinfo;
	char buffer[80];

	time(&rawtime);
	timeinfo = localtime(&rawtime);
	strftime(buffer, 80, "[%x %X] ", timeinfo);

	fprintf(log, buffer);
	fflush(log);
	va_list arg_point;
	va_start(arg_point, message);
	vfprintf(log, message, arg_point);
	va_end(arg_point);
}

void close_log(FILE *log){
	fclose(log);
}
