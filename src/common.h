#ifndef COMMON_H
#define COMMON_H

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <skalibs/allreadwrite.h>
#include <skalibs/webipc.h>
#include <skalibs/strerr2.h>
#include <skalibs/genalloc.h>
#include <skalibs/stralloc.h>
#include <skalibs/iopause.h>
#include <skalibs/selfpipe.h>
#include <skalibs/sgetopt.h>
#include <skalibs/types.h>
#include <skalibs/netstring.h>
#include <skalibs/sig.h>
#include <skalibs/lolstdio.h>
#include <skalibs/alloc.h>

#define BUF_SIZE 4096


extern const char **environ;

enum CHILD_OUTPUT_TYPE {
    CHILD_UNKNOWN,
    CHILD_STDOUT,
    CHILD_STDERR,
    CHILD_OUTPUT_NULL,
};

static const char* const CHILD_OUTPUT_TYPES[] =
{
  "unknown",
  "stdout",
  "stderr",
  0
};


struct connection_t_ {
    int client;

    unsigned int child_argc;
    genalloc child_argv;
    stralloc child_stdin;
    unsigned int child_stdin_pos;
    int child_stdin_done;

    pid_t child_pid;
    int child_exit_code;
    int child_exit_signal;
    int child_stdin_fd;
    int child_stdout_fd;
    int child_stderr_fd;

    stralloc child_outputs[3];

    stralloc client_in_buffer;
    unsigned int client_in_buffer_pos;

    stralloc client_out_buffer;
    unsigned int client_out_buffer_pos;

    tain_t deadline;
    int sigsent;
};
typedef struct connection_t_ connection_t;

static const connection_t connection_t_zero = {
    .client = -1,
    .child_argc = 0,
    .child_argv = GENALLOC_ZERO,
    .child_stdin = STRALLOC_ZERO,
    .child_stdin_pos = 0,
    .child_stdin_done = 0,
    .child_pid = 0,
    .child_exit_code = -1,
    .child_exit_signal = -1,
    .child_stdin_fd = -1,
    .child_stdout_fd = -1,
    .child_stderr_fd = -1,
    .child_outputs = {
        STRALLOC_ZERO,
        STRALLOC_ZERO,
        STRALLOC_ZERO,
    },
    .client_in_buffer = STRALLOC_ZERO,
    .client_in_buffer_pos = 0,
    .client_out_buffer = STRALLOC_ZERO,
    .client_out_buffer_pos = 0,
    .deadline = TAIN_ZERO,
    .sigsent = 0
};

/* global variables */
genalloc _conn_tbl;
genalloc _fds_tbl;
genalloc _fd_tbl;
const char *sockname;
tain_t *deadline;
tain_t *now;
unsigned int timeout;
unsigned int kill_timeout;
char debug;

#define  fds_tbl   (genalloc_s(iopause_fd,&_fds_tbl))
#define  fd_tbl    (genalloc_s(int,&_fd_tbl))
#define  conn_tbl  (genalloc_s(connection_t,&_conn_tbl))

#define  fds_tbl_len   (genalloc_len(iopause_fd,&_fds_tbl))
#define  fd_tbl_len    (genalloc_len(int,&_fd_tbl))
#define  conn_tbl_len  (genalloc_len(connection_t,&_conn_tbl))

#define fds_tbl_ready(n)    genalloc_ready(iopause_fd,&_fds_tbl,n)
#define fd_tbl_ready(n)     genalloc_ready(int,&_fd_tbl,n)
#define conn_tbl_ready(n)   genalloc_ready(connection_t,&_conn_tbl,n)

#define fds_tbl_append(p)   genalloc_append(iopause_fd,&_fds_tbl,p)
#define fd_tbl_append(p)    genalloc_append(int,&_fd_tbl,p)
#define conn_tbl_append(p)  genalloc_append(connection_t,&_conn_tbl,p)

#define tain_lesseq(a,b)    (tain_less(a,b) && !tain_less(b,a))

static const iopause_fd iopause_zero = {
    .fd = -1,
    .events = IOPAUSE_READ,
    .revents = 0
};


#include "functions.h"

#endif
