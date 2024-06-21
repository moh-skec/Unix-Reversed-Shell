#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <pthread.h>

#define MAX 15000
#define BACKLOG 10

struct ClientInfo
{
	int sockfd;
	struct sockaddr_storage addr;
};

void *handle_client(void *arg) {
    
    	struct ClientInfo *client = (struct ClientInfo *)arg;
    	int new_fd = client->sockfd;
    	struct sockaddr_storage their_addr = client->addr;
    	char inbuff[MAX], outbuff[MAX];
    	
    while (1) {    
        // Send message to client
        fgets(outbuff, MAX, stdin);
        outbuff[strcspn(outbuff, "\n")] = 0; // Remove newline character
        send(new_fd, outbuff, strlen(outbuff), 0);
        
        while(1){
        	// Receive message from client
		if (recv(new_fd, inbuff, MAX, 0) <= 0) {
		    perror("recv");
		    close(new_fd);
		    pthread_exit(NULL);
		}

		int n = strlen(inbuff) - 1;

		// Check for termination message
		if (inbuff[n] == '*' && inbuff[n - 1] == '*' && inbuff[n - 2] == '*') {
		    inbuff[n - 2] = '\0';
		    printf("%s", inbuff);
		    break;
		}
	}
	
        printf("%s", inbuff);
        memset(inbuff, 0, MAX);
    }

    // close(new_fd);
    pthread_exit(NULL);
}

int main(int argc, char *argv[]) {
    struct addrinfo hints, *res;
    int status;

    if (argc < 2) {
        printf("Usage: ./paraserver <port>\nExample: ./paraserver 3000\n");
        return 1;
    }

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET; // AF_INET or AF_INET6 to force version
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    if ((status = getaddrinfo(NULL, argv[1], &hints, &res)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(status));
        return 2;
    }

    int sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);

    if (bind(sockfd, res->ai_addr, res->ai_addrlen) == -1) {
        perror("bind");
        fprintf(stderr, "Error: Failed to bind to port %s. Port may already be in use.\n", argv[1]);
        freeaddrinfo(res);
        close(sockfd);
        return 4;
    }

    listen(sockfd, 10); // up to 10 connection requests can be queued (backlog = 10)
    pthread_t client_threads[BACKLOG];
    int client_count = 0;
    while (1) {
        struct sockaddr_storage their_addr;
        socklen_t addr_size = sizeof their_addr;
        int new_fd = accept(sockfd, (struct sockaddr *)&their_addr, &addr_size);

        if (new_fd == -1) {
            perror("accept");
            continue;
        }

        printf("A new client connected to the server:\n");
	
	struct ClientInfo *client = malloc(sizeof(struct ClientInfo));
	client->sockfd = new_fd;
	memcpy(&client->addr, &their_addr, sizeof their_addr);

        pthread_create(&client_threads[client_count], NULL, handle_client, (void *)client);
        
        client_count++;
        
        if (client_count >= BACKLOG)
        {
        	printf("Maximum number of clients reached. No more connections will be accepted\n");
        	break;
        }
        
        //pthread_detach(thread_id); // Detach thread to avoid memory leaks
    }
    
    
    for(int i = 0; i < client_count; i++)
    {
    	        pthread_join(client_threads[i], NULL);
    }
    	        
    close(sockfd);
    return 0;
}

