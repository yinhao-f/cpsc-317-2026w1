#include "netbuffer.h"
#include "mailuser.h"
#include "server.h"
#include "util.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/utsname.h>
#include <ctype.h>

#define MAX_LINE_LENGTH 1024

typedef enum state {
    Undefined,
    // TODO: Add additional states as necessary
    AUTHORIZATION,
    TRANSACTION,
    UPDATE,
} State;

typedef struct serverstate {
    int fd;
    net_buffer_t nb;
    char recvbuf[MAX_LINE_LENGTH + 1];
    char *words[MAX_LINE_LENGTH];
    int nwords;
    State state;
    struct utsname my_uname;
    // TODO: Add additional fields as necessary
    char *user_name;
} serverstate;

static void handle_client(void *new_fd);

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Invalid arguments. Expected: %s <port>\n", argv[0]);
        return 1;
    }
    run_server(argv[1], handle_client);
    return 0;
}

// syntax_error returns
//   -1 if the server should exit
//    1 otherwise
int syntax_error(serverstate *ss) {
    if (send_formatted(ss->fd, "-ERR %s\r\n", "Syntax error in parameters or arguments") <= 0) return -1;
    return 1;
}

// checkstate returns
//   -1 if the server should exit
//    0 if the server is in the appropriate state
//    1 if the server is not in the appropriate state
int checkstate(serverstate *ss, State s) {
    if (ss->state != s) {
        if (send_formatted(ss->fd, "-ERR %s\r\n", "Bad sequence of commands") <= 0) return -1;
        return 1;
    }
    return 0;
}

// All the functions that implement a single command return
//   -1 if the server should exit
//    0 if the command was successful
//    1 if the command was unsuccessful

int do_quit(serverstate *ss) {
    // Note: This method has been filled in intentionally!
    dlog("Executing quit\n");
    send_formatted(ss->fd, "+OK Service closing transmission channel\r\n");
    ss->state = Undefined;
    return -1;
}

int do_user(serverstate *ss) {
    dlog("Executing user\n");
    // TODO: Implement this function
    int cs = checkstate(ss, AUTHORIZATION);
    if (cs != 0) return cs;
    if (ss->nwords != 2) return syntax_error(ss);
    
    char *user_name = trim_angle_brackets(ss->words[1]);
    int valid_user = is_valid_user(user_name, NULL);
    if (!valid_user) {
	dlog("Invalid user name %s\n", user_name);
	if (send_formatted(ss->fd, "-ERR user %s does not exist\r\n", user_name) <= 0) return -1;
	return 1;
    }
    
    dlog("User name %s valid, waiting for password\n", user_name);
    ss->user_name = strdup(user_name);
    if (send_formatted(ss->fd, "+OK Enter password for %s\r\n", user_name) <= 0) return -1;
    return 0;
}

int do_pass(serverstate *ss) {
    dlog("Executing pass\n");
    // TODO: Implement this function
    int cs = checkstate(ss, AUTHORIZATION);
    if (cs != 0) return cs;
    if (ss->nwords != 2) return syntax_error(ss);
    if (ss->user_name == NULL) {
	dlog("User name not provided yet\n");
	if (send_formatted(ss->fd, "-ERR Please enter a user name first\r\n") <= 0) return -1;
	return 1;
    }

    int valid_user = is_valid_user(ss->user_name, ss->words[1]);
    if (!valid_user) {
	dlog("Password incorrect\n");
	if (send_formatted(ss->fd, "-ERR Password incorrect\r\n") <= 0) return -1;
	return 1;
    }

    ss->state = TRANSACTION;
    dlog("User %s authenticated\n", ss->user_name);
    if (send_formatted(ss->fd, "+OK Welcome, %s\r\n", ss->user_name) <= 0) return -1;
    return 0;
}

int do_stat(serverstate *ss) {
    dlog("Executing stat\n");
    // TODO: Implement this function
    return 0;
}

int do_list(serverstate *ss) {
    dlog("Executing list\n");
    // TODO: Implement this function
    return 0;
}

int do_retr(serverstate *ss) {
    dlog("Executing retr\n");
    // TODO: Implement this function
    return 0;
}

int do_rset(serverstate *ss) {
    dlog("Executing rset\n");
    // TODO: Implement this function
    return 0;
}

int do_noop(serverstate *ss) {
    dlog("Executing noop\n");
    // TODO: Implement this function
    int cs = checkstate(ss, TRANSACTION);
    if (cs != 0) return cs;
    if (ss->nwords != 1) return syntax_error(ss);

    if (send_formatted(ss->fd, "+OK\r\n") <= 0) return -1;
    return 0;
}

int do_dele(serverstate *ss) {
    dlog("Executing dele\n");
    // TODO: Implement this function
    return 0;
}

void handle_client(void *new_fd) {
    int fd = *(int *)(new_fd);

    size_t len;
    serverstate mstate, *ss = &mstate;

    ss->fd = fd;
    ss->nb = nb_create(fd, MAX_LINE_LENGTH);
    ss->state = Undefined;
    uname(&ss->my_uname);
    // TODO: Initialize additional fields in `serverstate`, if any
    if (send_formatted(fd, "+OK POP3 Server on %s ready\r\n", ss->my_uname.nodename) <= 0) return;
    ss->state = AUTHORIZATION;

    while ((len = nb_read_line(ss->nb, ss->recvbuf)) >= 0) {
        if (ss->recvbuf[len - 1] != '\n') {
            // command line is too long, stop immediately
            send_formatted(fd, "-ERR Syntax error, command unrecognized\r\n");
            break;
        }
        if (strlen(ss->recvbuf) < len) {
            // received null byte somewhere in the string, stop immediately.
            send_formatted(fd, "-ERR Syntax error, command unrecognized\r\n");
            break;
        }
        // Remove CR, LF and other space characters from end of buffer
        while (isspace(ss->recvbuf[len - 1])) ss->recvbuf[--len] = 0;

        dlog("%x: Command is %s\n", fd, ss->recvbuf);
        if (strlen(ss->recvbuf) == 0) {
            send_formatted(fd, "-ERR Syntax error, blank command unrecognized\r\n");
            break;
        }
        // Split the command into its component "words"
        ss->nwords = split(ss->recvbuf, ss->words);
        char *command = ss->words[0];

        /* TODO: Handle the different values of `command` and dispatch it to the correct implementation
         *  TOP, UIDL, APOP commands do not need to be implemented and therefore may return an error response */
	int res;

	if (strcmp(command, "USER") == 0) {
	    res = do_user(ss);
	} else if (strcmp(command, "PASS") == 0) {
	    res = do_pass(ss);
	} else if (strcmp(command, "STAT") == 0) {
	    res = do_stat(ss);
	} else if (strcmp(command, "LIST") == 0) {
	    res = do_list(ss);
	} else if (strcmp(command, "RETR") == 0) {
	    res = do_retr(ss);
	} else if (strcmp(command, "DELE") == 0) {
	    res = do_dele(ss);
	} else if (strcmp(command, "RSET") == 0) {
	    res = do_rset(ss);
	} else if (strcmp(command, "NOOP") == 0) {
	    res = do_noop(ss);
	} else if (strcmp(command, "QUIT") == 0) {
	    res = do_quit(ss);
	} else {
	    // Unsupported command
	    res = send_formatted(fd, "-ERR Invalid command\r\n");
	}

	// Server should exit
	if (res == -1) break;
    }
    // TODO: Clean up fields in `serverstate`, if required
    nb_destroy(ss->nb);
    close(fd);
    free(new_fd);
}
