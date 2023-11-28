#include "myftp.h"

/*
TODO:
- Make my rls work with his client / server
- fix all valgrind errors
- double check all error handling
	- lots of function calls currently aren't checked if they fail
- Check with Prof if we need to free 100% mallocs
	- if we are exiting (say for exit handler or fatal error), couldn't we just let OS do it?
*/

// NOTE: code from setup_ctrl_conn, get_server_addr, and connect_to_server is taken from assignment 8
int setup_ctrl_conn(const char *address);
int setup_data_conn(const char *address, const char *port);

int get_server_addr(const char *address, const char *port,
                    struct addrinfo **resinfo);

int connect_to_server(struct addrinfo *info, int *sockfd);

void handle_exit(int ctrl_sock);
void handle_cd(char *path);
void handle_rcd(int ctrl_sock, char *path);
void handle_ls();
void handle_rls(int ctrl_sock, char *address);
void handle_get(int ctrl_sock, char *path);
void handle_show(int ctrl_sock, char *path);
void handle_put(int ctrl_sock, char *path);

void handle_command(int ctrl_sock, char *cmd, char *path, char *address);

// reads from a file descriptor until a "\n" or EOF is reached
// fd should only contain at most 1 line while the function executes
// returns the line on success, NULL on error
char *get_line(int fd);

// fills type and message with response from server
// type can be filled to be either 'E' or 'A'
// message can be '\0' or <port number> on 'A', and an error message on 'E'
// returns 0 on success, -1 if server unexpectidly closed, and errno if we failed to read
int handle_response(int ctrl_sock, char *type, char **message);

// sends a server command, appends a "\n", returns 0 on success, errno on error
int send_command(int ctrl_sock, char cmd, char *path);

// sends a series of bytes to the server, 0 on success, errno on error
int send_bytes(int ctrl_sock, char *bytes, int length);

int main(int argc, char **argv)
{
	// check if any input is received
	if (argc == 1) {
		fprintf(stderr, "%s\n", "Error: No server adress specified");
		return 1;
	}

	int ctrl_sock = setup_ctrl_conn(argv[1]);

	// start command loop here
	char *line;

	printf("MYFTP> ");
	fflush(stdout);

	while ((line = get_line(0)) != NULL) {
		char *cmd = strtok(line, " ");
		char *path = strtok(NULL, " ");

		if (cmd != NULL)
			handle_command(ctrl_sock, cmd, path, argv[1]);

		free(line);
		printf("MYFTP> ");
		fflush(stdout);
	}

	// get_line failed, meaning read() failed
	// i don't think this is a fatal error, consider kep trying to read
	fprintf(stderr, "%s\n", strerror(errno));
	return errno;
}

void handle_command(int ctrl_sock, char *cmd, char *path, char *address)
{
	if (!strcmp(cmd, "exit")) {
		// printf("handle_exit()\n");
		free(cmd);
		handle_exit(ctrl_sock);
	} else if (!strcmp(cmd, "cd")) {
		// printf("handle_cd()\n");
		handle_cd(path);
	} else if (!strcmp(cmd, "rcd")) {
		// printf("handle_rcd()\n");
		handle_rcd(ctrl_sock, path);
	} else if (!strcmp(cmd, "ls")) {
		// printf("handle_ls()\n");
		handle_ls();
	} else if (!strcmp(cmd, "rls")) {
		// printf("handle_rls()\n");
		handle_rls(ctrl_sock, address);
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
char *get_line(int fd)
{
	char *result = NULL;
	int result_size = 0;

	char buffer[READ_BUFFER_SIZE] = {0};
	int actual = 0;

	while ((actual = read(fd, buffer, READ_BUFFER_SIZE)) > 0) {

		char *tmp = calloc(result_size + actual, 1);

		if (result) {
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
		if (result)
			free(result);
		return NULL;
	}

	return result;
}

int get_server_addr(const char *address, const char *port,
                    struct addrinfo **info)
{
	int status;
	struct addrinfo hints;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;

	// get the address info of the server
	if ((status = getaddrinfo(address, port, &hints, info)))
		fprintf(stderr, "Error: %s\n", gai_strerror(status));

	return status;
}

int connect_to_server(struct addrinfo *info, int *sockfd)
{
	*sockfd = socket(info->ai_family, info->ai_socktype,
	                 info->ai_protocol);

	if (*sockfd == -1)
		return errno;

	// try connecting to the first address returned by getaddrinfo
	if (connect(*sockfd, info->ai_addr, info->ai_addrlen)) {
		fprintf(stderr, "Error: %s\n", strerror(errno));
		return errno;
	}

	return 0;
}

int setup_ctrl_conn(const char *address)
{
	// stringify the port number
	char port[NI_MAXSERV] = {0};
	sprintf(port, "%d", SERVER_PORT);

	struct addrinfo *info;
	int error = get_server_addr(address, port, &info);

	if (error)
		exit(error);

	int sockfd;
	error = connect_to_server(info, &sockfd);

	if (error)
		exit(error);

	printf("Connected to server %s\n", address);

	freeaddrinfo(info);

	return sockfd;
}

int setup_data_conn(const char *address, const char *port)
{
	struct addrinfo *info;
	if (get_server_addr(address, port, &info))
		return -1;

	int sockfd;
	if (connect_to_server(info, &sockfd))
		return -1;

	freeaddrinfo(info);

	return sockfd;
}

int handle_response(int ctrl_sock, char *type, char **message)
{
	// NOTE: error check later
	int bytes_read = read(ctrl_sock, type, 1);

	if (bytes_read < 0) {
		fprintf(stderr, "Error: %s\n", strerror(errno));
		return -1;
	}

	if (bytes_read == 0) {
		fprintf(stderr, "Error: control socket closed unexpectedly\n");
		return -1;
	}

	if (message)
		*message = get_line(ctrl_sock);

	if (*message == NULL) {
		fprintf(stderr, "Error: Failed to retrieve server response\n");
		return -1;
	}

	if (*type == 'E') {
		fprintf(stderr, "Server Error: %s\n", *message);
	} else if (*type != 'A') {
		fprintf(stderr,
		        "Error: Unrecognized server response type %c\n", *type);
		return -1;
	}

	return 0;
}

int send_command(int ctrl_sock, char cmd, char *path)
{
	int result = send_bytes(ctrl_sock, &cmd, 1);

	if (result)
		return result;

	if (path) {
		result = send_bytes(ctrl_sock, path, strlen(path));
		if (result)
			return result;
	}

	char new_line = '\n';
	result = send_bytes(ctrl_sock, &new_line, 1);

	if (result)
		return result;
	return 0;
}

int send_bytes(int ctrl_sock, char *bytes, int length)
{
	int result = 0;
	int actual = 0;
	while (actual < length) {
		result = write(ctrl_sock, bytes + actual, length - actual);
		if (result < 0)
			return errno;
		actual += result;
	}
	return 0;
}

void handle_exit(int ctrl_sock)
{
	// NOTE: error check later
	int error = send_command(ctrl_sock, 'Q', NULL);
	if (error) {
		fprintf(stderr, "Error: %s\n", strerror(error));
		exit(0);
	}

	char type = 0;
	char *message;
	error = handle_response(ctrl_sock, &type, &message);
	if (message)
		free(message);
	exit(0);
}

void handle_cd(char *path)
{
	if (!path) {
		fprintf(stderr, "Command error: expecting a parameter.\n");
		return;
	}

	if (chdir(path)) {
		fprintf(stderr, "Change directory: %s\n", strerror(errno));
		return;
	}
}

void handle_rcd(int ctrl_sock, char *path)
{
	if (!path) {
		fprintf(stderr, "Command error: expecting a parameter.\n");
		return;
	}

	int error = send_command(ctrl_sock, 'C', path);

	if (error) {
		fprintf(stderr, "Error: %s\n", strerror(error));
	}

	char type = 0;
	char *message;
	error = handle_response(ctrl_sock, &type, &message);
	if (message)
		free(message);
}

// NOTE: code taken from assignment 4
void handle_ls()
{
	// NOTE: error check later
	int fd[2];
	int rdr, wtr;
	pipe(fd);
	rdr = fd[0];
	wtr = fd[1];

	int f1 = fork();

	if (f1 > 0) {
		close(rdr); close(wtr);
		wait(NULL);
		return;
	} else if (f1 == -1) {
		fprintf(stderr, "Error: %s\n", strerror(errno));
		return;
	}

	int f2 = fork();

	if (f2 > 0) {
		close(wtr);
		// replace stdin with rdr
		close(0); dup(rdr); close(rdr);
		execlp("more", "more", "-20", (char *) NULL);
		fprintf(stderr, "Error: %s\n", strerror(errno));
		exit(1);
	} else if (f2 == 0) {
		close(rdr);
		// replace stdout with wtr
		close(1); dup(wtr); close(wtr);
		execlp("ls", "ls", "-l", (char *) NULL);
		fprintf(stderr, "Error: %s\n", strerror(errno));
		exit(1);
	}
	fprintf(stderr, "Error: %s\n", strerror(errno));
}

void handle_rls(int ctrl_sock, char *address)
{
	// the segmant below can be containerized into a single function
	/*________________________________________*/
	int error = send_command(ctrl_sock, 'D', NULL);

	if (error)
		fprintf(stderr, "Error: %s\n", strerror(error));

	char type;
	char *message;
	if (handle_response(ctrl_sock, &type, &message) || type == 'E') {
		if (message)
			free(message);
		return;
	}

	int datafd = setup_data_conn(address, message);

	if (datafd == -1) {
		free(message);
		return;
	}

	error = send_command(ctrl_sock, 'L', NULL);

	if (error)
		fprintf(stderr, "Error: %s\n", strerror(error));
	/*________________________________________*/

	// NOTE: error check later
	int f1 = fork();

	if (f1 > 0) {
		wait(NULL);
		close(datafd);
		return;
	} else if (f1 == -1) {
		fprintf(stderr, "Error: %s\n", strerror(errno));
		return;
	}

	// replace stdin with datafd
	close(0); dup(datafd); close(datafd);
	execlp("more", "more", "-20", (char *) NULL);
	fprintf(stderr, "Error: %s\n", strerror(errno));
	exit(1);
}