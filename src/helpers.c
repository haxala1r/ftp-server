#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <time.h>

char *read_line(int fd, int block_size) {
	char c[block_size];
	char *buf = NULL;
	int len = 0;

	while (1) {
		int amount_read = read(fd, c, block_size);
		if (amount_read <= 0) {
			break;
		}
		/* Copy the read message to the buffer. */
		buf = realloc(buf, len + block_size);
		memcpy(buf + len, c, amount_read);
		len += amount_read;

		/* Check if we reached the end of the message. */
		for (int i = 0; i < block_size; i++) {
			if ((c[i] == '\r') || (c[i] == '\n') || (c[i] == '\0')) {
				c[i] = '\0';
				memcpy(buf + len - amount_read, c, amount_read);
				goto ret;
			}
		}
	}

ret:
	buf = realloc(buf, len + 1);
	buf[len] = '\0';
	return buf;
};


/* This function is used to combine a command's argument and the working_dir
 * attribute of a connection into a full path.
 */
char *construct_full_path(char *file_name, char *working_dir) {
	if (!file_name || !working_dir) {
		return NULL;
	}

	char *fpath = malloc(strlen(file_name) + strlen(working_dir) + 2);
	if (fpath == NULL) {
		return NULL;
	}

	fpath[0] = '.';
	fpath[1] = '\0';

	/* conn->working_dir always begins with a slash*/
	strcat(fpath, working_dir);
	strcat(fpath, file_name);

	return fpath;
};




/* Returns a random, unprivilidged, valid port number to be used. */
int gen_port() {
	srand(time(NULL));
	int port = (rand() + 1025) % 0xffff;
	return port;
};



