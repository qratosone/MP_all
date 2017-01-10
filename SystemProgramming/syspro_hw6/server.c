/** @file server.c */

#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <pthread.h>
#include <netdb.h>
#include <unistd.h>
#include <signal.h>
#include <queue.h>
#include <assert.h>
#include <sys/types.h>
#include <sys/socket.h>

#include "queue.h"
#include "libhttp.h"
#include "libdictionary.h"
#define MAX_CLIENTS 10
#define MAX_SIZE 4096
const char *HTTP_404_CONTENT = "<html><head><title>404 Not Found</title></head><body><h1>404 Not Found</h1>The requested resource could not be found but may be available again in the future.<div style=\"color: #eeeeee; font-size: 8pt;\">Actually, it probably won't ever be available unless this is showing up because of a bug in your program. :(</div></html>";
const char *HTTP_501_CONTENT = "<html><head><title>501 Not Implemented</title></head><body><h1>501 Not Implemented</h1>The server either does not recognise the request method, or it lacks the ability to fulfill the request.</body></html>";

const char *HTTP_200_STRING = "OK";
const char *HTTP_404_STRING = "Not Found";
const char *HTTP_501_STRING = "Not Implemented";

/**
 * Processes the request line of the HTTP header.
 * 
 * @param request The request line of the HTTP header.  This should be
 *                the first line of an HTTP request header and must
 *                NOT include the HTTP line terminator ("\r\n").
 *
 * @return The filename of the requested document or NULL if the
 *         request is not supported by the server.  If a filename
 *         is returned, the string must be free'd by a call to free().
 */
char* process_http_header_request(const char *request)
{
	// Ensure our request type is correct...
	if (strncmp(request, "GET ", 4) != 0)
		return NULL;

	// Ensure the function was called properly...
	assert( strstr(request, "\r") == NULL );
	assert( strstr(request, "\n") == NULL );

	// Find the length, minus "GET "(4) and " HTTP/1.1"(9)...
	int len = strlen(request) - 4 - 9;

	// Copy the filename portion to our new string...
	char *filename = malloc(len + 1);
	strncpy(filename, request + 4, len);
	filename[len] = '\0';

	// Prevent a directory attack...
	//  (You don't want someone to go to http://server:1234/../server.c to view your source code.)
	if (strstr(filename, ".."))
	{
		free(filename);
		return NULL;
	}

	return filename;
}
void interrupt_handler(int signal);
void close_server();
void interrupt_handler(int signal){
	if(signal == SIGINT){
		close_server();
	}
}
void close_server() {
	
	exit(1);
  // Use a signal handler to call this function and close the server
}
int endingServer;
int serverSocket;
int clients[MAX_CLIENTS];
int clientsConnected;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
void *processClient(void *arg)
{
	int client_fd=(intptr_t)arg;
    char buffer[MAX_SIZE];
	int len=0;
	int num;
	num=recv(client_fd,buffer+len,MAX_SIZE,0);
	len+=num;
	while()
	
}
int main(int argc, char **argv)
{
	char* port="8080";
	if(argc > 1)
	{
		port =argv[1];
	}
	signal(SIGINT,interrupt_handler);
	serverSocket=socket(AF_INET,SOCK_STREAM,0);
	struct addrinfo hints,*result;
	memset(&hints,0,sizeof(struct addrinfo));
	hints.ai_family=AF_INET;
	hints.ai_socktype=SOCK_STREAM;
	hints.ai_flags=AI_PASSIVE;
	int s=getaddrinfo(NULL,port,&hints,&result);
	if(s!=0)
	{
		printf("err getaddrinfo\n");
		perror(NULL);
		exit(1);
	}
	if(bind(serverSocket,result->ai_addr,result->ai_addrlen)!=0)
	{
		printf("err bind\n");
		perror(NULL);
		exit(1);
	}
	if(listen(serverSocket,MAX_CLIENTS)!=0)
	{
		printf("err listen\n");
		perror(NULL);
		exit(1);
	}
	int i = 0;
	clientsConnected=0;
	pthread_t p[MAX_CLIENTS];
	while(clientsConnected < MAX_CLIENTS){
		clients[i] = accept(serverSocket, NULL, NULL);
		if(clients[i]==-1){continue;}
		pthread_mutex_lock(&mutex);
		clientsConnected++;
		pthread_mutex_unlock(&mutex);
		pthread_create(&p[i], 0, processClient, (void *)((size_t)clients[i]));
		i++;
	}
	return 0;
}
