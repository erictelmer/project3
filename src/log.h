/*****************************************************************************
 *                               																						 *
 * log.h																																		 *
 *																																					 *
 * Description: This file contains the parameters for log.c									 *
 *																															             *
 * Code reused and modified from Lauren's Project 1							             *
 *																																					 *
 ****************************************************************************/
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
//    path: path for file name
// Returns:
//    FILE *file: the logfile
//
FILE *open_log(const char *path);

//
// Log: function to write to a log file
//      Uses a variable list in order to write information/settings
//      to the log from clients
// Parameters:
//    message: to write to the log file
//    file : logfile to write to
//
void Log(FILE *file, const char *message, ...);

//
// close_log: function to close the log file
// Parameters:
//    file: the logfile
//
void close_log(FILE *file);

#endif
