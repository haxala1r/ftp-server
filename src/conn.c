#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include "cmds/cmds.h"
#include "helpers.h"


int open_data_conn(struct connection *conn) {
	if (conn == NULL) { return -1; }

	/* This function establishes a data connection. If  a PORT or PASV command
	 * was not sent before, this will obviously fail, but that should not be a
	 * problem for a correctly-functioning FTP client.
	 */

	if (conn->passive == 0) {
		/* Active mode. */
		write(conn->fd, opening_data_conn, strlen(opening_data_conn));

		if ((conn->data_port == 0) || (conn->data_ip == 0)) {
			/* A PORT command was not sent before this one. */
			write(conn->fd, cant_open_data, strlen(cant_open_data));
			return -1;
		}

		/* Create the socket. */
		int data_sock = socket(AF_INET, SOCK_STREAM, 0);
		struct sockaddr_in addr;
		memset(&addr, 0, sizeof(addr));

		/* Set the port and address. */
		addr.sin_family = AF_INET;
		addr.sin_addr.s_addr = htonl(conn->data_ip);
		addr.sin_port = htons(conn->data_port);

		if (connect(data_sock, (struct sockaddr *)&addr, sizeof(addr))) {
			/* Inform client of the failure. */
			write(conn->fd, cant_open_data, strlen(cant_open_data));
			close(data_sock);
			return -1;
		}

		return data_sock;
	} else {
		/* Passive mode. */
		write(conn->fd, opening_data_conn, strlen(opening_data_conn));

		if (conn->pasv_sock == 0) {
			/* A PASV command was not sent before this. */
			write(conn->fd, cant_open_data, strlen(cant_open_data));
			return -1;
		}

		/* PASV command should have set a passive socket to accept from. */
		struct sockaddr_in addr;
		int addr_size = sizeof(addr);
		int data_sock = accept(conn->pasv_sock, (struct sockaddr *)&addr, &addr_size);

		if (data_sock < 0) {
			write(conn->fd, cant_open_data, strlen(cant_open_data));
			return -1;
		}

		close(conn->pasv_sock);
		return data_sock;
	}
};



int close_data_conn(int fd, struct connection *conn) {
	if (conn == NULL) { return -1; }
	if (fd < 0)       { return -1; }

	close(fd);

	conn->data_ip = 0;
	conn->data_port = 0;
	conn->passive = 0;
	conn->pasv_sock = 0;

	return 0;
};

