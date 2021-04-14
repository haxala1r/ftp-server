
/* These are a couple helper functions not directly related to ftp. */
char *read_line(int fd, int block_size);
char *construct_full_path(char *file_name, char *working_dir);
int gen_port();

/* Extract certain arguments from the list passed to main(). */
int get_port_arg(int argc, char *argv[]);
char *get_dir_arg(int argc, char *argv[]);
char *get_users_arg(int argc, char *argv[]);
