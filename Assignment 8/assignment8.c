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
void client(const char *address);

int main(int argc, char **argv)
{
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
			        "Error: client must receive server adress");
			return 1;
		}
	}
	return 0;
}

void server() {
	int listenfd = socket(AF_INET, SOCK_STREAM, 0);

	if (listenfd == -1) {
		perror("Error: ");
		exit(errno);
	}

	if (setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &(int) {1},
	    sizeof(int))) {
		perror("Error: ");
		exit(errno);
	}

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

	if (listen(listenfd, BACK_LOG)) {
		perror("Error: ");
		exit(errno);
	}

	int clientfd;
	struct sockaddr_in client_addr;
	int length = sizeof(client_addr);

	int total_connections = 0;

	while (1) {
		while (waitpid(-1, NULL, WNOHANG) > 0);

		clientfd = accept(listenfd, (struct sockaddr *) &client_addr,
		                  (socklen_t *) &length);

		if (clientfd == -1) {
			perror("Error: ");
			exit(errno);
		}

		total_connections++;

		if (fork()) {
			close(clientfd);
			continue;
		}

		char client_name[NI_MAXHOST];
		int client_entry;
		client_entry = getnameinfo((struct sockaddr *) &client_addr,
		                           sizeof(client_addr),
		                           client_name,
		                           sizeof(client_name),
		                           NULL,
		                           0,
		                           NI_NUMERICSERV);
		if (client_entry)
			fprintf(stderr, "Error: %s\n",
			        gai_strerror(client_entry));
		else
			printf("%s %d\n", client_name, total_connections);

		const time_t seconds = time(NULL);
		char date[26] = {0};

		if (ctime_r(&seconds, date) == NULL) {
			perror("Error: ");
			exit(errno);
		}

		date[18] = '\0';
		if (write(clientfd, date, 18) == -1) {
			perror("Error: ");
			exit(errno);
		}
		exit(client_entry);
	}
}

void client(const char *address) {
	int status;
	struct addrinfo hints;
	struct addrinfo *res;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;

	char port[NI_MAXSERV] = {0};

	sprintf(port, "%d", SERVER_PORT);

	status = getaddrinfo(address, port, &hints, &res);

	if (status) {
		fprintf(stderr, "Error: %s\n", gai_strerror(status));
		exit(status);
	}

	int sockfd = socket(res->ai_family, res->ai_socktype,
	                    res->ai_protocol);

	if (sockfd == -1) {
		perror("Error: ");
		exit(errno);
	}

	if (connect(sockfd, res->ai_addr, res->ai_addrlen)) {
		perror("Error: ");
		exit(errno);
	}

	char date[18] = {0};

	if (read(sockfd, date, 18) == -1) {
		perror("Error: ");
		exit(errno);
	}
	printf("%s\n", date);
}