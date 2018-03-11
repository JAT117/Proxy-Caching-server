/* 
   Client program: This is the code for a client who sends an URL as request to proxy server and receives the HTTP response of that URL through the proxy server. If it wants to quit,  
   then it sends "quit" message to the proxy server in place of URL. 
*/ 

#include <stdio.h>              
#include <sys/types.h>          
#include <sys/socket.h>         
#include <netinet/in.h>
#include <netdb.h>
#include <error.h>
#include <unistd.h> 
#include <string.h>
#include <stdlib.h>

int main(int argc, char *argv[])
{
	int sockfd, portno, n, j;
	FILE *fc;
	struct sockaddr_in serv_addr;
	struct hostent *server;
	char buffer[25000], url[50];

	system("clear");

	if (argc < 2)
	{
		printf("\nPort number is missing...\n");
		exit(0);
	}

	portno = atoi(argv[1]);
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd < 0)
		error(EXIT_FAILURE, 0, "ERROR opening socket");
	
	server = gethostbyname("cse01.cse.unt.edu"); //Proxy server
	if (server == NULL)
	{
		printf("\nERROR, no such host...\n");
		exit(0);
	}

	bzero((char *) &serv_addr, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(portno);
	memcpy(&serv_addr.sin_addr, server->h_addr, server->h_length);
	
	if(connect(sockfd,(struct sockaddr*)&serv_addr,sizeof(serv_addr))<0)
		error(EXIT_FAILURE, 0, "ERROR connecting the server...");

	printf("\n...Welcome to client interface...\n"); 

	while(1)
	{
		printf("\nEnter URL of the website (type 'quit' without quotes in case of exit): ");
		bzero(url,50);
		scanf("%s", url);
		n = write(sockfd,url,strlen(url));
		if (n < 0)
		{
			error(EXIT_FAILURE, 0, "ERROR writing to socket");
			break;
		}

		if(strcmp(url, "quit") == 0)
		{
			printf("\nClient quits...\n\n");
			close(sockfd);
			exit(0);
		}
		
		bzero(buffer,25000);
		n = read(sockfd,buffer,25000);
		if (n < 0)
			error(EXIT_FAILURE, 0, "ERROR reading from socket");
		else
		{
			if(strcmp(buffer, "ERR") == 0)
				printf("\nWrong URL. Web page not found...\n");
			else
			{
				printf("The content received from the proxy server:\n");
				printf("%s\n\n", buffer);
			}
		}		
	}

	return 0;	
}
}