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

void handle_command(int ctrl_sock, char *cmd, char *path);

// reads from a file descriptor until a "\n" or EOF is reached
// fd should only contain at most 1 line while the function executes
// returns the line on success, NULL on error
char *get_line(int fd);

// return port num in string form (or empty string) if recieved A, return -1 on E, print error message
char *handle_response(int ctrl_sock);

int main(int argc, char **argv)
{
	// check if any input is received
	if (argc == 1) {
		fprintf(stderr, "%s\n", "Error: No server adress specified");
		return 1;
	}

	int ctrl_sock = ctrl_conn(argv[1]);

	// start command loop here
	char *line;

	printf("MYFTP> ");	
	fflush(stdout);

	while ((line = get_line(0)) != NULL) {
		char *cmd = strtok(line, " ");
		char *path = strtok(NULL, " ");

		if (cmd != NULL)
			handle_command(ctrl_sock, cmd, path);

		free(line);
		printf("MYFTP> ");
		fflush(stdout);
	}

	// get_line failed, meaning read() failed
	// i don't think this is a fatal error, consider kep trying to read
	fprintf(stderr, "%s\n", strerror(errno));
	return errno;
}

void handle_command(int ctrl_sock, char *cmd, char *path) {
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
}

// function might not be full proof
char *get_line(int fd) {
	char *result = NULL;
	int result_size = 0;

	char buffer[READ_BUFFER_SIZE] = {0};
	int actual = 0;

	while ((actual = read(fd, buffer, READ_BUFFER_SIZE)) > 0) {
		
		char *tmp = calloc(result_size + actual, 1);

		if(result) {
			memcpy(tmp, result, result_size);
			free(result);
		}
		memcpy(tmp + result_size, buffer, actual);

		result = tmp;
		result_size += actual;

		int i = 0;
		while (result[i] != '\n' && i < result_size)
			i++;
		if (i < result_size) {
			result[i] = '\0';
			break;
		}
		memset(buffer, 0, READ_BUFFER_SIZE);
	}

	if (actual < 0) {
		if(result)
			free(result);
		return NULL;
	}

	return result;
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
		perror("Error");
		exit(errno);
	}

	// try connecting to the first address returned by getaddrinfo
	if (connect(sockfd, res->ai_addr, res->ai_addrlen)) {
		perror("Error");
		close(sockfd);
		exit(errno);
	}

	printf("Connected to server %s\n", address);

	return sockfd;
}

char *handle_response(int ctrl_sock) {
	char type;
	read(ctrl_sock, &type, 1);
	if(type == 'E') {
		char *error = get_line(ctrl_sock);
		fprintf(stderr, "Server Error: %s\n", error);
		free(error);
		return NULL;
	}

	return get_line(ctrl_sock);
}
	
void handle_exit(int ctrl_sock) {
	// NOTE: error check this later
	write(ctrl_sock, "Q\n", 2);
	char *response = handle_response(ctrl_sock);
	free(response);
	close(ctrl_sock);
	exit(0);
}