#include "common.h"


/* returns 1 if connection was closed, 0 otherwise */
int close_connection(conn_id, force, quitting)
int conn_id;
int force;
int quitting;
{
    if(debug)
    {
        fprintf(stderr,"Connection %d: attempting to close, %s\n",conn_id, force ? "forced": "not forced");
    }
    unsigned int i;
    tain_t c_now;

    if( (conn_tbl[conn_id].child_stdout_fd > 0 || conn_tbl[conn_id].child_stderr_fd > 0) && force == 0)
    {
        if(debug)
        {
            fprintf(stderr,"Connection %d: not closing, child outputs are still open\n",conn_id);
        }
        return 0;
    }

    if(conn_tbl[conn_id].child_pid > 0)
    {
        if(conn_tbl[conn_id].sigsent == 0 && force == 0)
        {
            if(debug)
            {
                fprintf(stderr,"Connection %d: not closing, process still running\n",conn_id);
            }
            return 0;
        }

        if(conn_tbl[conn_id].sigsent == 1 || quitting)
        {
            if(debug)
            {
                fprintf(stderr,"Connection %d: sending SIGKILL\n",conn_id);
            }
            kill(conn_tbl[conn_id].child_pid, SIGKILL);
            conn_tbl[conn_id].sigsent = 2;
            return 0;
        }

        if(conn_tbl[conn_id].sigsent == 0)
        {
            tain_now(&c_now);
            tain_addsec(&(conn_tbl[conn_id].deadline),&c_now,kill_timeout);

            if(deadline == 0 || tain_less(&(conn_tbl[conn_id].deadline),deadline))
            {
                deadline = &(conn_tbl[conn_id].deadline);
            }
            if(debug)
            {
                fprintf(stderr,"Connection %d: sending SIGTERM\n",conn_id);
            }
            kill(conn_tbl[conn_id].child_pid, SIGTERM);
            conn_tbl[conn_id].sigsent = 1;
            return 0;
        }
    }

    if( (conn_tbl[conn_id].client_out_buffer_pos != conn_tbl[conn_id].client_out_buffer.len) && force == 0)
    {
        if(debug)
        {
            fprintf(stderr,"Connection %d: not closing, buffered data exists\n",conn_id);
        }
        /* make sure client is set to write */
        fds_tbl[conn_tbl[conn_id].client].fd = conn_tbl[conn_id].client;
        fds_tbl[conn_tbl[conn_id].client].events = IOPAUSE_WRITE;
        return 0;
    }

    if(debug)
    {
        fprintf(stderr,"Connection %d: closing\n",conn_id);
    }

    if(conn_tbl[conn_id].client > -1)
    {
        fd_close(conn_tbl[conn_id].client);
        fds_tbl[conn_tbl[conn_id].client].fd = -1;
        fd_tbl[conn_tbl[conn_id].client] = -1;
        conn_tbl[conn_id].client = -1;
    }

    if(conn_tbl[conn_id].child_stdin_fd > -1)
    {
        fd_close(conn_tbl[conn_id].child_stdin_fd);
        fds_tbl[conn_tbl[conn_id].child_stdin_fd].fd = -1;
        fd_tbl[conn_tbl[conn_id].child_stdin_fd] = -1;
        conn_tbl[conn_id].child_stdin_fd = -1;
    }

    if(conn_tbl[conn_id].child_stdout_fd > -1)
    {
        fd_close(conn_tbl[conn_id].child_stdout_fd);
        fds_tbl[conn_tbl[conn_id].child_stdout_fd].fd = -1;
        fd_tbl[conn_tbl[conn_id].child_stdout_fd] = -1;
        conn_tbl[conn_id].child_stdout_fd = -1;
    }

    if(conn_tbl[conn_id].child_stderr_fd > -1)
    {
        fd_close(conn_tbl[conn_id].child_stderr_fd);
        fds_tbl[conn_tbl[conn_id].child_stderr_fd].fd = -1;
        fd_tbl[conn_tbl[conn_id].child_stderr_fd] = -1;
        conn_tbl[conn_id].child_stderr_fd = -1;
    }

    conn_tbl[conn_id].child_stdin_pos = 0;
    conn_tbl[conn_id].child_stdin_done = 0;
    conn_tbl[conn_id].client_out_buffer_pos = 0;
    conn_tbl[conn_id].client_in_buffer_pos = 0;

    /* only thing that resets a child_pid is the SIGCHLD handler */
    for(i=0; i<genalloc_len(char **,&(conn_tbl[conn_id].child_argv)); i++)
    {
        alloc_free(genalloc_s(char **,&(conn_tbl[conn_id].child_argv))[i]);
    }
    conn_tbl[conn_id].child_argc = 0;
    conn_tbl[conn_id].child_exit_code = -1;
    conn_tbl[conn_id].child_exit_signal = -1;

    stralloc_free(&(conn_tbl[conn_id].client_in_buffer));
    stralloc_free(&(conn_tbl[conn_id].client_out_buffer));
    stralloc_free(&(conn_tbl[conn_id].child_stdin));
    stralloc_free(&(conn_tbl[conn_id].child_stdout));
    stralloc_free(&(conn_tbl[conn_id].child_stderr));
    genalloc_free(char **,&(conn_tbl[conn_id].child_argv));
    return 1;

}

