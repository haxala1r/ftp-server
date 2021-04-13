#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include "cmds.h"
#include "../helpers.h"

struct command *str_to_cmd(char *raw) {
	if (raw == NULL) {
		return NULL;
	}

	/* Allocate the cmd struct. */
	struct command *cmd = malloc(sizeof(*cmd));
	memset(cmd, 0, sizeof(*cmd));

	/* Find the type of command this is. */
	int i = 0;
	while (1) {
		if (cmd_funcs[i] == NULL) {
			free(cmd);
			return NULL;
		}
		if (!strncmp(raw, cmd_funcs[i], strlen(cmd_funcs[i]))) {
			cmd->type = i;
			break;
		}
		i++;
	}

	/* If the command doesn't take any arguments, return.*/
	if (strlen(cmd_funcs[i]) == strlen(raw)) {
		return cmd;
	}

	/* Copy over the rest of the string as argument. */
	char *src = raw + strlen(cmd_funcs[i]) + 1;
	char *dest = malloc(strlen(src) + 1);
	strcpy(dest, src);
	dest[strlen(src)] = '\0';

	cmd->arg = dest;

	return cmd;
};



int handle_cmd(struct command *cmd, struct connection *conn) {
	if (cmd == NULL) {
		return 1;
	}
	if (conn == NULL) {
		return 1;
	}

	/* Non-privilidged commands. */
	switch (cmd->type) {
	case FTP_USER:
		ftp_cmd_user(cmd, conn);
		return 0;

	case FTP_PASS:
		ftp_cmd_pass(cmd, conn);
		return 0;

	case FTP_QUIT:
		ftp_cmd_quit(cmd, conn);
		return 1;

	default:
		break;
	}

	/* User-privilidged commands. */
	if (conn->logged_in == 0) {
		write(conn->fd, need_login, strlen(need_login));
		return 0;
	}

	switch (cmd->type) {
	case FTP_SYST:
		write(conn->fd, system_info, strlen(system_info));
		break;

	case FTP_LIST:
		ftp_cmd_list(cmd, conn);
		break;

	case FTP_PORT:
		ftp_cmd_port(cmd, conn);
		break;

	case FTP_PASV:
		ftp_cmd_pasv(cmd, conn);
		break;

	case FTP_CWD:
		ftp_cmd_cwd(cmd, conn);
		break;

	case FTP_PWD:
		ftp_cmd_pwd(cmd, conn);
		break;

	case FTP_RETR:
		ftp_cmd_retr(cmd, conn);
		break;

	case FTP_STOR:
		ftp_cmd_stor(cmd, conn);
		break;

	case FTP_DELE:
		ftp_cmd_dele(cmd, conn);
		break;

	case FTP_RMD:
		ftp_cmd_rmd(cmd, conn);
		break;

	case FTP_MKD:
		ftp_cmd_mkd(cmd, conn);
		break;

	default:
		write(conn->fd, not_implemented, strlen(not_implemented));
		conn->data_port = 0;
		conn->data_ip = 0;
		break;
	}

	return 0;
};


