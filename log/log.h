/****************************************************************
 *                               								*
 * log.h														*
 *																*
 * Description: This file contains the parameters for log.c		*
 *																*
 * Code reused from Lauren's Project 1							*
 *																*
 ***************************************************************/
#ifndef _LOG_H_
#define _LOG_H_

#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <stdarg.h>

extern int LogCreated;

FILE *open_log(const char *path);
void Log(FILE *file, const char *message, ...);
void close_log(FILE *file);

#endif
