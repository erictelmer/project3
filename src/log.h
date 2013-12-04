/**************************************************************
 *                               															*
 * log.h																											*
 *																														*
 * Description: This file contains the method declarations		*
 * 							and descriptions for functions in log.c				*
 *																														*
 * Code reused and modified from Lauren's Project 1						*
 *																														*
 *************************************************************/
#ifndef _LOG_H_
#define _LOG_H_

#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <stdarg.h>
#include <inttypes.h>

extern int LogCreated;

//
// open_log: function to open a new log file
// Parameters:
// 		FILE *log: file for the log
//    path: path for file name
// Returns:
//    FILE *file: the logfile
//
FILE *open_log(FILE *log, const char *path);

//
// log_proxy: function to write to the proxy log file
// Parameters:
// 		file: proxy logfile to write to
// 		chunk_list_s *chunk
// 		stream_s *st
//
void log_proxy(FILE *log, chunk_list_s *chunk, stream_s *st, char *ser);

//
// log_dns: function to write to the dns log file
// Parameters:
// 		file: dns logfile to write to
// 		message: to write to the log file
//
void log_dns(FILE *file, const char *message, ...);

//
// log: function to write to a log file
//      Uses a variable list in order to write information/settings
//      to the log from clients
// Parameters:
//    message: to write to the log file
//    file : logfile to write to
//
void log_msg(FILE *file, const char *message, ...);

//
// close_log: function to close the log file
// Parameters:
//    file: the logfile
//
void close_log(FILE *file);

#endif
