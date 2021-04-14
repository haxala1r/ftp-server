#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdint.h>

#include "cmds.h"
#include "../helpers.h"


/* CWD - Change Working Directory. Must not be used to move up in the hierarchy.*/
void ftp_cmd_cwd(struct command *cmd, struct connection *conn) {
	if (cmd == NULL)   { return; }
	if (conn == NULL)  { return; }

	if (cmd->arg == NULL) {
		return;
	}

	/* CWD should not be used to move to the parent directory. */
	for (int i = 0; i < strlen(cmd->arg); i++) {
		if (!strncmp(cmd->arg + i, "..", 2)) {
			write(conn->fd, invalid_param, strlen(invalid_param));
			return;
		}
	}

	char *slash = "/";
	conn->working_dir = realloc(conn->working_dir, strlen(conn->working_dir) + 3 + strlen(cmd->arg));
	strcat(conn->working_dir, cmd->arg);
	strcat(conn->working_dir, slash);

	write(conn->fd, succ, strlen(succ));
	return;
};

/* PWD - Print Working Directory. */
void ftp_cmd_pwd(struct command *cmd, struct connection *conn) {
	if (cmd == NULL)   { return; }
	if (conn == NULL)  { return; }

	dprintf(conn->fd, "257 Your current working directory is '%s'\n", conn->working_dir);
	return;
};

/* CDUP - Move to the parent directory. */
void ftp_cmd_cdup(struct command *cmd, struct connection *conn) {
	if (cmd == NULL)   { return; }
	if (conn == NULL)  { return; }

	int cur_len = strlen(conn->working_dir);
	if (cur_len <= 2) {
		write(conn->fd, succ, strlen(succ));
		return;
	}

	int len = 0;
	for (int i = cur_len - 2; i >= 0; i--, len++) {
		if (conn->working_dir[i] == '/') {
			conn->working_dir[i + 1] = '\0';
			break;
		}
	}

	/* Now resize the string. */
	conn->working_dir = realloc(conn->working_dir, cur_len - len + 1);

	write(conn->fd, succ, strlen(succ));
	return;
};

