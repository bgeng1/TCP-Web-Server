/* TCP Web Server by Bright Geng */
/* possible improvements: 
	- break up the read function into smaller functions
	- build an html page to send with 404 status message
	- doesn't work on file names with spaces (but neither does url?)
*/
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h> 

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BACKLOG 10			//max queue size on the socket
#define MESSAGE_LIMIT 8000	//max size of HTTP header
#define FILENAME_LIMIT 25	//max file name size

int readRequest();
char *getExtension();
void removeSpaces();

int main(int argc, char *argv[]){
	/* standard TCP socket variables */
	int s, sn, clientLength;
	unsigned short port;
	struct sockaddr_in clientAddress, serverAddress;

	if(argc != 2) {
		printf("usage:%s <port> \n",argv[0]);
		exit(1);
	}
	port = atoi(argv[1]);

	
	/* create socket */
	s = socket(AF_INET, SOCK_STREAM, 0);
	if(s < 0){
		perror("couldn't create socket!");
		exit(1);
	}
	
	/* populate socket address structure */
	serverAddress.sin_family		= AF_INET;
	serverAddress.sin_addr.s_addr	= htonl(INADDR_ANY);
	serverAddress.sin_port			= htons(port);
	
	/* assign socket address */
	if ( bind(s, (struct sockaddr *) &serverAddress, sizeof(serverAddress)) < 0 ){
		perror("couldn't bind socket!");
		exit(1);
	}
	
	/* tell socket to listen */
	if( listen(s,BACKLOG) < 0 ){
		perror("couldn't listen to socket!");
		exit(1);
	}
	
	/* infinite loop for requests */
	while(1){
		/* wait for a connection */
		printf("Waiting for TCP connection on port %hu \n",port);
		clientLength = sizeof(serverAddress);
		sn = accept(s, NULL, NULL);
		if(sn < 0){
			perror("error calling accept!");
			exit(1);
		}
		
		/* parse HTTP request */
		readRequest(sn);
		if( close(sn) < 0 ){
			perror("couldn't close accepting socket!");
			exit(1);
		}
	}
	/* shouldn't reach this point */
	return EXIT_FAILURE;
}

int readRequest(int socket){
	char buffer[MESSAGE_LIMIT];		//contains header
	char *fileBuffer;				//contains body (dynamically allocated)
	char curr=buffer[0]; 			//used to parse request
	char filename[FILENAME_LIMIT];	//contains file name
	char contentType[20];			//contains Content-Type
	int i = 5; 						//initialised to 5 to skip the "GET /"
	int j = 0;						//counter to fill the filename array
	int fileLength = 0;				//contains size of file contents
	
	/* initialising strings */
	memset(contentType,0,20);
	memset(buffer,0,MESSAGE_LIMIT);
	memset(filename,0,FILENAME_LIMIT);
	
	/* reading the request from socket */
	if( read(socket,buffer,MESSAGE_LIMIT) < 0){ 
		perror("couldn't read from socket!");
		exit(1);
	}
	printf("****************Received request:****************** \n%s",buffer);
	
	/* loop through header looking for requested file name */
	while(curr != ' '){
		curr = buffer[i];
		filename[j] = curr;
		i++;
		j++;
	}
	removeSpaces(filename);
	printf("received request for file: %s on socket: %d \n\n",filename,socket);
	
	/* get file extension */
	char *extension=getExtension(filename);
	if(strncmp(extension,"html",20)==0){
	strcpy(contentType,"text/html");
	}else if(strncmp(extension,"png",20)==0){ 
	strcpy(contentType,"image/png");
	}
	if(extension == NULL){
		perror("file has no extension!");
		exit(1);
	}

	/* opening requested file */
	//printf("trying to open %s with extension: %s\n",filename,extension);
	FILE *reqFile = fopen(filename,"rb");
	
	/* error handling when the file is not found */
	if(reqFile == NULL){
		perror("file not found!");
		write(socket,"HTTP/1.1 404 Not Found\nContent-Length: 0\nContent-Type: text/html\r\n\r\n",strlen("HTTP/1.1 404 Not Found\nContent-Length: 0\nContent-Type: text/html\r\n\r\n"));
		return EXIT_FAILURE;
	}
	
	/* getting size of the file to dynamically allocate memory for body */
	fseek(reqFile, 0, SEEK_END);
	fileLength = ftell(reqFile);
	rewind(reqFile);
	fileBuffer = (char*) malloc (sizeof(char)*fileLength);
	if( fread(fileBuffer, 1, fileLength, reqFile) <= 0 ){
		perror("requested file is empty!");
	}
	
	/* write a response back to client */
	sprintf(buffer,"HTTP/1.1 200 OK\nContent-Length: %d\nContent-Type: %s\r\n\r\n",fileLength,contentType);
	
	printf("sending this: \n%s%s",buffer,fileBuffer);
	
	/* write header */
	if( write(socket,buffer,strlen(buffer)) < 0){
		perror("couldn't write to socket");
		exit(1);
	}
	/* write body */
	if( write(socket,fileBuffer,fileLength) < 0){
		perror("couldn't write to socket");
		exit(1);
	}
	
	/* close file */
	fclose(reqFile);	
	
	

	return EXIT_SUCCESS;
}



char *getExtension(char *filename) {
	char *dot = strrchr(filename, '.');
    if(dot == NULL) return "";
    return dot + 1;
}

/* got the idea for this function off stack overflow */
void removeSpaces(char *string)
{
  char *i = string;
  char *j = string;
  while(*j != '\0')
  {
    *i = *j++;
    if(*i != ' ') i++;
  }
  *i = 0;
}
