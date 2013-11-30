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

static FILE *log;
int LogCreated = 0;

FILE *open_log(const char *path){
i	/*Check that we haven't already created the log*/
	if (LogCreated == 1){
		log = fopen(path, "a");
	} else{
		log = fopen(path, "w");
		LogCreated = 1;
	}

	if (log == NULL){
		fprintf(stdout, "Error creating logfile. \n");
		LogCreated = 0;
		exit(EXIT_FAILURE);
	}

	/*Set buffering for file to be new line*/
	setvbuf(log, NULL, _IOFBF, 0);
	return log;
}

void Log(FILE *log, const char *message, ...){
	time_t rawtime;
	struct tm *timeinfo;
	char buffer[80];

	time(&rawtime);
		//	timeinfo = localtime(&rawtime);
		//	strftime(buffer, 80, "[%x %X] ", timeinfo);

	//Print the current time in seconds since the epoch
	fprintf(log, "%llu\t", (uintmax_t)&rawtime);
	fflush(log);
	va_list arg_point;
	va_start(arg_point, message);
	vfprintf(log, message, arg_point);
	va_end(arg_point);
}

void close_log(FILE *log){
	fclose(log);
}
