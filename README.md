# ftp-server
A lightweight ftp server written in C.

# Implemented commands
USER, PASS: users can login using these. Valid usernames and passwords are drawn
from a file in the current working directory of the server, named 'users'.
each line contains an entry in the format of `username:password`.
a sample users file containing 3 users is provided.

PORT, PASV: Both active and passive mode are supported.

SYST, QUIT, LIST, CWD, PWD, RETR, STOR, DELE, RMD, MKD are all implemented.

If the server doesn't recognize a command, it will automatically send a
`502 Command not implemented.`

# Usage
Simply launch the resulting executable after building. It will start listening on
port 2121. Change this from the source if you like. The ability to specify this
in the command line will be implemented soon.
