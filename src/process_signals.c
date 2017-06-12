#include "common.h"

int process_signals(void)
{
    int code;
    code = selfpipe_read();
    if(debug) {
        fprintf(stderr,"Caught signal: %s\n",sig_name(code));
    }
    switch (code) {
        case SIGTERM:
        case SIGINT:
        {
            cleanup();
            break;
        }
        case SIGCHLD:
        {
            unsigned int i = 0;
            int status = 0;
            pid_t child;
            child = wait_nohang(&status);
            while(child > 0)
            {
                i = 0;
                while(conn_tbl[i].child_pid != child && i<conn_tbl_len)
                {
                    i++;
                }
                if(i == conn_tbl_len)
                {
                    fprintf(stderr,"WARNING: unable to find connection for pid %d\n",child);
                    return 0;
                }
                if(debug)
                {
                    fprintf(stderr,"Connection %d: child process finished\n",i);
                }
                conn_tbl[i].child_pid = 0;

                if(WIFEXITED(status))
                {
                    conn_tbl[i].child_exit_code = WEXITSTATUS(status);
                }
                else if(WIFSIGNALED(status))
                {
                    conn_tbl[i].child_exit_signal = WTERMSIG(status);
                }
                conn_tbl[i].deadline = tain_zero;
                update_client(i);
                child = wait_nohang(&status);
            }
            break;
        }
        case SIGUSR1:
        {
            unsigned int i;
            for(i=0; i<conn_tbl_len; i++)
            {
                dump_connection(i,0);
            }
            break;
        }
        case SIGUSR2:
        {
            unsigned int i;
            for(i=0; i<fds_tbl_len; i++)
            {
                dump_fds(i);
            }
            break;
        }
        /* TODO do I need to explicitly handle SIGPIPE? */
    }
    return 0;
}

