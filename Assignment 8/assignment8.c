#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <sys/wait.h>
#include <time.h>

#define SERVER_PORT 49999
#define BACK_LOG 1

void server();
void client(char *address);

int main(int argc, char **argv)
{
	// parse argv here, run client / server depending on argv[1]
	if (argc == 1) {
		fprintf(stderr, "%s\n",
		        "Usage: ./assignment8 <client/server> <address>");
		return 1;
	}

	if (strcmp(argv[1], "server") == 0) {
		server();
	} else if (strcmp(argv[1], "client") == 0) {
		if (argc >= 3) {
			client(argv[2]);
		} else {
			fprintf(stderr, "%s\n",
			        "Error: client must recieve server adress");
			return 1;
		}
	}
	return 0;
}

void server() {
	int listenfd = socket(AF_INET, SOCK_STREAM, 0);

	setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &(int) {1},
	           sizeof(int));

	struct sockaddr_in serv_addr;
	memset(&serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(SERVER_PORT);
	serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);

	if (bind(listenfd, (struct sockaddr *) &serv_addr,
	         sizeof(serv_addr)) < 0) {
		perror("Error: ");
		exit(errno);
	}

	listen(listenfd, BACK_LOG);

	int clientfd;
	struct sockaddr_in client_addr;
	int length = sizeof(client_addr);

	while (1) {
		while (waitpid(-1, NULL, WNOHANG) > 0);

		clientfd = accept(listenfd, (struct sockaddr *) &client_addr,
		                  (socklen_t *) &length);
		if (fork()) {
			close(clientfd);
			continue;
		}

		char client_name[NI_MAXHOST];
		int client_entry;
		client_entry = getnameinfo((struct sockaddr*) &client_addr,
		                        sizeof(client_addr),
		                        client_name,
		                        sizeof(client_name),
		                        NULL,
		                        0,
		                        NI_NUMERICSERV); // idk what flag to set here
		if(client_entry)
			fprintf(stderr, "Error: %s\n", 
			        gai_strerror(client_entry));
		else
			printf("%s\n", client_name);
		exit(client_entry);
	}
}

void client(char *address) {
	int status;
	struct addrinfo hints;
	struct addrinfo *res;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;

	char port[NI_MAXSERV] = {0};

	sprintf(port, "%d", SERVER_PORT);

	// NOTE: error check later
	status = getaddrinfo(address, port, &hints, &res);

	int sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);

	// NOTE: loop and connect to first address that succeeds (later)
	// NOTE: also error check later
	if(connect(sockfd, res->ai_addr,
	           sizeof(res->ai_addr)) == 0) {
		perror("Error: ");
		exit(errno);
	}
}