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

/* Description:
 *	Listens for connections and logs the name of the client and how many 
 *	total connections where received.
 *
 * Params:
 *	None
 *
 * Return:
 *	None
 */
void server();

/* Description:
 *	Connects to the specified address and prints a 18 byte long message 
 *	recieved by the server (in this case, the date.)
 *
 * Params:
 *	address: the name of the server to connect to
 *
 * Return:
 *	None
 */
void client(const char *address);

/* Description:
 *	This program can run in two ways, as a server or a client (specified by
 *	the first command line arg.) If ran as a server, it will print the name
 *	of every client, and how many total connections have been recieved. If 
 *	ran as a client, an address must be past which specifies the server to 
 *	connect to. A date will be recieved by the server and printed to 
 *	stdout.
 *
 * Return:
 *	0 if everything went successful, 1 if input args were not valid. If any
 *	socket error occured, the cooresponding error message will be printed 
 *	along with the error code.	
 */
int main(int argc, char **argv)
{
	// check if any input is received
	if (argc == 1) {
		fprintf(stderr, "%s\n",
		        "Usage: ./assignment8 <client/server> <address>");
		return 1;
	}

	// run server code if "server" arg is passed
	if (strcmp(argv[1], "server") == 0) {
		server();
	} else if (strcmp(argv[1], "client") == 0) {
		// run client if address arg is given
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

	// remove port already in use error by setting socket options
	if (setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &(int) {1},
	    sizeof(int))) {
		perror("Error: ");
		exit(errno);
	}

	// set up server adress info
	struct sockaddr_in serv_addr;
	memset(&serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(SERVER_PORT);
	// just use the wildcard IP
	serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);

	// bind socket to ip and port
	if (bind(listenfd, (struct sockaddr *) &serv_addr,
	         sizeof(serv_addr)) < 0) {
		perror("Error: ");
		exit(errno);
	}

	// set socket as listening
	if (listen(listenfd, BACK_LOG)) {
		perror("Error: ");
		exit(errno);
	}

	int clientfd;
	struct sockaddr_in client_addr;
	int length = sizeof(client_addr);

	int total_connections = 0;

	// set up daemon behavior
	while (1) {
		// clear zombie children
		while (waitpid(-1, NULL, WNOHANG) > 0);

		// wait for a connection
		clientfd = accept(listenfd, (struct sockaddr *) &client_addr,
		                  (socklen_t *) &length);

		if (clientfd == -1) {
			perror("Error: ");
			exit(errno);
		}

		total_connections++;

		if (fork()) {
			// parent doesn't need this
			close(clientfd);
			continue;
		}

		// get the client name from address info
		char client_name[NI_MAXHOST];
		int client_entry;
		client_entry = getnameinfo((struct sockaddr *) &client_addr,
		                           sizeof(client_addr),
		                           client_name,
		                           sizeof(client_name),
		                           NULL,
		                           0,
		                           NI_NUMERICSERV);

		// print client name and total connections if successful
		if (client_entry) {
			fprintf(stderr, "Error: %s\n",
			        gai_strerror(client_entry));
			close(clientfd);
			exit(client_entry);
		} else {
			printf("%s %d\n", client_name, total_connections);
		}

		// get current time in seconds
		const time_t seconds = time(NULL);
		char date[26] = {0};

		// convert seconds to string date
		if (ctime_r(&seconds, date) == NULL) {
			perror("Error: ");
			close(clientfd);
			exit(errno);
		}

		// send 18 bytes of date info to client
		if (write(clientfd, date, 18) == -1) {
			perror("Error: ");
			close(clientfd);
			exit(errno);
		}

		close(clientfd);
		exit(0);
	}
}

void client(const char *address) {
	int status;
	struct addrinfo hints;
	struct addrinfo *res;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;

	// stringify the port number
	char port[NI_MAXSERV] = {0};
	sprintf(port, "%d", SERVER_PORT);

	// get the address info of the server
	status = getaddrinfo(address, port, &hints, &res);

	if (status) {
		fprintf(stderr, "Error: %s\n", gai_strerror(status));
		exit(status);
	}

	/*
	create the socket with the same family, socktype, and protocal as the 
	first result from getaddrinfo
	*/
	int sockfd = socket(res->ai_family, res->ai_socktype,
	                    res->ai_protocol);

	if (sockfd == -1) {
		perror("Error: ");
		exit(errno);
	}

	// try connecting to the first address returned by getaddrinfo
	if (connect(sockfd, res->ai_addr, res->ai_addrlen)) {
		perror("Error: ");
		close(sockfd);
		exit(errno);
	}

	// 19 bytes to include the null terminator
	char date[19] = {0};

	// read the date from the server
	if (read(sockfd, date, 18) == -1) {
		perror("Error: ");
		close(sockfd);
		exit(errno);
	}

	// print the date
	printf("%s\n", date);
	close(sockfd);
}