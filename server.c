#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/in.h>

#define MAX 15000

int main(int argc, char *argv[])
{
	struct addrinfo hints, *res;
	int status;
	char inbuff[MAX], outbuff[MAX];

	if (argc < 2)
	{
		printf("Usage: ./server <port>\nExample: ./server 3000\n");
		return 1;
	}

	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_INET; // AF_INET or AF_INET6 to force version
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;

	if ((status = getaddrinfo(NULL, argv[1], &hints, &res)) != 0)
	{
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(status));
		return 2;
	}

	int sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
	
	if (bind(sockfd, res->ai_addr, res->ai_addrlen) == -1)
	{
		perror("bind");
		fprintf(stderr, "Error: Failed to bind to port %s. Port may already be in use.\n", argv[1]);
		freeaddrinfo(res);
		close(sockfd);
		return 4;
	}
	
	listen(sockfd, 3); //up to 3 connection requets can be queued (backlog = 3)

	while (1)
	{
		struct sockaddr_storage their_addr;
		socklen_t addr_size = sizeof their_addr;
		int new_fd = accept(sockfd, (struct sockaddr *)&their_addr, &addr_size);
		
		if (new_fd == -1)
		{
			perror("accept");
			continue;
		}

		printf("A new client connected to the server:\n");

		while (1)
		{
			// Print client IP address
			char client_ip[INET_ADDRSTRLEN];			
			struct sockaddr_in *client_addr = (struct sockaddr_in *)&their_addr;
			inet_ntop(AF_INET, &client_addr->sin_addr, client_ip, INET_ADDRSTRLEN);
			printf("%s $", client_ip);
			
			if (strcmp(inbuff, "exit") == 0)
			{
				printf("Client disconnected\n $");
				break;
			}
            
			fgets(outbuff, MAX, stdin);
			outbuff[strcspn(outbuff, "\n")] = 0; // Remove newline character

			send(new_fd, outbuff, strlen(outbuff), 0);

			memset(inbuff, 0, MAX);
		    
			while (1)
			{
				recv(new_fd, inbuff, MAX, 0);

				int n = strlen(inbuff) - 1;
				
				if (inbuff[n] == '*' && inbuff[n - 1] == '*' && inbuff[n - 2] == '*')
				{
					inbuff[n - 2] = '\0';
					printf("%s", inbuff);
					break;
				}

				printf("%s", inbuff);
			}
		    
			memset(inbuff, 0, MAX);
			strcpy(inbuff, "");
		}

		close(new_fd);
	}

	close(sockfd);
	return 0;
}


