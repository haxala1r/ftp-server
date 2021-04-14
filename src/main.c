#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include "cmds/cmds.h"
#include "helpers.h"


void serve_client(struct connection *conn) {
	/* Send the welcome message. */
	write(conn->fd, welcome, strlen(welcome));

	/* Repeatedly recieve commands from the client. */
	while (1) {
		char *msg = read_line(conn->fd, 4);
		if (strlen(msg) < 3) {
			/* There's no command. */
			free(msg);
			continue;
		}

		/* Parse the message. */
		struct command *cmd = str_to_cmd(msg);
		free(msg);


		if (cmd == NULL) {
			/* A valid command was not detected. */
			write(conn->fd, not_implemented, strlen(not_implemented));
			continue;
		}

		if (handle_cmd(cmd, conn)) {
			/* A non-zero return value indicates the connection terminated. */
			free(cmd);
			break;
		}

		if (cmd->arg != NULL) {
			free(cmd->arg);
		}
		free(cmd);
	}
	return;
};


int main(int argc, char *argv[]) {
	/* Load the users and their passwords from a file. */
	if (ftp_init_users(get_users_arg(argc, argv))) {
		puts("Error occured while loading user info.\n");
		return  1;
	}

	int port = get_port_arg(argc, argv);

	/* Create the listening socket, and then... well, listen.*/
	int sd = socket(AF_INET, SOCK_STREAM, 0);
	int opt_val = 1;

	setsockopt(sd, SOL_SOCKET, SO_REUSEPORT, &opt_val, sizeof(opt_val));
	struct sockaddr_in saddr;

	saddr.sin_family = AF_INET;
	saddr.sin_addr.s_addr = INADDR_ANY;
	saddr.sin_port = htons(port);

	if (bind(sd, (struct sockaddr *)&saddr, sizeof(saddr))) {
		puts("Bind failed.\n");
		close(sd);
		return 1;
	}


	listen(sd, 3);

	printf("Listening on port %d\n", port);

	while (1) {
		/* Wait for a new connection, then accept. */
		int size = sizeof(struct sockaddr_in);
		struct sockaddr_in *client = malloc(sizeof(*client));

		int connfd = accept(sd, (struct sockaddr *)client, (socklen_t *)&size);

		/* Create the new connection object. */
		struct connection *conn = malloc(sizeof(*conn));
		memset(conn, 0, sizeof(*conn));
		conn->fd = connfd;
		conn->client = client;

		conn->working_dir = malloc(2);
		conn->working_dir[0] = '/';
		conn->working_dir[1] = '\0';

		/* Spawn new process to handle the connection. */
		int pid = fork();
		if (pid < 0) {
			perror("main:  can't fork()");
			break;
		}
		if (pid == 0) {
			close(sd);

			/* Change directory, then serve it. */
			chdir(get_dir_arg(argc, argv));
			serve_client(conn);


			close(conn->fd);
			exit(0);
		}
		close(conn->fd);
		free(conn->working_dir);
		free(conn);
		printf("Spawned proces %4d for a new client.\n", pid);
	}

	close(sd);
	return 0;
}
