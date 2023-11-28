#include "myftp.h"
#define BACK_LOG 4

/*
TODO:
- Make rcd and cd fail when trying to cd into non-readable directories
- Do a passthrough of error checking (double check all system call error handling)
	-waitpid will need more error checking
- Implement a data connection establisher type function
- Ask Prof about error checking respond() (if write() fails) to client (how should we debug / respond, is it fatal?)
*/

// returns -1 on error (sends cooresponding error message to stderr), and socketfd on success
// if port is 0, assigned_port will be filled with the port chosen by bind()
// NULL can be passed to assigned_port if you aren't interested in it
int init_socket(int port, int back_log, int *assigned_port);
int ctrl_conn_loop(int listenfd);

int send_bytes(int clientfd, char *bytes, int length);

// sends a response back to the client
// type is either 'A' or 'E'
// message is either NULL, a port num, or an error message
// message must have a null terminator
// returns 0 on success, errno on error
int respond(int clientfd, char type, char *message);

void handle_Q(int clientfd);
void handle_C(int clientfd, char *path);
void handle_D(int clientfd, char *path);
void handle_L(int datafd);
void handle_G(int datafd, char *path);
void handle_P(int datafd, char *path);

char *get_line(int fd);

int main(int argc, char **argv)
{
	int listenfd = init_socket(SERVER_PORT, BACK_LOG, NULL);

	if (listenfd == -1) {
		return errno;
	}

	int clientfd = ctrl_conn_loop(listenfd);

	// we are now in the child process

	char *line;
	while ((line = get_line(clientfd)) != NULL) {
		if (line[0] == 'Q') {
			free(line);
			handle_Q(clientfd);
		} else if (line[0] == 'C') {
			handle_C(clientfd, line + 1);
		} else if (line[0] == 'D') {
			handle_D(clientfd, line + 1);
		} else {
			int error = respond(clientfd, 'E',
			                    "Unrecognized control command");

			if (error)
				fprintf(stderr, "Error: %s\n",
				        strerror(error));
		}
		free(line);
	}

	return 0;
}

// NOTE: the source of this function is taken from my assignment 8 source code
int init_socket(int port, int back_log, int *assigned_port)
{
	int listenfd = socket(AF_INET, SOCK_STREAM, 0);

	if (listenfd == -1) {
		fprintf(stderr, "Error: %s\n", strerror(errno));
		return -1;
	}

	// remove port already in use error by setting socket options
	if (setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &(int) {1},
	    sizeof(int))) {
		fprintf(stderr, "Error: %s\n", strerror(errno));
		return -1;
	}

	// set up server adress info
	struct sockaddr_in serv_addr;
	memset(&serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(port);
	// just use the wildcard IP
	serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);

	// bind socket to ip and port
	if (bind(listenfd, (struct sockaddr *) &serv_addr,
	                sizeof(serv_addr)) < 0) {
		fprintf(stderr, "Error: %s\n", strerror(errno));
		return -1;
	}

	// retrieve the assigned port
	if (port == 0) {
		// set up server adress info
		struct sockaddr_in serv_addr;
		memset(&serv_addr, 0, sizeof(serv_addr));
		int length = sizeof(serv_addr);
		if (getsockname(listenfd, (struct sockaddr *) &serv_addr,
		                (socklen_t *) &length)) {
			fprintf(stderr, "Error: %s\n", strerror(errno));
			return -1;
		}
		if (assigned_port)
			*assigned_port = ntohs(serv_addr.sin_port);
	}

	// set socket as listening
	if (listen(listenfd, back_log)) {
		fprintf(stderr, "Error: %s\n", strerror(errno));
		return -1;
	}

	return listenfd;
}

// NOTE: this function is taken from my assignment 8 source code
int ctrl_conn_loop(int listenfd)
{
	int clientfd;
	struct sockaddr_in client_addr;
	int length = sizeof(client_addr);

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
			printf("Child %d: Connection accepted from host %s\n",
			       getpid(), client_name);
		}

		return clientfd;
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

int send_bytes(int clientfd, char *bytes, int length)
{
	int result = 0;
	int actual = 0;
	while (actual < length) {
		result = write(clientfd, bytes + actual, length - actual);
		if (result < 0)
			return errno;
		actual += result;
	}
	return 0;
}

int respond(int clientfd, char type, char *message)
{
	int error = send_bytes(clientfd, &type, 1);

	if (error)
		return error;

	if (message) {
		error = send_bytes(clientfd, message, strlen(message));

		if (error)
			return error;
	}

	char new_line = '\n';
	error = send_bytes(clientfd, &new_line, 1);

	if (error)
		return error;

	return 0;
}

void handle_Q(int clientfd)
{
	int error = respond(clientfd, 'A', NULL);

	if (error)
		fprintf(stderr, "Error: %s\n", strerror(error));

	close(clientfd);
	printf("Child %d: Quitting\n", getpid());
	exit(0);
}

void handle_C(int clientfd, char *path)
{
	int error = 0;
	if (chdir(path)) {
		error = respond(clientfd, 'E', strerror(errno));
		if (error)
			fprintf(stderr, "Error: %s\n", strerror(error));
		return;
	}

	error = respond(clientfd, 'A', NULL);

	if (error)
		fprintf(stderr, "Error: %s\n", strerror(error));
}

// NOTE: error check later
void handle_D(int clientfd, char* path)
{
	int port_num = 0;
	int listenfd = init_socket(0, 1, &port_num);

	if (listenfd == -1) {
		respond(clientfd, 'E', "Failed to initialize data socket");
		return;
	}

	// stringify the port number
	char port[NI_MAXSERV] = {0};
	sprintf(port, "%d", port_num);

	respond(clientfd, 'A', port);

	struct sockaddr_in client_addr;
	int length = sizeof(client_addr);

	int datafd = accept(listenfd, (struct sockaddr *) &client_addr,
	                    (socklen_t *) &length);

	char *line = get_line(clientfd);

	if (line[0] == 'L') {
		handle_L(datafd);
	} else if (line[0] == 'G') {
		// handle_G(datafd, path);
	} else if (line[0] == 'P') {
		// handle_P(datafd, path);
	} else {
		int error = respond(clientfd, 'E',
		                    "Unrecognized control command");

		if (error)
			fprintf(stderr, "Error: %s\n",
			        strerror(error));
	}
	free(line);
}

void handle_L(int datafd)
{
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

	// replace stdout with datafd
	// NOTE: perhaps make this a function called replace(1, datafd) and do error checking in there
	close(1); dup(datafd); close(datafd);
	execlp("ls", "ls", "-l", (char *) NULL);
	fprintf(stderr, "Error: %s\n", strerror(errno));
	exit(1);
}