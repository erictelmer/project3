#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../src/log.h"

//
// main: runs a simple test to check the logger functions
//

void main(int argc, char* argv[]){
  int i = 2;
  FILE *log;

  log = open_log(log, "logger.txt");
  log_msg(log, "Test 1 \n");
  log_msg(log, "Test %d \n", i);
  close_log(log);

  exit(0);
}                
