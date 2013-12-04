/******************************************************************************
* proxy.c				                                                              *
*                                                                             *
* Description: This file contains the C source code for 											*
* 						 a video streaming proxy  																		  *
*                                                                             *
* Authors:Eric Telmer <eit@andrew.cmu.edu>                                    *
* 				Lauren Milisits <lvm@andrew.cmu.edu>																*
*                                                                             *
*******************************************************************************/

#include "proxy.h"

static FILE *log, *p_log, *dns_log;
int port_offset;

int close_socket(int sock){
  if (close(sock)){
    log_msg(log, "Failed closing socket..\n");
        return 1;
  }
  return 0;
}

void sigINThandler(int sig){
	close_socket(3);
	exit(1);
}

void * Malloc(size_t size, char *name){
  void *loc;
  loc = malloc(size);
  if (loc == NULL){
    log_msg(log, "Error: Malloc on %s\n", name);
    exit(3);
  }
  return loc;
}

void * Realloc(void *pointer, size_t size, char *name){
  void *loc;
  loc =  realloc(pointer,size);
  if (loc == NULL){
    log_msg(log, "Error: Realloc on %s\n", name);
    exit(3);
  }
  return loc;
}

int waitForAction(fd_set *master, fd_set *read_fds, int fdmax, struct timeval tv, int fdcont){

  int i;

  *read_fds = *master; // copy it

  tv.tv_sec = 60;
  i = select(fdmax+1, read_fds, NULL, NULL, &tv);
  if (i == -1) {
    perror("select");
    exit(4);
  }
  if (i == 0){
    printf("I'm waiting...\n");
    return -1;
  }
  //L1
  for(; fdcont<=fdmax; fdcont++){
    if (FD_ISSET(fdcont, read_fds)){ //socket ready
      return fdcont;
    }//End fd isset
  }//End for loop
  return 0;
}

int acceptBrowserServerConnectionToStream(int browserListener, fd_set * master, int * fdmax, stream_s **stream, command_line_s *commandLine){
  socklen_t addr_size;

  struct sockaddr_in browser_addr;
  connection_list_s *connection = NULL;

  int browser_sock;
  int server_sock;
  struct sockaddr_in client_addr;
  struct sockaddr_in server_addr;

  char str[50];
  addr_size = sizeof(browser_addr);

  log_msg(log, "Attempting to accept connection\n");

  if(*stream == NULL){//new stream!
    memcpy(&client_addr.sin_addr, &commandLine->fake_ip, sizeof(struct in_addr));
    memcpy(&server_addr.sin_addr, &commandLine->www_ip, sizeof(struct in_addr));
    client_addr.sin_family = AF_INET;
    server_addr.sin_family = AF_INET;
    client_addr.sin_port = htons(RAND_PORT);
    server_addr.sin_port = htons(SERV_PORT);

    memset(client_addr.sin_zero, '\0', sizeof(client_addr.sin_zero));
    memset(server_addr.sin_zero, '\0', sizeof(server_addr.sin_zero));

    *stream = newStream(&client_addr, &server_addr, commandLine->alpha);
  }
  else{
    memcpy(&client_addr, &(*stream)->client_addr, addr_size);
    memcpy(&server_addr, &(*stream)->server_addr, addr_size);
  }

  //create new socket to server//CHECK FOR ERRORS
  if ((server_sock = socket(PF_INET, SOCK_STREAM, 0)) < 0) printf("socket failed\n");

  //change port number to unoccupied one, client_addr should be copied from stream
  client_addr.sin_port = htons(ntohs(client_addr.sin_port) + port_offset);
  INC_POFFSET();

  if (bind(server_sock, (struct sockaddr*)&client_addr, addr_size) < 0) {
    char tmp[80];
    inet_ntop(AF_INET, &client_addr.sin_addr, tmp, 80);
    log_msg(log, "Socket: %d\nBind addr: %s\nport: %d\n", server_sock, tmp, ntohs(client_addr.sin_port)); 
    log_msg(log, "Bind failed\n");
    perror("bind"); 
    exit(EXIT_FAILURE);
  }

  if (connect(server_sock, (struct sockaddr*)&(*stream)->server_addr, addr_size))
		log_msg(log, "Connect failed\n");

  //accept new socket to browser
  if ((browser_sock = accept(browserListener, (struct sockaddr *) &browser_addr,
                            &addr_size)) == -1)
    {
      close(browserListener);
      log_msg(log, "Error accepting connection.");
      return EXIT_FAILURE;
    }
  
  //create new connection adn add it to stream
  connection = newConnection(browser_sock, server_sock);
  addConnectionToStream(connection, *stream);
  log_msg(log, "Added connections on sockets b:%d,s:%d\n",connection->browser_sock, connection->server_sock); 


  //Add socket to browser to fdset
  FD_SET(browser_sock, master); // add new socket to master set
  if (browser_sock > *fdmax) {    // keep track of the max
    *fdmax = browser_sock;
  }

  //Add socket to server to master
  FD_SET(server_sock, master); // add new socket to master set
  if (server_sock > *fdmax) {    // keep track of the max
    *fdmax = server_sock;
  }

  inet_ntop(AF_INET, (void *)(&(browser_addr.sin_addr)), str, 50);
  log_msg(log, "Accepted connection from %s", str);
  return 0;
}

int receive(int fd, fd_set * master, int *fdmax, int listener, char (* buf)[BUF_SIZE], connection_list_s *connection, stream_s *stream){
  int readret;
  int i;
  if ((readret = recv(fd, *buf, BUF_SIZE, 0)) > 0)//Check if connection closed
    {
      //Http handling//
      
      log_msg(log, "Read data from fd:%d", fd);
      return readret;
    } 
 
  if (readret <= 0){//If connection closed
    log_msg(log, "Connection closed on fd:%d",fd);
    close_socket(connection->server_sock);
    close_socket(connection->browser_sock);
    FD_CLR(connection->server_sock, master);
    FD_CLR(connection->browser_sock, master);

    log_msg(log, "Closing connection associated with sockets %d,%d because of %d\n", connection->server_sock, connection->browser_sock, fd);

    removeConnectionFromStream(connection, stream);
    freeConnection(connection);

    for(i=0; i<=*fdmax; i++){
      if(FD_ISSET(i, master)){
        log_msg(log, "FD_SET: %d", i);
      }
    }
    //change fdmax
  }
  return -1;
}


int sendResponse(int fd, char * response, int responselen){
  if (send(fd,response,responselen,0)!=responselen)
    {
      close_socket(fd);
      close_socket(3);
      printf("Send Failed\n");
      log_msg(log, "Error sending to client.");
      return EXIT_FAILURE;
    }
  log_msg(log, "Sent response to fd:%d", fd);
  return 0;
}

int setupBrowserListenerSocket(int * plistener, unsigned short port){
  int listener;
  struct sockaddr_in addr;
  
  if ((listener = socket(PF_INET, SOCK_STREAM, 0)) == -1){
    log_msg(log, "Failed creating socket.");
    return -1;
  }

  addr.sin_family = AF_INET;
  addr.sin_port = htons(port);
  addr.sin_addr.s_addr = INADDR_ANY;
 
  log_msg(log, "Binding to port:%d\n", port);
 
  /* servers bind sockets to ports---notify the OS they accept connections */
  if (bind(listener, (struct sockaddr *) &addr, sizeof(addr))){
    close_socket(listener);
    log_msg(log, "Failed binding socket.");
    return -1;
  }
  
  if (listen(listener, 5)){
    close_socket(listener);
    log_msg(log, "Error listening on socket.");
    return -1;
  }

  *plistener = listener;
  return 0;
}

int replaceString(char *buf, unsigned int bufstrlen, char *str, unsigned int index, unsigned int deleteLen, unsigned int replaceLen)
{
  char *endBuf = malloc(sizeof(char) * bufstrlen);
  memcpy(endBuf, buf + index + deleteLen, bufstrlen + 1 - index - deleteLen);
  memcpy(buf + index, str, replaceLen);
  memcpy(buf + index + replaceLen /*- deleteLen*/, endBuf, bufstrlen + 1 - index - deleteLen);
  free(endBuf);
  return bufstrlen + replaceLen - deleteLen;
}

int startChunk(connection_list_s *connection, char *chunkName){
  //Check for NULL
  chunk_list_s *chunk = newChunkList();
  addChunkToConnections(chunk, connection);
	chunk->bytesLeft = 0;
  //+1 to include NULL char
  if (strlen(chunkName) + 1 > 64){
		printf("Chunk name too big\n");
		exit(4);
	}
  memcpy(chunk->chunk_name, chunkName, strlen(chunkName) + 1);

  gettimeofday(&chunk->time_started, NULL);
  return 0;
}

int finishChunk(stream_s *stream, connection_list_s *connection, command_line_s commandLine){
  chunk_list_s *chunk = connection->chunk_throughputs;
  double duration;
  unsigned int throughput;
  float alpha = stream->alpha;

  gettimeofday(&chunk->time_finished, NULL);

	duration = ((chunk->time_finished.tv_sec - chunk->time_started.tv_sec) * 1000000.0
						 + (chunk->time_finished.tv_usec - chunk->time_started.tv_usec)) / 1000000.0;

  throughput = (chunk->chunk_size / duration) * (8.0 / 1000);

  stream->current_throughput = (throughput * alpha) + ((1.0 - alpha) * stream->current_throughput); 

  //Log to proxy_log in correct format
  char *ser = inet_ntoa(commandLine.www_ip);
  log_proxy(p_log, chunk, stream, ser);

  removeChunkFromConnections(chunk, connection);
  //beware of freeing the entire list if concurrent chunks
  free(chunk);
  return 0;
}

int main(int argc, char* argv[])
{
  
  command_line_s commandLine;
  stream_s *stream = NULL;
  connection_list_s *connection;

  int browserListener;
 
  fd_set master;
  fd_set read_fds;

  int fdmax;
  int sock;
  struct timeval tv;
  
  char buf[BUF_SIZE];
  int ret;
  int sockcont=0;

  port_offset = 0;
  signal(SIGINT, sigINThandler);
  
  if (parseCommandLine(argc, argv, &commandLine) < 0)
    return -1;

  log = open_log(log, "error.log");
  log_msg(log, "Created log file\n");

  p_log = open_log(p_log, commandLine.logfile);
  
  tv.tv_sec = 60;
  tv.tv_usec = 0;
  FD_ZERO(&master);
  FD_ZERO(&read_fds);

  log_msg(log, "Server initiated");
  
  /* set up socket to listen for incoming connections from the browser*/
  if (setupBrowserListenerSocket(&browserListener, commandLine.listen_port) < 0)
    return EXIT_FAILURE;
  
  /*Add browser listerner to master set of fd's*/
  FD_SET(browserListener, &master);
  fdmax = browserListener;

  log_msg(log, "Browser listener: %d\n", browserListener);
  
  tv.tv_sec = 60;
  while (1)
    {
      //wait for a socket to have data to read
      sock = waitForAction(&master,&read_fds, fdmax, tv, sockcont);/*blocking*/
      if (sock < 0) continue;//select timeout
      if (sock == 0){//read through read_fs , start from beginning
        sockcont = 0;
        continue;
      }

      sockcont = sock + 1;
      
      log_msg(log, "Browser requesting a new connection\n");
      //Browser is requesting new connection
      if (sock == browserListener){ //new connection        
        if (stream == NULL){
				  log_msg(log, "Stream is null\n");
        }

        if (acceptBrowserServerConnectionToStream(browserListener, &master, &fdmax, &stream, &commandLine) < 0)//add connection to stream
          //add browser sock to read_fs
          return EXIT_FAILURE;
        //if no current streams   
      }

      else {
				log_msg(log, "Determining if connection is from browser or server...\n");
        //Determine if coming from browser or server
        connection =  getConnectionFromSocket(stream, sock);

        if (connection == NULL){
          log_msg(log, "Error: NULL connection\n EXIT_FAILURE\n");
          return EXIT_FAILURE;
        }

        if (sock == connection->browser_sock){
	  			log_msg(log, "Found a browser connection\n");

				  memset(buf, 0, BUF_SIZE);
    			ret = receive(sock, &master, &fdmax, browserListener, &buf, connection, stream);

	  			int x;
	  			char *get;
	  			char bit[16];
	  			char header[100];
	  			x = strcspn(buf, "\n");
				  if (x > 99) x = 99;
				  memcpy(header, buf, x);
				  header[x] = '\0';

	  			log_msg(log, "Received request from browser:\n%s\n\n", header);
//	 				printf("Received header request from browser:\n%s\n\n", header);

          if (ret > 0){
						//GET request for .f4m file
						//Send GET for ... _nolist.f4m
				    if ((get = strstr(header, "big_buck_bunny.f4m")) != NULL){
							stream->current_throughput = getBitrate(stream->current_throughput, stream->available_bitrates);
							unsigned int index = strstr(buf, ".f4m") - buf;
							ret = replaceString(buf, ret, "_nolist", index, 0, strlen("_nolist") );
	//						printf("buf:%s\n", buf);
						}

				    //Get request is for /vod/###SegX-FragY
	  			  //Modify for current tput and start new chunk
	    			if ((get = strstr(header, "Seg")) != NULL){

							// /vod/###SegX-FragY
							char *chunkName;
							char chunk[64];
							chunkName = strstr(buf, "/vod/");
							int intLen =  strstr(buf, "Seg") - (chunkName + strlen("/vod/"));

							unsigned int index = 0;
							int br = getBitrate(stream->current_throughput, stream->available_bitrates); //###
		//					printf("cur tput: %d, found br: %d\n", stream->current_throughput, br);
							snprintf(bit, 16, "%d", br); //convert br into string

							ret = replaceString(chunkName + strlen("/vod/"), ret, bit, index, intLen, strlen(bit));
							memcpy(chunk, chunkName, strstr(buf, " HTTP") - chunkName);
							chunk[strstr(buf, " HTTP") - chunkName] = '\0';

			//				printf("Chunkname sent to start chunk: %s\n", chunk); 
							startChunk(connection, chunk);
						}
						sendResponse(connection->server_sock, buf, ret);
	  			}
        }
	
        if (sock == connection->server_sock){
				  log_msg(log, "Received request from server\n");

	  			memset(buf, 0, BUF_SIZE);
          ret = receive(sock, &master, &fdmax, browserListener, &buf, connection, stream);
				  //Use close to avoid streamlining 
	  			char *p, *type, *len, *hlen;
	  			char close[] = "close     ";
	  			char contentType[] = "Content-Type: ";
				  char header[100];
				  memset(header, '\0', 100);
				  int x, contentLength;

					if (ret <= 0) {
				//		printf("Continuing!\n");
						continue;
					}

					if (strstr(buf, "Not Modified") != NULL){
						exit(4);
					}

					if (memcmp(buf, "HTTP/1.1", strlen("HTTP/1.1")) == 0){
			//			printf("Is HTTP\n");
						if ((p = strstr(buf, "Connection: ")) != NULL){
	   					p = p + strlen("Connection: ");
				     	memcpy(p, close, strlen(close));
	    			}	
	   
	  				// p is a pointer to Content-Type
				  	p = strstr(buf, contentType);
				  	if (p != NULL){
				    	x = strcspn(p, "\n");
	    				if (x > 99) x = 99;
				    	memcpy(header, p, x);
				    	header[x] = '\0';
	    				fflush(stdout);

		    			if ((type = strstr(header, "Content-Type: ")) != NULL){ //found Content-Type
								type = type + strlen("Content-Type: ");
				   		}
	  				}		
	 
		//				printf("TYPE:%s\n", type); 	  
         	 	if (ret > 0){
	  //    			printf("Received %d:\n%s\n",ret, buf);
							//If text/xml, set tput to be min
			  	 	 	if( strstr(type, "text/xml") != NULL){
								stream->current_throughput = getBitrate(stream->current_throughput, stream->available_bitrates);
							}

							//measure header length until /n/r/n/r and then 
							//subtract that from ret to see how much data you got and forward
							if ( strstr(type, "video/f4f") != NULL){
								if ((len = strstr(buf, "Content-Length: ")) != NULL){
									len = len + strlen("Content-Length: ");
									contentLength = atoi(len);
									connection->chunk_throughputs->chunk_size = contentLength;
									if ((hlen = strstr(buf, "\r\n\r\n")) != NULL){
										hlen = hlen + strlen("\n\r\n\r");
										unsigned int h = hlen - buf;
										unsigned int bytesRead = ret - h;
		//								printf("bytesRead: %d\nbytesTotal: %d\n", bytesRead, contentLength);
										connection->chunk_throughputs->bytesLeft = contentLength - bytesRead;
									}
								}
							}
							sendResponse(connection->browser_sock, buf, ret);
		  			}
					} // end IF "HTTP/1.1"
					else{ // still reading same request chunk
	//					printf("Reading video data\nBuf:\n%s\nRet: %d\n", buf, ret);
	//					printf("Chunk throughputs: %p\n", connection->chunk_throughputs);
						if(connection->chunk_throughputs != NULL){
							connection->chunk_throughputs->bytesLeft = connection->chunk_throughputs->bytesLeft - ret;
							if (connection->chunk_throughputs->bytesLeft == 0){
	//							printf("***FINISHING CHUNK***\n");
								finishChunk(stream, connection, commandLine);
	//							printf("BYE\n");
							}
						}					
						sendResponse(connection->browser_sock, buf, ret);
					}
        }//end server sock

      } // end ELSE
    } // end WHILE(1)

  close_socket(browserListener);
      //endLogger();
  return EXIT_SUCCESS;
}
