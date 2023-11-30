#include "myftp.h"

/*
TODO:
- Make my rls work with his client / server
- Make every fail in communication (send_command / handle_response) with server fatal (aka. call exit())
- fix all valgrind errors
- double check all error handling
	- lots of function calls currently aren't checked if they fail
- check with prof if the way I wait() in handle_ls is good
- Ask prof if we send back A before or after sending data
*/

// NOTE: code from setup_ctrl_conn, get_server_addr, and connect_to_server is taken from assignment 8
int setup_conn(int *fd, const char *address, const char *port);
int get_server_addr(const char *address, const char *port,
                    struct addrinfo **resinfo);
int connect_to_server(struct addrinfo *info, int *sockfd);
int est_data_conn(int ctrl_sock, const char *address);

void handle_exit(int ctrl_sock);
void handle_cd(char *path);
void handle_rcd(int ctrl_sock, char *path);
void handle_ls();
void handle_rls(int ctrl_sock, char *address);
void handle_get(int ctrl_sock, char *path, char *address);
void handle_show(int ctrl_sock, char *path, char *address);
void handle_put(int ctrl_sock, char *path, char *address);

void handle_command(int ctrl_sock, char *cmd, char *path, char *address);

// reads from a file descriptor until a "\n" or EOF is reached
// fd should only contain at most 1 line while the function executes
// returns the line on success, NULL on error
char *get_line(int fd);

// fills type and message with response from server
// type can be filled to be either 'E' or 'A'
// message can be '\0' or <port number> if 'A', NULL if 'E' (ignored if passed NULL)
// returns 0 on success, -1 if server unexpectidly closed or invalid type read, and errno if we failed to read
int handle_response(int ctrl_sock, char *type, char **port);

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

	// stringify the port number
	char port[NI_MAXSERV] = {0};
	sprintf(port, "%d", SERVER_PORT);

	int ctrl_sock;
	int error;
	if ((error = setup_conn(&ctrl_sock, argv[1], port)))
		exit(error);

	printf("Connected to server %s\n", argv[1]);

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

int setup_conn(int *fd, const char *address, const char *port)
{
	struct addrinfo *info;
	int error;
	if ((error = get_server_addr(address, port, &info)))
		return error;

	if ((error = connect_to_server(info, fd)))
		return error;

	freeaddrinfo(info);

	return 0;
}

int est_data_conn(int ctrl_sock, const char *address)
{
	int error = send_command(ctrl_sock, 'D', NULL);

	if (error)
		fprintf(stderr, "Error: %s\n", strerror(error));

	char res_type;
	char *port = NULL;
	if (handle_response(ctrl_sock, &res_type, &port) || res_type == 'E')
		return -1;

	int datafd;

	if (setup_conn(&datafd, address, port)) {
		free(port);
		return -1;
	}

	free(port);

	return datafd;
}

int handle_response(int ctrl_sock, char *type, char **port)
{
	// NOTE: error check later
	int bytes_read = read(ctrl_sock, type, 1);

	if (bytes_read < 0) {
		fprintf(stderr, "Error: %s\n", strerror(errno));
		return errno;
	}

	if (bytes_read == 0) {
		fprintf(stderr, "Error: control socket closed unexpectedly\n");
		return -1;
	}

	char *line = get_line(ctrl_sock);

	if (line == NULL) {
		fprintf(stderr, "Error: %s\n", strerror(errno));
		return errno;
	}

	if (*type == 'E') {
		fprintf(stderr, "Server Error: %s\n", line);
		free(line);
		return 0;
	}

	if (*type != 'A') {
		fprintf(stderr,
		        "Error: Unrecognized server response type %c\n", *type);
		return -1;
	}

	if (port)
		*port = line;
	else
		free(line);

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
		exit(error);
	}

	char type = 0;
	handle_response(ctrl_sock, &type, NULL);
	exit(0);
}

void handle_cd(char *path)
{
	if (!path) {
		fprintf(stderr, "Command error: expecting a parameter.\n");
		return;
	}

	if (access(path, R_OK)) {
		fprintf(stderr, "Change directory: %s\n", strerror(errno));
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
	handle_response(ctrl_sock, &type, NULL);
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
	int datafd = est_data_conn(ctrl_sock, address);

	int error;
	char res_type;
	if ((error = send_command(ctrl_sock, 'L', NULL)))
		fprintf(stderr, "Error: %s\n", strerror(error));

	if ((error = handle_response(ctrl_sock, &res_type, NULL)))
		exit(error);

	if (datafd == -1)
		return;

	// NOTE: better error checking later
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

void handle_get(int ctrl_sock, char *path, char *address)
{

	// first check last bit of file path to see if it already exists
	// return if file already exists, send error message to stderr
	// probably use access here

	// establish a data connection once we know we can take the file
	int datafd = est_data_conn(ctrl_sock, address);

	if (datafd == -1)
		return;

	// open the new file with the last part of path as its name

	// send 'G' and start reading from datafd

	// handle server response
}