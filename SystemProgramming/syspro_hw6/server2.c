/** @file server.c */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <pthread.h>
#include <netdb.h>
#include <unistd.h>
#include <signal.h>
#include <assert.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <semaphore.h>

#include "queue.h"
#include "libdictionary.h"

#define BACKLOG 10
#define BUFFER_LEN 100


const char *HTTP_404_CONTENT = "<html><head><title>404 Not Found</title></head><body><h1>404 Not Found</h1>The requested resource could not be found but may be available again in the future.<div style=\"color: #eeeeee; font-size: 8pt;\">Actually, it probably won't ever be available unless this is showing up because of a bug in your program. :(</div></html>";
const char *HTTP_501_CONTENT = "<html><head><title>501 Not Implemented</title></head><body><h1>501 Not Implemented</h1>The server either does not recognise the request method, or it lacks the ability to fulfill the request.</body></html>";

const char *HTTP_200_STRING = "OK";
const char *HTTP_404_STRING = "Not Found";
const char *HTTP_501_STRING = "Not Implemented";
const char *html = "text/html";
const char *css = "text/css";
const char *jpg = "image/jpeg";
const char *png = "image/png";
const char *other = "text/plain";
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

void handle()
{

	exit(0);

}
 const char * myparser(const char * input)
{
	char str[100];
	strcpy(str, input);
	char *prev;
	char *curr;
	curr = strtok(str, ".");
	do{
		prev = curr;
		curr = strtok(NULL, ".");
	}while(curr != NULL);

	if(!strcmp(prev, "html"))
		return html;
	else if(!strcmp(prev, "css"))
		return css;
	else if(!strcmp(prev, "jpg"))
		return jpg;
	else if(!strcmp(prev, "png"))
		return png;
	else
		return other;

}

char* process_http_header_request(const char *request)
{
	// Ensure our request type is correct...
	if (strncmp(request, "GET ", 4) != 0)
		return NULL;

	// Ensure the function was called properly...
	assert( strstr("\n", request) == NULL );
	assert( strstr("\r", request) == NULL );

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

/*
	connection_receiver()
*/
void *slave(void *input) //pthread worker
{
	int fd = *((int *)input);
	char main_buffer[2048];
	memset(main_buffer, 0, 2048);
	char buffer[513];
	memset(buffer,0,513);
	// ==========================
	// = receive client request =
	// ==========================
	int byte_count;
	while(1)
	{
		while(1)
		{
			byte_count = recv(fd, buffer, 512, 0);
			if(byte_count == -1)
			{
				perror("recv:");
				break;
			}
			if(byte_count == 0)
			{
				fprintf(stderr, "Client closed connection.\n");
				close(fd);
				pthread_exit(0);
			}
			//printf("bytes: %d\n", byte_count);
			// printf("%s", buffer);
			sprintf(main_buffer+strlen(main_buffer), buffer);
			if(strstr(main_buffer,"\r\n\r\n"))
			{
				// printf("Found it!!!!!!!\n\n\n\n");
				break;
			}
			memset(buffer,0,512);
		}
		printf("%s\n", main_buffer);
		// =====================
		// = Parse the request =
		// =====================
		char *pch;
		pch = strtok(main_buffer, "\r\n");
		printf("%s\n",pch);
		char *request = process_http_header_request(pch);
		// printf("PARSED REQUEST: %s\n", request);
		dictionary_t dic;
		dictionary_init(&dic);
		char *pch2;
		do {
			pch2 = strtok(NULL, "\r\n");
			if(pch2){
				// printf("PARSED: %s\n", pch2);
				dictionary_parse(&dic, pch2);
			}
		} while(pch2);
		printf("finished parsing.\n");
		// =======================
		// = generating response =
		// =======================
		char my_response[800000];
		FILE *reply_file;
		int status_code;
		const char *response_string;
		const char *content_type = other;
		if(request==NULL)
		{
			status_code = 501;
			response_string = HTTP_501_STRING;
		}
		else
		{
			if(!strcmp(request,"/"))
			{
				reply_file = fopen("web/index.html","r");
				status_code = 200;
				response_string = HTTP_200_STRING;
				content_type = html;
			}
			else{
				content_type = myparser(request);
				char prefix[100];
				sprintf(prefix, "web");
				sprintf(prefix+strlen(prefix), request);
				printf("REQUESTED FILE PATH: %s\n", prefix);
				reply_file = fopen(prefix, "r");
				status_code = 200;
				response_string = HTTP_200_STRING;
			}
			if(reply_file == NULL)
			{
				status_code = 404;
				response_string = HTTP_404_STRING;
			}
		}
		sprintf(my_response, "HTTP/1.1 %d %s\r\n", status_code, response_string);
		sprintf(my_response+strlen(my_response), "Content-Type: %s\r\n", content_type);
		long content_length;
		if(status_code == 404)
			content_length = strlen(HTTP_404_CONTENT);
		else if(status_code == 501)
			content_length = strlen(HTTP_501_CONTENT);
		else{
			fseek(reply_file, 0, SEEK_END);
			long pos = ftell(reply_file);
			content_length = pos;
		}
		sprintf(my_response+strlen(my_response), "Content-Length: %ld\r\n", content_length);
		const char *conn = dictionary_get(&dic, "Connection");
		int result = 1;
		if(conn != NULL)
		{
			result = strcasecmp(conn,"Keep-Alive");
			if(result == 0){
				sprintf(my_response+strlen(my_response), "Connection: Keep-Alive\r\n\r\n");
			}
			else
			{
				sprintf(my_response+strlen(my_response), "Connection: close\r\n\r\n");
			}
		}
		else
		{
			sprintf(my_response+strlen(my_response), "Connection: close\r\n\r\n");
		}
		unsigned long total = 0;
		while(1)
		{
			size_t num = send(fd, my_response + total, strlen(my_response) - total, 0);
			// printf("Bytes sent: %d\n", (int)num);
			total += num;
			// printf("Total bytes sent: %d\n", (int)total);
			if(total == strlen(my_response))
				break;
		}
		// ===========
		// = PAYLOAD =
		// ===========
		unsigned long size;
		char *payload = NULL;
		if(status_code == 404)
		{
			payload = strdup(HTTP_404_CONTENT);
		}
		else if(status_code == 501)
		{
			payload = strdup(HTTP_501_CONTENT);
		}
		else{
			fseek(reply_file, 0, SEEK_END);
			unsigned long pos = ftell(reply_file);
			fseek(reply_file, 0, SEEK_SET);

			char *bytes = malloc(pos+5);
			fread(bytes, pos, 1, reply_file);
			size = pos;
			fclose(reply_file);
			payload = bytes;
		}
		total = 0;
		while(1)
		{
			unsigned long num = send(fd, payload + total, size - total, 0);
			// printf("%ld\n", num);
			printf("%ld\n", num);
			total += num;
			if(total == size)
				break;
		}
		free(payload);
		if(request!=NULL)
		{
			free(request);
			request = NULL;
		}
		printf("finished generating response.\n");
		// ==================================
		// = send the HTTP response to user =
		// ==================================

		memset(main_buffer, 0, 2048);
		memset(buffer, 0, 513);
		dictionary_destroy(&dic);
		if(result!=0)
			break;
	}
	close(fd);
	printf("file descriptor closed.\n");
	printf("%s", main_buffer);
	pthread_exit(0);
}


// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa)
{
	if (sa->sa_family == AF_INET) {
		return &(((struct sockaddr_in*)sa)->sin_addr);
	}

	return &(((struct sockaddr_in6*)sa)->sin6_addr);
}


/** Entry point to the program. */
int main(int argc, char **argv)
{
// ==========
// = PART 1 =
// ==========
	char* PORT = "8080";
	if(argc > 1)
	{
		PORT = argv[1];
	}

	int sockfd, new_fd; // listen on sock_fd, new connection on new_fd
	struct addrinfo hints, *servinfo, *p;
	struct sockaddr_storage their_addr; //connector's adress information
	socklen_t sin_size;
	int yes = 1;
	char s[INET6_ADDRSTRLEN];
	int rv;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE; //use my IP
	// ===================
	// = get my own info =
	// ===================
	if((rv = getaddrinfo(NULL, PORT, &hints, &servinfo))!=0)
	{
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
	}

	// loop through all the results and bind to the first we can
	for(p = servinfo; p != NULL; p = p -> ai_next)
	{
		// =================
		// = create socket =
		// =================
		if((sockfd = socket(AF_INET, SOCK_STREAM, 0))==-1)
		{
			perror("server: socket");
			continue;
		}

		if(setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int))==-1)
		{
			perror("setsockopt");
		}

		// =============================
		// = bind socket to my address =
		// =============================
		if(bind(sockfd, p->ai_addr, p->ai_addrlen)==-1){
			close(sockfd);
			perror("server: bind");
			continue;
		}

		break;
	}

	if(p == NULL)
	{
		fprintf(stderr, "server: fail to bind\n");
		return 2;
	}
	freeaddrinfo(servinfo); //all done with this structure

	// ========================================
	// = set socket to passive listening state =
	// ========================================
	if(listen(sockfd, BACKLOG))
	{
		perror("listen");
		exit(1);
	}

	printf("server: waiting for connections...\n");

	struct sigaction sa;
	sa.sa_handler = handle;
	sa.sa_flags = 0;
	sigemptyset(&sa.sa_mask);
	sigaction(SIGQUIT, &sa, NULL);
	// ==========
	// = PART 2 =
	// ==========


	while(1) {  // main accept() loop
		printf("Got here.\n");
		sin_size = sizeof(their_addr);
		new_fd = accept(sockfd, (struct sockaddr *)&their_addr, &sin_size);
		if (new_fd == -1) {
			perror("accept");
			continue;
		}

		inet_ntop(their_addr.ss_family, get_in_addr((struct sockaddr *)&their_addr), s, sizeof(s));
		printf("server: got connection from %s\n", s);
		pthread_t pid;
		pthread_create(&pid, NULL, slave, &new_fd);
	}

	// ================
	// = Catch Signal =
	// ================

	return 0;
}
