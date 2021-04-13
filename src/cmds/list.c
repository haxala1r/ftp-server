#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>

#include "cmds.h"
#include "../helpers.h"


void ftp_cmd_list(struct command *cmd, struct connection *conn) {
	if (cmd == NULL)   { return; }
	if (conn == NULL)  { return; }

	int data_sock = open_data_conn(conn);
	if (data_sock < 0) { return;}

	/* First, open the directory. */
	char *path = malloc(strlen(conn->working_dir) + 2);
	path[0] = '.';
	path[1] = '\0';
	strcat(path, conn->working_dir);

	DIR *cdfd = opendir(path);
	struct dirent *ent;

	/* Send directory entries. */
	struct stat fs;

	while ((ent = readdir(cdfd)) != NULL) {
		/* Construct the path of the file relative to the working directory. */
		char *file_path = malloc(strlen(path) + strlen(ent->d_name) + 1);
		strcpy(file_path, path);
		strcat(file_path, ent->d_name);

		stat(file_path, &fs);
		free(file_path);
		/* Determine file type. */
		char ftype = '-';

		if (S_ISREG(fs.st_mode)) {
			ftype = '-';
		} else if (S_ISDIR(fs.st_mode)) {
			ftype = 'd';
		} else if (S_ISCHR(fs.st_mode)) {
			ftype = 'c';
		} else if (S_ISBLK(fs.st_mode)) {
			ftype = 'b';
		} else if (S_ISLNK(fs.st_mode)) {
			ftype = 'l';
		}


		/* Determine file permissions. */
		char uperm[4] = "---";
		char gperm[4] = "---";
		char operm[4] = "---";

		if (fs.st_mode & 00400) {
			uperm[0] = 'r';
		}
		if (fs.st_mode & 00200) {
			uperm[1] = 'w';
		}
		if (fs.st_mode & 00100) {
			uperm[2] = 'x';
		}

		if (fs.st_mode & 00040) {
			gperm[0] = 'r';
		}
		if (fs.st_mode & 00020) {
			gperm[1] = 'w';
		}
		if (fs.st_mode & 00010) {
			gperm[2] = 'x';
		}

		if (fs.st_mode & 00004) {
			operm[0] = 'r';
		}
		if (fs.st_mode & 00002) {
			operm[1] = 'w';
		}
		if (fs.st_mode & 00001) {
			operm[2] = 'x';
		}

		dprintf(data_sock, "%c%s%s%s %8d %s \n", ftype, uperm, gperm, operm, fs.st_size, ent->d_name);
	}

	closedir(cdfd);
	free(path);

	write(conn->fd, closing_data_conn, strlen(closing_data_conn));
	close_data_conn(data_sock, conn);
	return;
};
