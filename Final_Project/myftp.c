#include "myftp.h"

int ctrl_conn(const char *address);


void handle_exit(int ctrl_sock);
void handle_cd(char *path);
void handle_rcd(int ctrl_sock, char *path);
void handle_ls();
void handle_rls(int ctrl_sock);
void handle_get(int ctrl_sock, char *path);
void handle_show(int ctrl_sock, char *path);
void handle_put(int ctrl_sock, char *path);

// reads from a file descriptor until a "\n" or EOF is reached or number of bytes read is == size
// fd should only contain at most 1 line while the function executes
// returns length of line on success, -1 on error
int get_line(int fd, char *result, int size);

// return 0 or port num if recieved A, return -1 on E, print error message
int handle_response(int ctrl_sock);

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
	while ((actual = get_line(0, buffer, MAX_COMMAND_LENGTH)) > 0) {
		char *cmd = strtok(buffer, " ");
		char *path = strtok(NULL, " ");

		if (cmd == NULL) {
			printf("MYFTP> ");
			fflush(stdout);
			memset(buffer, 0, MAX_COMMAND_LENGTH);
			continue;
		}

		if (!strcmp(cmd, "exit")) {
			// printf("handle_exit()\n");
			handle_exit(ctrl_sock);
		} else if (!strcmp(cmd, "cd")) {
			printf("handle_cd()\n");
			// handle_cd(path);
		} else if (!strcmp(cmd, "rcd")) {
			printf("handle_rcd()\n");
			// handle_rcd(ctrl_sock, path);
		} else if (!strcmp(cmd, "ls")) {
			printf("handle_ls()\n");
			// handle_ls();
		} else if (!strcmp(cmd, "rls")) {
			printf("handle_rls()\n");
			// handle_rls(ctrl_sock);
		} else if (!strcmp(cmd, "get")) {
			printf("handle_get()\n");
			// handle_get(ctrl_sock, path);
		} else if (!strcmp(cmd, "show")) {
			printf("handle_show()\n");
			// handle_show(ctrl_sock, path);
		} else if (!strcmp(cmd, "put")) {
			printf("handle_put()\n");
			// handle_put(ctrl_sock, path);
		} else {
			printf("Command '%s' is unknown - ignored\n", cmd);
		}

		printf("MYFTP> ");
		fflush(stdout);
		memset(buffer, 0, MAX_COMMAND_LENGTH);
	}

	if (actual < 0) {
		// error check read() here
		// i don't think this is a fatal error, consider kep trying to read
	}

	return 0;
}

// still haven't checked edge cases, debug more later
int get_line(int fd, char *result, int size) {
	char buffer[READ_BUFFER_SIZE] = { 0 };
	int actual = 0;
	int offset = 0;
	while ((actual = read(fd, buffer, READ_BUFFER_SIZE)) > 0 && offset < size) {
		if ((offset + actual) >= size)
			actual = size - offset;

		memcpy(result + offset, buffer, actual);
		offset += actual;

		int i = 0;
		while (result[i] != '\n' && i < offset)
			i++;
		if (i < offset) {
			result[i] = '\0';
			break;
		}
		memset(buffer, 0, READ_BUFFER_SIZE);
	}

	if (actual < 0) {
		return -1;
	}

	return offset;
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

void handle_exit(int ctrl_sock) {
	// length is arbitrary for now, this is done to account for if the server responds with E
	char response[512] = {0};
	char status = 0;
	write(ctrl_sock, "Q\n", 2);
	// call handle_response here
	read(ctrl_sock, &status, 1);
	if (status == 'E') {
		printf("ERROR OCCURED\n");
		// while(actual = read(ctrl_sock, response, 512) > 0) {
		// 	// print error 512 bytes at a time
		// }
	}
	close(ctrl_sock);
	exit(0);
}