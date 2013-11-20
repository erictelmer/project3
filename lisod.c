
/******************************************************************************
* echo_server.c                                                               *
*                                                                             *
* Description: This file contains the C source code for an echo server.  The  *
*              server runs on a hard-coded port and simply write back anything*
*              sent to it by connected clients.  It does not support          *
*              concurrent clients.                                            *
*                                                                             *
* Authors:Eric Telmer <eit@andrew.cmu.edu>                                    *
*                                                                             *
*******************************************************************************/

#include <netinet/in.h>
#include <netinet/ip.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <signal.h>
#include <arpa/inet.h>
#include <netdb.h>

#include "openssl/ssl.h"

/*#include <openssl/rsa.h>
#include <openssl/crypto.h>
#include <openssl/x509.h>
#include <openssl/pem.h>
#include <openssl/ssl.h>
#include <openssl/err.h>
*/


//#include "log.h"
#include "generateResponse.h"

#define ECHO_PORT 8088
#define BUF_SIZE 4096

#define CHK_NULL(x) if ((x)==NULL) {logString("NULL ERROR"); exit (1);}
#define CHK_ERR(err,s) if ((err)==-1) { logString("%s error", s);perror(s); exit(1); }
#define CHK_SSL(err) if ((err)==-1) {  logString("SSL ERROR");exit(2); }
#define FREE(x,s) /*fprintf(stderr,"freeing %s @ %p\n",s,x);*/ free(x);

//Command Line Args
int HTTPport;
int HTTPSport;
char logfile[30];
char lockfile[30];
char wwwfolder[30];
char CGIfolder[30];
char privateKeyFile[30];
char certificateFile[30];

//


void sigINThandler(int);

void leave(void){
  //	endLogger();
}

void * Malloc(size_t size, char * name){
  void * loc;
  loc = malloc(size);
  if (loc == NULL){
    logString("MALLOC ERROR");
    exit(3);
  }
  // printf("Mallocing %s of size: %x @ %p\n", name, (unsigned int)size, loc);
  return loc;
}

void * Realloc(void * pointer, size_t size, char * name){
  void * loc;
  loc =  realloc(pointer,size);
  if (loc == NULL){
    logString("MALLOC ERROR");
    exit(3);
  }
  //  printf("Reallocing %s @ %p of size: %x @ %p\n", name,pointer, (unsigned int)size, loc);
  return loc;
}

int close_socket(int sock)
{
    if (close(sock))
    {
        logString("Failed closing socket../");
        return 1;
    }
    return 0;
}



int printHelp(){
  printf("usage: ./lisod <HTTP port> <HTTPS port> <log file> <lock file> <www folder> <CGI folder or script name> <private key file> <certificate file>\nHTTP port\n  – the port for the HTTP (or echo) server to listen on\nHTTPS port\n  – the port for the HTTPS server to listen on\nlog file\n  – file to send log messages to (debug, info, error)\nlock file\n  – file to lock on when becoming a daemon process\nwww folder\n  – folder containing a tree to serve as the root of a website\nCGI folder or script name\n  – folder containing CGI programs—each file should be executable;\n  if a file it should be a script where you redirect all /cgi/* URIs to\nprivate key file\n  – private key file path\ncertificate file\n  – certificate file path\n");
  return -1;
}

int parseCommandLine(int argc, char*argv[]){
  if (argc != 9)
    return printHelp();
  if (sscanf(argv[1],"%d",&HTTPport) < 1)
    return printHelp();
  if (sscanf(argv[2],"%d",&HTTPSport) < 1)
    return printHelp();

  if (strlen(argv[3]) >= sizeof(logfile))
      printf("Log file too large\n");
  else if (sscanf(argv[3],"%s",logfile) < 1)
    return printHelp();

  if (strlen(argv[4]) >= sizeof(lockfile))
      printf("Lock file too large\n");
  else if (sscanf(argv[4],"%s",lockfile) < 1)
    return printHelp();

  if (strlen(argv[5]) >= sizeof(wwwfolder))
      printf("wwwfolder too large\n");
  else if (sscanf(argv[5],"%s",wwwfolder) < 1)
    return printHelp();

  if (strlen(argv[6]) >= sizeof(CGIfolder))
      printf("CGIFolder too large\n");
  else if (sscanf(argv[6],"%s",CGIfolder) < 1)
    return printHelp();

  if (strlen(argv[7]) >= sizeof(privateKeyFile))
      printf("privateKeyFile too large\n");
  else if (sscanf(argv[7],"%s",privateKeyFile) < 1)
    return printHelp();

  if (strlen(argv[8]) >= sizeof(certificateFile))
      printf("CertificateFile too large\n");
  else if (sscanf(argv[8],"%s",certificateFile) < 1)
    return printHelp();
  
  return 0;
}
int waitForAction(fd_set master, fd_set * read_fds, fd_set *write_fds, int fdmax, struct timeval tv, int fdcont){

  int i;

  *read_fds = master; // copy it
  *write_fds = master;
  tv.tv_sec = 60;
  i = select(fdmax+1, read_fds, write_fds, NULL, &tv);
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

int acceptNewConnection(int listener, fd_set * master, int * fdmax){
  socklen_t cli_size;
  struct sockaddr_in cli_addr;
  int client_sock;
  char str[50];
  cli_size = sizeof(cli_addr);
  if ((client_sock = accept(listener, (struct sockaddr *) &cli_addr,
			    &cli_size)) == -1)
    {
      close(listener);
      logString("Error accepting connection.");
      return EXIT_FAILURE;
    }
  FD_SET(client_sock, master); // add new socket to master set
  if (client_sock > *fdmax) {    // keep track of the max
    *fdmax = client_sock;
  }
  inet_ntop(AF_INET, (void *)(&(cli_addr.sin_addr)), str, 50);
  logString("Accepted connection from %s", str);
  return 0;
}


int acceptNewSecureConnection(int listener, fd_set * master, int * fdmax, SSL_CTX * ctx, fd_set*phttps,SSL *** ssl_set, int * sslsize){
  socklen_t cli_size;
  struct sockaddr_in cli_addr;
  int client_sock;
  int err;
  //  X509* client_cert;
  // char * str;
  fd_set https = *phttps;
  SSL * ssl;
  char addrstr[50];

  cli_size = sizeof(cli_addr);
  if ((client_sock = accept(listener, (struct sockaddr *) &cli_addr,
			    &cli_size)) < 0)
    {
      close(listener);
      logString("Error accepting connection.");
      return EXIT_FAILURE;
    }
  FD_SET(client_sock, master); // add new socket to master set
  FD_SET(client_sock, &https);
  if (client_sock > *fdmax) {    // keep track of the max
    *fdmax = client_sock;
  }
  ssl = SSL_new (ctx);                           CHK_NULL(ssl);
  SSL_set_fd (ssl, client_sock);
  err = SSL_accept (ssl);                        CHK_SSL(err);
  //Add ssl to the ssl set
  //printf("NEW SSL: %p\n", ssl);
  while (client_sock >= *sslsize){
    *sslsize *= 2;
    *ssl_set = Realloc(*ssl_set, *sslsize * sizeof(ssl),"ssl_set");
  }
  (*ssl_set)[client_sock] = ssl;
  /* Get the cipher - opt */
  
  //printf ("SSL connection using %s\n", SSL_get_cipher (ssl));
  
  inet_ntop(AF_INET, (void *)(&(cli_addr.sin_addr)), addrstr, 50);
  logString("Accepted https connection from %s", addrstr);
  *phttps = https;
  return 0;
}

int secureReceive(SSL* ssl, fd_set * master, int *fdmax, fd_set write_fds, int listener, char (* buf)[BUF_SIZE]){
  int readret;
  int j;
  int fd = SSL_get_fd(ssl);
  /*  if (SSL_get_shutdown(ssl)  == SSL_RECEIVED_SHUTDOWN){
    logString("Secure connection closed on fd:%d",fd);
    close_socket(fd);
    SSL_free(ssl);
    FD_CLR(fd, master);
    }*/
  
  if (!FD_ISSET(fd, &write_fds)) {
    memset(*buf,0, BUF_SIZE);
    logString("Could not write");
    return -2;
  }
  //Http handling//
  if ((readret = SSL_read(ssl, *buf, BUF_SIZE)) < 0){
    return -1;
  }

  if ((readret = SSL_read(ssl, (*buf)+1, BUF_SIZE-1)) > 0){
    logString("Read from secure fd:%d",fd);
    return 0;
  }
  memset(*buf,0,BUF_SIZE);
  if (readret == 0){//If connection closed
    logString("Connextion on fd:%d closed",fd);
    close_socket(fd);
    SSL_free(ssl);
    FD_CLR(fd, master);
  }
  if (readret < 0)
    {
      for(j = listener + 1; j<=*fdmax; j++){
	close_socket(j);
      }
      logString("Error reading data from client");
      FD_ZERO(master);
      FD_SET(listener, master);
      *fdmax = listener;
    }
  return -1;
}


int receive(int fd, fd_set * master, int *fdmax, fd_set write_fds, int listener, char (* buf)[BUF_SIZE]){
  int readret;
  int j;
  if ((readret = recv(fd, *buf, 1, MSG_PEEK)) > 0)//Check if connection closed
    {
      if (!FD_ISSET(fd, &write_fds)) {
	memset(*buf,0, BUF_SIZE);
	logString("Could not write");
	return -2;
      }
      //Http handling//
      if ((readret = recv(fd, *buf, BUF_SIZE, 0)) > 0){
	logString("Read data from fd:%d", fd);
	return 0;
      }
      //End gttp handling//
      memset(*buf,0,BUF_SIZE);
    } 
  if (readret == 0){//If connection closed
    logString("Connection closed on fd:%d",fd);
    close_socket(fd);
    FD_CLR(fd, master);
  }
  if (readret < 0)
    {
      for(j = listener + 1; j<=*fdmax; j++){
	close_socket(j);
      }
      logString("Error reading data from client");
      FD_ZERO(master);
      FD_SET(listener, master);
      *fdmax = listener;
    }
  return -1;
}

int sendSecureResponse(SSL* ssl, char * response, int responselen){
  if (SSL_write(ssl,response,responselen)!=responselen)
    {
      close_socket(SSL_get_fd(ssl));
      close_socket(3);
      logString("Error sending to client.");
      return EXIT_FAILURE;
    }
  logString("Sent secure response to fd:%d", SSL_get_fd(ssl));
  return 0;
}

int sendResponse(int fd, char * response, int responselen){
  if (send(fd,response,responselen,0)!=responselen)
    {
      close_socket(fd);
      close_socket(3);
      logString("Error sending to client.");
      return EXIT_FAILURE;
    }
  logString("Sent response to fd:%d", fd);
  return 0;
}

int setupListenerSocket(int * plistener, int port){
  int listener = *plistener;
  struct sockaddr_in addr;
  
  if ((listener = socket(PF_INET, SOCK_STREAM, 0)) == -1)
    {
      logString("Failed creating socket.");
      return -1;
    }
  addr.sin_family = AF_INET;
  addr.sin_port = htons(port);
  addr.sin_addr.s_addr = INADDR_ANY;
  
  /* servers bind sockets to ports---notify the OS they accept connections */
  if (bind(listener, (struct sockaddr *) &addr, sizeof(addr)))
    {
      close_socket(listener);
      logString("Failed binding socket.");
      return -1;
    }
  
  if (listen(listener, 5))
    {
      close_socket(listener);
      logString("Error listening on socket.");
      return -1;
    }
  *plistener = listener;
  return 0;
}

int initSSL(SSL_CTX** pctx){
  //  SSL_METHOD *meth;
  SSL_CTX * ctx = *pctx;
  // SSL_load_error_strings();
  SSL_library_init();
  //  meth = SSLv23_server_method();
  ctx = SSL_CTX_new (SSLv23_server_method());
  if (!ctx) {
    exit(2);
  }
  
  if (SSL_CTX_use_certificate_file(ctx, certificateFile, SSL_FILETYPE_PEM) <= 0) {
    exit(3);
  }
  if (SSL_CTX_use_PrivateKey_file(ctx, privateKeyFile, SSL_FILETYPE_PEM) <= 0) {
    exit(4);
  }

  if (!SSL_CTX_check_private_key(ctx)) {
    fprintf(stderr,"Private key does not match the certificate public key\n");
    exit(5);
  }
  *pctx = ctx;
  return 0;
}

SSL ** newSSLSet(int sslsize){
  SSL ** ssl_set = Malloc(sizeof(SSL *) * (sslsize), "ssl_set");
  return ssl_set;
}

int main(int argc, char* argv[])
{
  
  int listener;
  int Slistener;

  fd_set https;
  fd_set master;
  fd_set read_fds;
  fd_set write_fds;
  int fdmax;
  int fd;
  struct timeval tv;
  
  SSL_CTX* ctx;
  SSL* ssl;
  int sslsize=20;
  SSL ** ssl_set = newSSLSet(sslsize);
  char buf[BUF_SIZE];
  int ret;
  int responselen = 200;
  char * response;
  
  signal(SIGINT, sigINThandler);
  
  if (parseCommandLine(argc, argv) < 0)
    return -1;
  initLogger(logfile);
  initSSL(&ctx);

  tv.tv_sec = 60;
  tv.tv_usec = 0;
  FD_ZERO(&https);
  FD_ZERO(&master);
  FD_ZERO(&read_fds);
  FD_ZERO(&write_fds);
  fprintf(stdout, "----- Echo Server -----\n");
  logString("Server Initiated");
  
  /* all networked programs must create a socket */
  if (setupListenerSocket(&listener, HTTPport) < 0)
    return EXIT_FAILURE;
  if (setupListenerSocket(&Slistener, HTTPSport) < 0)
    return EXIT_FAILURE;
  
  /*Add listerner to master set of fd's*/
  FD_SET(listener, &master);
  FD_SET(Slistener, &master);
  fdmax = (Slistener > listener) ? Slistener : listener;
  tv.tv_sec = 60;
  int fdcont = 0;
  /* finally, loop waiting for input and then write it back */
  while (1)
    {
      // int fdcont=0;
      fd = waitForAction(master,&read_fds,&write_fds, fdmax, tv, fdcont);/*blocking*/ 
      if (fd <0) continue;
      if (fd == 0){
	fdcont = 0;
	continue;
      }
      fdcont = fd;
      if (fd == listener){ //new connection
	if (acceptNewConnection(listener, &master, &fdmax) < 0)
	  return EXIT_FAILURE;
      }
      else if (fd == Slistener){
	if (acceptNewSecureConnection(Slistener,&master,&fdmax,ctx,&https,&ssl_set,&sslsize) < 0)
	  return EXIT_FAILURE;
      }
      else {
      	responselen=200;
	response = Malloc(responselen, "response");
	memset(buf,0,BUF_SIZE);
	if (FD_ISSET(fd, &https)){
	  ssl = ssl_set[fd];
	  if ((ret = secureReceive(ssl,&master,&fdmax,write_fds,Slistener,&buf)) < 0){
	    FREE(response,"response");
	    continue;
	  }
	}
	else if ((ret = receive(fd,&master,&fdmax,write_fds,listener, &buf)) < 0){
	  FREE(response,"response");
	  continue;
	}
	if ((ret = generateResponse(buf, &response, &responselen)) < 0){
	  FREE(response,"response");
	  continue;
	}
	//before call responselen is size of Malloc, after is size of message
	
	
	if (FD_ISSET(fd,&https)){
	  ssl = ssl_set[fd];
	  sendSecureResponse(ssl,response,responselen);
	}
	else sendResponse(fd,response, responselen);
	FREE(response,"response");
     
      }//End else
    }//End while(1)
  FREE(ssl_set,"sslset");
  SSL_CTX_free(ctx);
  close_socket(listener);
  close_socket(Slistener);
  //endLogger();
  return EXIT_SUCCESS;
}

void sigINThandler(int sig)
{
  close_socket(3);
  //endLogger();
}
