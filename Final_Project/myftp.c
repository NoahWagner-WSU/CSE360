#include "myftp.h"

int ctrl_conn(const char *address);

// loops until a valid command is read from stdin, then returns it
// char *get_command();

int main(int argc, char **argv)
{
	// check if any input is received
	if (argc == 1) {
		fprintf(stderr, "%s\n", "Error: No server adress specified");
		return 1;
	}
	
	int ctrl_sock = ctrl_conn(argv[1]);

	// start command loop here
	char buffer[MAX_COMMAND_LENGTH] = {0};
	int actual = 0;

	printf("MYFTP> ");
	fflush(stdout);
	while((actual = read(1, buffer, MAX_COMMAND_LENGTH)) > 0) {
		printf("%s", buffer);
		printf("MYFTP> ");
	}

	if(actual < 0) {
		// error check read() here
	}

	// get_command(stdin)
	
	return 0;
}

// NOTE: this function is taken from my assignment 8 source code
int ctrl_conn(const char *address)
{
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

	printf("Connected to server %s\n", address);

	return sockfd;
}