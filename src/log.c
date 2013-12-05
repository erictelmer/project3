/******************************************************************************
 * 						                                                      		      *
 *  log.c      				                                                 	      *
 *                                                              							*
 *  Description: This file contains the description and methods for						*
 *  		 creating a log file and writing to it with the expected							*
 *  		 format as specified in the handout for Project 3											*
 *																																						*
 *  Code reused and modified from Lauren's Project 1													*
 *                                                              							*
 *****************************************************************************/

#include "throughput_connections.h"
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


void log_proxy(FILE *log, chunk_list_s *chunk, stream_s *st, char *ser){

  time_t rawtime;
  time(&rawtime);

	int cur = chunk->time_finished.tv_sec;

  //calculate duration
  double dur = ((chunk->time_finished.tv_sec - chunk->time_started.tv_sec) * 1000000.0
             + (chunk->time_finished.tv_usec - chunk->time_started.tv_usec)) / 1000000.0;

  //calculate throughput for current chunk
  unsigned int tput = (chunk->chunk_size / dur)*(8.0/1000);
  
  //current EWMA tput estimate in Kbps
  double avg = st->current_throughput;
  
  //bitrate
  int br = getBitrate(st->current_throughput, st->available_bitrates);
  
  //server-ip: given as *ser

  //Print log
  fprintf(log, "%d\t%f\t%d\t%f\t%d\t%s\t%s\n", cur, dur, tput, avg, br, ser, chunk->chunk_name);
  fflush(log);
}

// When DNS Server works, this will be modified to log correctly.
// Behaves exactly as log_msg
void log_dns(FILE *log, const char *message, ...){
	time_t rawtime; 
	//struct tm *timeinfo;
  time(&rawtime);

  //Print the current time in seconds since the epoch
  fprintf(log, "%lu\t", (uintmax_t)&rawtime);
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
	strftime(buffer, 80, "[%x %X]: ", timeinfo);

	fprintf(log, buffer);
	fflush(log);
	va_list arg_point;
	va_start(arg_point, message);
	vfprintf(log, message, arg_point);
	va_end(arg_point);
	fflush(log);
}

void close_log(FILE *log){
	fclose(log);
}
