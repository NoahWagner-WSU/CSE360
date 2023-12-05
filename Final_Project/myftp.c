#include "myftp.h"

/*
TODO:
- Any repeated code in put / show / get functions, make into their own functions (same on server)
- fix all valgrind errors
- double check all error handling
	- lots of function calls currently aren't checked if they fail
- check with prof if the way I wait() in handle_ls is good
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

// fills type and message with response from server
// type can be filled to be either 'E' or 'A'
// message can be '\0' or <port number> if 'A', NULL if 'E' (ignored if passed NULL)
// returns 0 on success, -1 if server unexpectidly closed or invalid type read, and errno if we failed to read
int handle_response(int ctrl_sock, char *type, char **port);

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
	fprintf(stderr, "%s\n", strerror(errno));
	return errno;
}

void handle_command(int ctrl_sock, char *cmd, char *path, char *address)
{
	if (!strcmp(cmd, "exit")) {
		free(cmd);
		handle_exit(ctrl_sock);
	} else if (!strcmp(cmd, "cd")) {
		handle_cd(path);
	} else if (!strcmp(cmd, "rcd")) {
		handle_rcd(ctrl_sock, path);
	} else if (!strcmp(cmd, "ls")) {
		handle_ls();
	} else if (!strcmp(cmd, "rls")) {
		handle_rls(ctrl_sock, address);
	} else if (!strcmp(cmd, "get")) {
		handle_get(ctrl_sock, path, address);
	} else if (!strcmp(cmd, "show")) {
		handle_show(ctrl_sock, path, address);
	} else if (!strcmp(cmd, "put")) {
		handle_put(ctrl_sock, path, address);
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
	send_msg(ctrl_sock, 'D', NULL);

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
		exit(errno);
	}

	if (bytes_read == 0) {
		fprintf(stderr, "Error: control socket closed unexpectedly\n");
		exit(EXIT_FAILURE);
	}

	char *line = get_line(ctrl_sock);

	if (line == NULL) {
		fprintf(stderr, "Error: %s\n", strerror(errno));
		exit(errno);
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

void send_msg(int ctrl_sock, char type, char *msg)
{
	int tmp_errno;
	if (write(ctrl_sock, &type, 1) == -1) {
		tmp_errno = errno;
		fprintf(stderr,
		        "Fatal Write Error: %s\n",
		        strerror(tmp_errno));
		exit(tmp_errno);
	}

	if (msg) {
		if (write(ctrl_sock, msg, strlen(msg)) == -1) {
			tmp_errno = errno;
			fprintf(stderr,
			        "Fatal Write Error: %s\n",
			        strerror(tmp_errno));
			exit(tmp_errno);
		}
	}

	char new_line = '\n';

	if (write(ctrl_sock, &new_line, 1) == -1) {
		tmp_errno = errno;
		fprintf(stderr,
		        "Fatal Write Error: %s\n",
		        strerror(tmp_errno));
		exit(tmp_errno);
	}
}

int copy(int src, int dst)
{
	int actual;
	char buffer[READ_BUFFER_SIZE] = {0};
	while ((actual = read(src, buffer, READ_BUFFER_SIZE)) > 0)
		write(dst, buffer, actual);
	return actual;
}

void handle_exit(int ctrl_sock)
{
	// NOTE: error check later
	send_msg(ctrl_sock, 'Q', NULL);

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

	send_msg(ctrl_sock, 'C', path);

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

	if (datafd == -1)
		return;

	int error;
	char res_type;

	send_msg(ctrl_sock, 'L', NULL);

	handle_response(ctrl_sock, &res_type, NULL);

	// NOTE: better error checking later
	int f1 = fork();

	if (f1 > 0) {
		close(datafd);
		wait(NULL);
		return;
	} else if (f1 == -1) {
		fprintf(stderr, "Error: %s\n", strerror(errno));
		close(datafd);
		return;
	}

	// replace stdin with datafd
	close(0); dup(datafd); close(datafd);
	execlp("more", "more", "-20", (char *) NULL);
	fprintf(stderr, "Error: %s\n", strerror(errno));
	exit(errno);
}

void handle_get(int ctrl_sock, char *path, char *address)
{
	if (!path) {
		fprintf(stderr, "Command error: expecting a parameter.\n");
		return;
	}

	char *file_name = basename(path);

	if (access(file_name, F_OK) == 0) {
		fprintf(stderr,
		        "Get Canceled: %s already exists in current directory\n",
		        file_name);
		return;
	}

	if (errno != ENOENT) {
		fprintf(stderr, "Get Error: %s\n", strerror(errno));
		return;
	}

	// establish a data connection once we know we can take the file
	int datafd = est_data_conn(ctrl_sock, address);

	if (datafd == -1)
		return;

	int error;

	// send 'G' and start reading from datafd
	send_msg(ctrl_sock, 'G', path);

	// handle server response
	char res_type;
	handle_response(ctrl_sock, &res_type, NULL);

	if (res_type == 'E') {
		close(datafd);
		return;
	}

	int newfd = open(file_name, O_CREAT | O_EXCL | O_WRONLY, 0644);

	if (newfd == -1) {
		fprintf(stderr, "File Open Error: %s\n", strerror(errno));
		close(datafd);
		return;
	}

	if (copy(datafd, newfd) < 0) {
		fprintf(stderr, "Error: %s\n", strerror(errno));
		unlink(file_name);
	}

	close(newfd); close(datafd);
}

void handle_show(int ctrl_sock, char *path, char *address)
{
	if (!path) {
		fprintf(stderr, "Command error: expecting a parameter.\n");
		return;
	}

	int datafd = est_data_conn(ctrl_sock, address);

	if (datafd == -1)
		return;

	// send 'G' and start reading from datafd
	send_msg(ctrl_sock, 'G', path);

	int error;
	char res_type;
	handle_response(ctrl_sock, &res_type, NULL);

	if (res_type == 'E') {
		close(datafd);
		return;
	}

	int f1 = fork();

	if (f1 > 0) {
		close(datafd);
		wait(NULL);
		return;
	} else if (f1 == -1) {
		fprintf(stderr, "Error: %s\n", strerror(errno));
		close(datafd);
		return;
	}

	// replace stdin with datafd
	close(0); dup(datafd); close(datafd);
	execlp("more", "more", "-20", (char *) NULL);
	fprintf(stderr, "Error: %s\n", strerror(errno));
	exit(errno);
}

void handle_put(int ctrl_sock, char *path, char *address)
{
	if (!path) {
		fprintf(stderr, "Command error: expecting a parameter.\n");
		return;
	}

	if (access(path, R_OK)) {
		fprintf(stderr, "Put Canceled: %s\n", strerror(errno));
		return;
	}

	struct stat s;
	if (lstat(path, &s)) {
		fprintf(stderr, "Put Canceled: %s\n", strerror(errno));
		return;
	}

	if (!S_ISREG(s.st_mode)) {
		fprintf(stderr, "Put Canceled: File is not regular\n");
		return;
	}

	int openfd = open(path, O_RDONLY);

	if(openfd == -1) {
		fprintf(stderr, "File Open Error: %s\n", strerror(errno));
		return;
	}

	int datafd = est_data_conn(ctrl_sock, address);

	if (datafd == -1)
		return;

	send_msg(ctrl_sock, 'P', basename(path));

	int error;
	char res_type;
	handle_response(ctrl_sock, &res_type, NULL);

	if(res_type == 'E') {
		close(datafd);
		return;
	}

	if (copy(openfd, datafd) < 0)
		fprintf(stderr, "Read Error: %s\n", strerror(errno));

	close(datafd); close(openfd);
}