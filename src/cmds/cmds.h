#include <sys/socket.h>
#include <arpa/inet.h>
#include <stddef.h>

/* Define command types. */
#define FTP_USER 0
#define FTP_PASS 1
#define FTP_SYST 2
#define FTP_QUIT 3
#define FTP_LIST 4
#define FTP_PORT 5
#define FTP_PASV 6
#define FTP_CWD  7
#define FTP_PWD  8
#define FTP_RETR 9
#define FTP_STOR 10
#define FTP_DELE 11
#define FTP_RMD  12
#define FTP_MKD  13




struct connection {
	int fd;
	struct sockaddr_in *client;
	int logged_in;
	struct user *u;

	char *working_dir;

	int passive;	/* Active if zero, passive otherwise. */

	/* These are used in Active mode. (with the PORT command) */
	int data_ip;
	int data_port;

	/* This is used in Passive mode. (with the PASV command) */
	int pasv_sock;
};

struct user {
	int online;
	char *username;
	char *password;
};

struct command {
	int type;
	char *arg;
};


struct command *str_to_cmd(char *raw);

int open_data_conn(struct connection *conn);
int close_data_conn(int fd, struct connection *conn);


void ftp_cmd_user(struct command *cmd, struct connection *conn);
void ftp_cmd_pass(struct command *cmd, struct connection *conn);
void ftp_cmd_quit(struct command *cmd, struct connection *conn);
void ftp_cmd_list(struct command *cmd, struct connection *conn);
void ftp_cmd_port(struct command *cmd, struct connection *conn);
void ftp_cmd_pasv(struct command *cmd, struct connection *conn);
void ftp_cmd_cwd(struct command *cmd, struct connection *conn);
void ftp_cmd_pwd(struct command *cmd, struct connection *conn);
void ftp_cmd_retr(struct command *cmd, struct connection *conn);
void ftp_cmd_stor(struct command *cmd, struct connection *conn);
void ftp_cmd_dele(struct command *cmd, struct connection *conn);
void ftp_cmd_rmd(struct command *cmd, struct connection *conn);
void ftp_cmd_mkd(struct command *cmd, struct connection *conn);


int ftp_init_users(char *users_file);


int handle_cmd(struct command *cmd, struct connection *conn);

static const char *cmd_funcs[] = {
	"USER", "PASS", "SYST", "QUIT", "LIST", "PORT", "PASV", "CWD", "PWD", "RETR", "STOR",
	"DELE", "RMD", "MKD", NULL
};

/* TODO: clean these up, and possibly dynamically load them from a file instead
 * of hardcoding them into the program.
 */
static const char *welcome = "220 - Welcome!\n";
static const char *login_success = "230 Login success.\n";
static const char *login_invalid = "430 Invalid username/password.\n";
static const char *logout_success = "221 Goodbye world!\n";
static const char *need_pass = "331 Please specify the password.\n";
static const char *system_info = "215 LINUX\n";
static const char *bad_sequence = "503 Bad sequence of commands.\n";
static const char *not_implemented = "502 This command is not implemented (yet).\n";
static const char *port_success = "200 Port command OK.\n";
static const char *file_unavailable = "550 File unavailable. \n";
static const char *need_login = "530 Please log in first.\n";
static const char *delete_success = "250 Delete success. \n";

static const char *opening_data_conn = "150 Opening data connection.\n";
static const char *closing_data_conn = "226 Command successful. Closing data connection.\n";
static const char *cant_open_data = "425 Falure while opening data connection.\n";
static const char *succ = "200 OK.\n";
static const char *internal_err = "451 Internal server error.\n";
