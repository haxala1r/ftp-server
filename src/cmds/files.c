#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdint.h>
#include <fcntl.h>
#include <sys/sendfile.h>
#include <sys/stat.h>

#include "cmds.h"
#include "../helpers.h"


/* RETR - Retrieve file. */
void ftp_cmd_retr(struct command *cmd, struct connection *conn) {
	if (cmd == NULL)  { return; }
	if (conn == NULL) { return; }

	/* Construct the path to the file. */
	char *fpath = malloc(strlen(conn->working_dir) + strlen(cmd->arg) + 2);
	fpath[0] = '.';
	fpath[1] = '\0';

	strcat(fpath, conn->working_dir);
	strcat(fpath, cmd->arg);

	/* Open the file.*/
	int fd = open(fpath, O_RDWR);
	if (fd < 0) {
		conn->data_port = 0;
		conn->data_ip = 0;
		write(conn->fd, file_unavailable, strlen(file_unavailable));
		return;
	}

	/* Now that we're sure the file exists and can be transferred, open data conn.*/
	int data_sock = open_data_conn(conn);
	if (data_sock < 0){
		close(fd);
		return;
	}

	/* Get the file size, then send the file. */
	int flen = lseek(fd, 0, SEEK_END);
	lseek(fd, 0, SEEK_SET);

	sendfile(data_sock, fd, NULL, flen);
	close(fd);

	/* Transfer complete. */
	write(conn->fd, closing_data_conn, strlen(closing_data_conn));
	close_data_conn(data_sock, conn);
	return;
};


/* STOR - Store file. */
void ftp_cmd_stor(struct command *cmd, struct connection *conn) {
	if (cmd == NULL)  { return; }
	if (conn == NULL) { return; }

	/* First, open the file. */
	char *fpath = malloc(strlen(conn->working_dir) + strlen(cmd->arg) + 3);
	fpath[0] = '.';
	fpath[1] = '\0';

	strcat(fpath, conn->working_dir);
	strcat(fpath, cmd->arg);

	int fd = open(fpath, O_WRONLY | O_CREAT | O_TRUNC, S_IRWXU);
	free(fpath);

	if (fd < 0) {
		write(conn->fd, file_unavailable, strlen(file_unavailable));
		return;
	}

	/* Open the data connection. */
	int data_sock = open_data_conn(conn);
	if (data_sock < 0) {
		close(fd);
		return;
	}

	/* Read from the socket, then write to the file.  */
	int block_size = 0x1000;
	int bytes_read = 0;
	char *buf = malloc(block_size);

	while ((bytes_read = read(data_sock, buf, block_size)) > 0) {
		write(fd, buf, bytes_read);
	}
	free(buf);
	close(fd);

	write(conn->fd, closing_data_conn, strlen(closing_data_conn));
	close_data_conn(data_sock, conn);
	return;
};


/* DELE - Delete file. */
void ftp_cmd_dele(struct command *cmd, struct connection *conn) {
	if (cmd == NULL)  { return; }
	if (conn == NULL) { return; }

	char *fpath = construct_full_path(cmd->arg, conn->working_dir);

	int s = unlink(fpath);
	free(fpath);

	if (s < 0) {
		write(conn->fd, file_unavailable, strlen(file_unavailable));
		return;
	}

	write(conn->fd, delete_success, strlen(delete_success));
	return;
};


/* RMD - Remove Directory. */
void ftp_cmd_rmd(struct command *cmd, struct connection *conn) {
	if (cmd == NULL)  { return; }
	if (conn == NULL) { return; }

	char *fpath = construct_full_path(cmd->arg, conn->working_dir);

	int s = rmdir(fpath);
	free(fpath);

	if (s < 0) {
		write(conn->fd, file_unavailable, strlen(file_unavailable));
		return;
	}

	write(conn->fd, delete_success, strlen(delete_success));
	return;
};


/* MKD - Make Directory. */
void ftp_cmd_mkd(struct command *cmd, struct connection *conn) {
	if (cmd == NULL)  { return; }
	if (conn == NULL) { return; }

	char *fpath = construct_full_path(cmd->arg, conn->working_dir);

	int s = mkdir(fpath, 0744);
	free(fpath);

	if (s < 0) {
		write(conn->fd, file_unavailable, strlen(file_unavailable));
		return;
	}

	write(conn->fd, succ, strlen(succ));
	return;
};


