#include "common.h"

int process_signals(void)
{
    int code;
    code = selfpipe_read();
    LOLDEBUG("got code %d (%s)",code,sig_name(code));
    switch (code) {
        case SIGTERM:
        case SIGINT: cleanup();
        case SIGCHLD:
        {
            unsigned int i = 0;
            int status = 0;
            pid_t child;
            child = wait_nohang(&status);
            while(child > 0)
            {
                while(conn_tbl[i].child_pid != child && i<conn_tbl_len)
                {
                    i++;
                }
                if(i == conn_tbl_len)
                {
                    printf("Crap: can't find conn_id for pid %d\n",child);
                    return 0;
                }
                conn_tbl[i].child_pid = 0;
                LOLDEBUG("WIFEXITED(status): %d",WIFEXITED(status));
                LOLDEBUG("WEXITSTATUS(status): %d",WEXITSTATUS(status));
                LOLDEBUG("WIFSIGNALED(status): %d",WIFSIGNALED(status));
                LOLDEBUG("WTERMSIG(status): %d",WTERMSIG(status));
                if(WIFEXITED(status))
                {
                    conn_tbl[i].child_exit_code = WEXITSTATUS(status);
                }
                else if(WIFSIGNALED(status))
                {
                    conn_tbl[i].child_exit_signal = WTERMSIG(status);
                }
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

