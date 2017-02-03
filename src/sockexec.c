#include "common.h"

#define MAX_QUEUE 10
#define USAGE "sockexec [-m max_connections] [-t timeout] /path/to/socket"
#define dieusage() strerr_dieusage(100, USAGE)
#define dienomem()    strerr_diefu1sys(111, "stralloc_catb")
#define dienosocket() strerr_diefu1sys(111, "ipc_stream_nb")
#define dienobind()   strerr_diefu1sys(111, "ipc_bind")
#define dienolisten() strerr_diefu1sys(111, "ipc_listen")

int main(int argc, char const *const *argv) {
    PROG="sockexec";

    timeout = 60;
    int events = 0;
    unsigned int i = 0;
    int opt = 0;
    unsigned int max_conns = 100;
    unsigned int max_fds = 0;
    static const int n1 = -1;
    tain_t _now = tain_zero;
    tain_t _deadline = tain_zero;
    deadline = 0;

    now = &_now;

    LOLDEBUG("main: start");

    subgetopt_t l = SUBGETOPT_ZERO ;

    PROG = "sockexec" ;

    while( (opt = subgetopt_r(argc,argv,"m:t:",&l)) > 0 )
    {
        switch(opt)
        {
            case 'm':
            {
                unsigned int m;
                if(!uint0_scan(l.arg,&m)) dieusage() ;
                max_conns = m;
                break;
            }
            case 't':
            {
                unsigned int t;
                if(!uint0_scan(l.arg,&t)) dieusage() ;
                if(t == 0) dieusage() ;
                timeout = t;
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

    LOLDEBUG("main: timeout is %u",timeout);

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
    if(ipc_bind(fds_tbl[1].fd,sockname) < 0) dienobind() ;
    if(ipc_listen(fds_tbl[1].fd,MAX_QUEUE) < 0) dienolisten() ;
    /* end listener */

    while(1)
    {
        tain_now(now);
        if(deadline == 0)
        {
            tain_addsec(&_deadline,now,timeout);
            deadline = &_deadline;
        }
        LOLDEBUG("main: entering iopause_stamp");
#ifdef DEBUG
        for(i=0;i<fds_tbl_len;i++) {
            dump_fds(i);
        }
#endif
        events = iopause_stamp(fds_tbl,max_fds,deadline,now);
        LOLDEBUG("main: return from iopause_stamp");
        if(events == 0)
        {
            LOLDEBUG("main: deadline reached");
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

