#include "cmds.h"
#include "../helpers.h"
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>



struct user *users;
int user_count;


int ftp_init_users(char *users_file) {
	int fd = open(users_file, O_RDONLY);

	/* The first line should contain the amount of users. */
	char *line = read_line(fd, 1);
	user_count = atoi(line);

	if (user_count == 0) {
		users = NULL;
		close(fd);
		return -1;
	}
	free(line);
	users = malloc(user_count * sizeof(*users));
	memset(users, 0, user_count * sizeof(*users));

	/* Now read the rest of the lines, which all contain a username:password pair. */
	for (int i = 0; i < user_count; i++) {
		line = read_line(fd, 1);
		int len = 0;

		/* Get username. */
		while (line[len] != ':') {
			/* Grow the string, and add the character to the end. */
			users[i].username = realloc(users[i].username, len + 1);
			users[i].username[len] = line[len];

			if (line[len] == '\0') {
				close(fd);
				free(line);
				free(users);
				users = NULL;
				user_count = 0;
				return -1;
			}

			len++;
		}

		/* Get password*/
		char *str = line + len + 1;
		len = 0;

		while (1) {
			users[i].password = realloc(users[i].password, len + 1);
			users[i].password[len] = str[len];
			if (str[len] == '\0') {
				break;
			}
			len++;
		}


		free(line);
	}

	close(fd);
	return 0;
};



void ftp_cmd_user(struct command *cmd, struct connection *conn) {
	if (cmd == NULL)  { return; }
	if (conn == NULL) { return; }

	if (cmd->arg == NULL) {
		write(conn->fd, login_invalid, strlen(login_invalid));
		return;
	}

	/* Look for the username. */
	for (int i = 0; i < user_count; i++) {
		if (!strcmp(users[i].username, cmd->arg)) {
			write(conn->fd, need_pass, strlen(need_pass));
			conn->u = users + i;
			return;
		}
	}

	/* The username was not found. */
	write(conn->fd, login_invalid, strlen(login_invalid));
	return;
};

void ftp_cmd_pass(struct command *cmd, struct connection *conn) {
	if (cmd == NULL)  { return; }
	if (conn == NULL) { return; }

	if (cmd->arg == NULL) {
		write(conn->fd, login_invalid, strlen(login_invalid));
		return;
	}

	/* Check if username has been specified. */
	if (conn->u == NULL) {
		write(conn->fd, bad_sequence, strlen(bad_sequence));
		return;
	}


	/* Check the password. */
	if (!strcmp(cmd->arg, conn->u->password)) {
		/* Correct password. */
		conn->logged_in = 1;
		conn->u->online++;
		write(conn->fd, login_success, strlen(login_success));
		return;
	}

	/* The password is invalid. */
	conn->logged_in = 0;
	conn->u = NULL;
	write(conn->fd, login_invalid, strlen(login_invalid));
};


void ftp_cmd_quit(struct command *cmd, struct connection *conn) {
	if (cmd == NULL)  { return; }
	if (conn == NULL) { return; }

	if ((conn->u != NULL) && (conn->logged_in)) {
		conn->u->online--;
		printf("User %s quit.\n", conn->u->username);
	}

	conn->logged_in = 0;
	conn->u = NULL;

	write(conn->fd, logout_success, strlen(logout_success));
	close(conn->fd);
	return;
};


