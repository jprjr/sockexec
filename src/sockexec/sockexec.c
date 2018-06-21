#include "common.h"

#define USAGE "sockexec [-v] [-d] [-q queue_size] [-M mode] [-m max_connections] [-t timeout] [-k kill_timeout] /path/to/socket"
#define VERSION "3.1.0"
#define dieusage() strerr_dieusage(100, USAGE)
#define dienomem()    strerr_diefu1sys(111, "stralloc_catb")
#define dienosocket() strerr_diefu1sys(111, "ipc_stream_nb")
#define dienobind()   strerr_diefu1sys(111, "ipc_bind")
#define dienolisten() strerr_diefu1sys(111, "ipc_listen")

const char* const CHILD_OUTPUT_TYPES[] =
{
  "unknown",
  "stdout",
  "stderr",
  0
};

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

static const iopause_fd iopause_zero = {
    .fd = -1,
    .events = IOPAUSE_READ,
    .revents = 0
};

int main(int argc, char const *const *argv) {

    timeout = 60;
    kill_timeout = 10;
    int events = 0;
    unsigned int i = 0;
    int opt = 0;
    unsigned int max_conns = 100;
    unsigned int max_fds = 0;
    unsigned int max_queue = 100;
    static const int n1 = -1;
    tain_t _now = tain_zero;
    tain_t _deadline = tain_zero;
    unsigned int perms = 0660;
    mode_t m;
    deadline = 0;
    debug = 0;

    now = &_now;

    subgetopt_t l = SUBGETOPT_ZERO ;

    PROG = "sockexec" ;

    while( (opt = subgetopt_r(argc,argv,"vdk:m:t:q:M:",&l)) > 0 )
    {
        switch(opt)
        {
            case 'v':
            {
                printf("%s version: %s\n",PROG,VERSION);
                return 0;
            }
            case 'd':
            {
                debug = 1;
                break ;
            }
            case 'q':
            {
                unsigned int q;
                if(!uint0_scan(l.arg,&q)) dieusage() ;
                if(q == 0) dieusage();
                max_queue = q;
                break;
            }
            case 'm':
            {
                unsigned int m;
                if(!uint0_scan(l.arg,&m)) dieusage() ;
                max_conns = m;
                break;
            }
            case 'k':
            {
                unsigned int k;
                if(!uint0_scan(l.arg,&k)) dieusage() ;
                if(k == 0) dieusage() ;
                kill_timeout = k;
                break;
            }
            case 't':
            {
                unsigned int t;
                if(!uint0_scan(l.arg,&t)) dieusage() ;
                timeout = t;
                break;
            }
            case 'M':
            {
                if(!uint0_oscan(l.arg,&perms)) dieusage() ;
                break;
            }
            default: dieusage() ;
        }
    }

    argc -= l.ind ; argv += l.ind ;
    if(argc == 0) dieusage() ;

    max_fds = 5 + (4 * max_conns);

    /* setup connection_t/iopause_fd tables */
    if(fds_tbl_ready(max_fds) <= 0)    strerr_diefu2sys(111,"genalloc_ready","fds_tbl");
    if(fd_tbl_ready(max_fds) <= 0)     strerr_diefu2sys(111,"genalloc_ready","fd_tbl");
    if(conn_tbl_ready(max_conns) <= 0) strerr_diefu2sys(111,"genalloc_ready","conn_tbl");
    if(tain_init() <= 0)               strerr_diefu1sys(111,"tain_init");

    for(i=0; i < max_fds; i++)
    {
        fds_tbl_append(&iopause_zero);
    }
    for(i=0; i < max_fds; i++)
    {
        fd_tbl_append(&n1);
    }
    for(i=0; i <= max_conns; i++)
    {
        conn_tbl_append(&connection_t_zero);
    }

    sockname = argv[0];

    /* setup the self-pipe */
    fds_tbl[0].fd = selfpipe_init();
    if(fds_tbl[0].fd < 0) strerr_diefu1sys(111, "selfpipe_init") ;

    if(selfpipe_trap(SIGTERM) < 0) strerr_diefu1sys(111, "trap SIGTERM");
    if(selfpipe_trap(SIGINT) < 0) strerr_diefu1sys(111, "trap SIGINT");
    if(selfpipe_trap(SIGCHLD) < 0) strerr_diefu1sys(111, "trap SIGCHLD");
    if(selfpipe_trap(SIGUSR1) < 0) strerr_diefu1sys(111, "trap SIGUSR1");
    if(selfpipe_trap(SIGUSR2) < 0) strerr_diefu1sys(111, "trap SIGUSR2");
    if(selfpipe_trap(SIGPIPE) < 0) strerr_diefu1sys(111, "trap SIGPIPE");

    /* start listener */
    fds_tbl[1].fd = ipc_stream_nb();
    if(fds_tbl[1].fd < 0) dienosocket() ;
    m = umask(~perms & 0777);
    if(ipc_bind(fds_tbl[1].fd,sockname) < 0) dienobind() ;
    umask(m);
    if(ipc_listen(fds_tbl[1].fd,max_queue) < 0) dienolisten() ;
    /* end listener */

    while(1)
    {
        tain_now(now);
        if(deadline == 0)
        {
            if(timeout == 0 || kill_timeout < timeout) {
                tain_addsec(&_deadline,now,kill_timeout);
            }
            else {
                tain_addsec(&_deadline,now,timeout);
            }

            deadline = &_deadline;
        }

        events = iopause_stamp(fds_tbl,max_fds,deadline,now);

        if(events < 0)
        {
            fprintf(stderr,"ERROR: %s\n",strerror(errno));
            cleanup();
        }

        if(events == 0)
        {
            deadline = 0; /* reset deadline */
            kill_processes(0); /* sends TERM/KILL signals, reset deadline */
        }

        if(events && fds_tbl[0].revents & IOPAUSE_READ)
        {
            process_signals();
        }

        if(events && fds_tbl[1].revents & IOPAUSE_READ)
        {
            accept_client(fds_tbl[1].fd);
        }

        for(i=2; events && i<max_fds; i++)
        {
            if(fds_tbl[i].revents)
            {
                route_event(i);
            }
        }
    }
}

