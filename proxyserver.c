/* 
   Server program: This is the code for "Proxy server" which maintains a cache of at most 5 latest requested web pages as well as a list of those webpages' URLs. For every new request,
   the cache and list are updated. After receiving the request from the client, the proxy server first checks its list. If the webpage is available in the list, it forwards the stored
   webpage from the cache. If it is not available, then the proxy server downloads the page from the origin server. If the HTTP response is "200 OK", it saves the web page, updates its
   cache and list, and forwards the web page to the client. If the HTTP response status code is not 200, then the proxy server directly forwards the HTTP response without caching it.
*/ 

#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>         
#include <netinet/in.h> 
#include <error.h>
#include <unistd.h> 
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <stdarg.h>

void file_updater(); 
void search_webpage();

char buffer[25000], url[50], status[4];
int count = 0; 

int main(int argc, char *argv[])
{
	/* Connection setup between client and proxy server */
	int sockfd, newsockfd, portno, clilen, n;
	char filename[60];
	FILE *fp;
	struct sockaddr_in serv_addr, cli_addr;
	system("clear");
	if(argc < 2)
	{
		printf("\nPort number is missing...\n");
		exit(0);
	}

	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd < 0)
		error(EXIT_FAILURE, 0, "ERROR opening socket");

	bzero((char *) &serv_addr, sizeof(serv_addr));
	portno = atoi(argv[1]);
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = INADDR_ANY;
	serv_addr.sin_port = htons(portno);
	if(bind(sockfd,(struct sockaddr *)&serv_addr,sizeof(serv_addr)) < 0)
		error(EXIT_FAILURE, 0, "ERROR binding");

	printf("\n...Welcome to the PROXY SERVER...\n");
	printf("\n..Waiting for clients' requests..\n");

	listen(sockfd, 5);
	while(1)
	{
		clilen=sizeof(cli_addr);
		newsockfd = accept(sockfd,(struct sockaddr *) &cli_addr, &clilen);

		if (newsockfd < 0)
			error(EXIT_FAILURE, 0, "ERROR on accept");

		printf("\nClient is connected...\n");
		
		while(1)
	   	{
			bzero(buffer, 25000);
		   	bzero(url, 50);
		   	n = read(newsockfd,url,49);
		   	if(n<0)
				error(EXIT_FAILURE, 0, "ERROR reading from socket");
			
			if(strcmp(url, "quit") == 0)
			{
				printf("\nClient quits...Waiting for another client...\n");
				close(newsockfd);
				break; 
			}
		   	else
		   	{
				printf("\nClient has requested for the webpage %s\n", url);
				
				/* Searching the webpage and updating "list.txt" file */
				search_webpage();

				if(strcmp(status, "ERR") == 0) /* Wrong URL is given by client */
				{
					printf("\nClient has requested a wrong URL...\n\n");
					strcpy(buffer, "ERR");
				}
				else if(strcmp(status, "AVL") == 0) /* Web page is available in cache */
				{
					printf("\nThe web page is already stored in cache...\n"); 
					goto use_cache;
				}
				else if(strcmp(status, "200") != 0) /* HTTP status code is other than 200 */
				{
					printf("\nHTTP status code is not 200. Page is not cached. Information is directly sent to the client...\n\n");
				}
				else /* HTTP status code is 200. Webpage is sent to client from cache */
				{
		use_cache:		strcpy(filename, url);
					strcat(filename, ".html");
					n = 0;
					fp = fopen(filename, "r");
					while(1)
				   	{
						buffer[n] = fgetc(fp);
						if(buffer[n] == EOF)
							break;
						n++;
					}
					fclose(fp);
					if(strcmp(status, "200") == 0)
						printf("\nThe web page is successfully cached with filename %s", filename);
					printf("\nThe web page is successfully sent to the client...\n");	
				}		        	
				n=write(newsockfd,buffer,strlen(buffer));
				bzero(buffer, 25000);
		   	}
		   }/* End of inner while */
	}/* End of outer while */
	
	return 0;
} /*End of main */

/* Function to connect to the client's requested web server */
void search_webpage()
{
	int sockfd, portno, n;
    	struct sockaddr_in serv_addr;
    	struct hostent *original_server;
	FILE *fp;

        int i, j, flag = 0;
        char buf[25000];
        char host[30], request[80], filename[20], rec[50], tmp_url[50], command[50];

    	portno = 80; /* HTTP port number */

        sockfd = socket(AF_INET, SOCK_STREAM, 0);
    	if (sockfd < 0) 
       		error(EXIT_FAILURE, 0, "ERROR opening socket");

    	original_server = gethostbyname(url);
    	if (original_server == NULL) /* Client has requested a wrong URL */
	{
		strcpy(status, "ERR");
		return;
    	}

    	bzero((char *) &serv_addr, sizeof(serv_addr));
    	serv_addr.sin_family = AF_INET;
    	bcopy((char *)original_server->h_addr, (char *)&serv_addr.sin_addr.s_addr, original_server->h_length);
    	serv_addr.sin_port = htons(portno);
    	if(connect(sockfd,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0) 
		error(EXIT_FAILURE, 0, "ERROR connecting");

	/* Searching list.txt to check whether the page is already cached or not */
	if(count != 0)
	{
		fp = fopen("list.txt", "r");
		for(i = 0; i < count; i++)
		{
			fscanf(fp, "%s", tmp_url);
			if(strcmp(url, tmp_url) == 0) /* Web page is already cached */
			{
				flag = 1;
				break;
			}
		}
		fclose(fp);
	}

	if(flag == 1) /* Web page is already cached */ 
	{
		/* Set the status as available */
		strcpy(status, "AVL");
		return;		
	}
	else /* Page is not cached */
	{
		/* Sending the HTTP request */
 		strcpy(request, "GET / HTTP/1.1\r\nHost:");
    		strcat(request, url);
    		strcat(request, "\r\n\r\n");
    		n = write(sockfd,request,strlen(request));
    		if(n < 0)
			error(EXIT_FAILURE, 0, "ERROR writing to socket");
    		bzero(buf,25000);
    		n = read(sockfd,buf,25000);
    		if(n < 0)
			error(EXIT_FAILURE, 0, "ERROR reading from socket");
           	close(sockfd);

		/* Getting the HTTP status number */
		status[0] = buf[9];
    		status[1] = buf[10];
    		status[2] = buf[11];
    		status[3] = '\0';

		if(strcmp(status, "200") != 0) /* Page has some error. Information will be directly sent to client without caching */
		{
			strcpy(buffer, buf);
			return;
		}
		else /* Page is successfully downloaded */
		{
			file_updater(); /* Updating the list.txt file with new web page entry */
		}
		
		/* Copying the downloaded webpage into proxy server's cache with timestamp as filename */ 
/*new_cache:*/	strcpy(filename, url);
	    	strcat(filename, ".html");
	    	fp = fopen(filename, "w");
	    	i = 0;
	    	while(buf[i] != '\0')
	    	{
			j = buf[i++];
			fputc(j, fp);
	    	}
	    	fclose(fp);
	}
}

/* list.txt file updater */
void file_updater()
{
	FILE *fp, *ft;
	int i = 0;
	char command[50], l_url[50], r_url[50];

	if(count < 5) /* Number of entry is less than 5, so just add the record at end*/
	{
		fp = fopen("list.txt", "a");
		fprintf(fp, "%s\n", url);
		count = count + 1;
		fclose(fp);
	}
	else /* The list.txt file already has 5 records, so delete the first record (oldest one) and add new record at end */
	{
		fp = fopen("list.txt", "r"); /* Original list file */
		ft = fopen("temp.txt", "w"); /* Temporary file */

		/* Copy the last 4 records from list.txt to temp.txt */
		i = 0;
		while(i < 5) 
		{
			fscanf(fp, "%s", l_url);
			i++;
			if(i == 1)
				strcpy(r_url, l_url); /* Get the first URL in the list for file removal */
			else /* Not the first record in list.txt */
				fprintf(ft, "%s\n", l_url); /* Copying the URL to the temporary file temp.txt */
		}

		fprintf(ft, "%s\n", url); /* Adding the new record at end */
		fclose(fp);
		fclose(ft);

		system("rm list.txt"); /* Removing old list.txt */
		system("mv temp.txt list.txt");	/* Renaming temp.txt to list.txt */

		/* Removing old file from proxy server cache */
		strcpy(command, "rm ");
		strcat(command, r_url);
		strcat(command, ".html");
		system(command); 	
	}
}