#include "myftp.h"

void client(const char *address);

// char *get_command();

int main(int argc, char **argv)
{
	// check if any input is received
	if (argc == 1) {
		fprintf(stderr, "%s\n", "Error: No server adress specified");
		return 1;
	}
	
	client(argv[1]);
	
	return 0;
}

// NOTE: this function is taken from my assignment 8 source code
void client(const char *address)
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