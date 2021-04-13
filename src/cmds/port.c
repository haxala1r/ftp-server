#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdint.h>

#include "cmds.h"
#include "../helpers.h"


/* PORT - Specify a port to connect to. */
void ftp_cmd_port(struct command *cmd, struct connection *conn) {
	if (cmd == NULL)  { return; }
	if (conn == NULL) { return; }

	int ip1, ip2, ip3, ip4;
	int port1, port2;

	sscanf(cmd->arg, "%d,%d,%d,%d,%d,%d", &ip1, &ip2, &ip3, &ip4, &port1, &port2);

	/* Big endian is used for these.*/
	conn->data_ip = (ip1 << 24) | (ip2 << 16) | (ip3 << 8) | ip4;
	conn->data_port = (port1 << 8) | port2;

	/* The client wants an active connection. */
	conn->passive = 0;

	/* Send a success message. */
	dprintf(conn->fd, "200 PORT command successful.\n");
	return;
};


/* TODO: PASV - Enter passive mode */
void ftp_cmd_pasv(struct command *cmd, struct connection *conn) {
	if (cmd == NULL)  { return; }
	if (conn == NULL) { return; }

	int ip = conn->client->sin_addr.s_addr;
	int port = gen_port();

	/* Divide it into its bytes - Big endian, most significant byte first. */
	int ip1 = ip & 0xFF;
	int ip2 = (ip >> 8) & 0xFF;
	int ip3 = (ip >> 16) & 0xFF;
	int ip4 = (ip >> 24) & 0xFF;

	int port1 = port & 0xFF;
	int port2 = (port >> 8) & 0xFF;

	/* Now create a socket, and start listening. don't accept yet - that will be
	 * done when open_data_conn() is called.
	 */
	conn->pasv_sock = socket(AF_INET, SOCK_STREAM, 0);
	if (conn->pasv_sock < 0) {
		conn->pasv_sock = 0;
		write(conn->fd, internal_err, strlen(internal_err));
		return;
	}

	/* Bind socket, and set it to listening mode. */
	struct sockaddr_in paddr;
	int addr_len = sizeof(paddr);
	paddr.sin_family = AF_INET;
	paddr.sin_addr.s_addr = INADDR_ANY;
	paddr.sin_port = port;

	if (bind(conn->pasv_sock, (struct sockaddr *)&paddr, addr_len)) {
		close(conn->pasv_sock);
		conn->pasv_sock = 0;
		write(conn->fd, internal_err, strlen(internal_err));
		return;
	}

	listen(conn->pasv_sock, 1);

	conn->passive = 1;
	dprintf(conn->fd, "227 Entering passive mode (%d,%d,%d,%d,%d,%d) \n", \
	        ip1, ip2, ip3, ip4, port1, port2);
	return;
};

