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

	if (conn->logged_in == 0) {
		write(conn->fd, need_login, strlen(need_login));
		return;
	}

	dprintf(conn->fd, "257 Your current working directory is '%s'\n", conn->working_dir);
	return;
};
