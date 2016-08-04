#include "common.h"


void close_connection(conn_id, force)
int conn_id;
int force;
{
    LOLDEBUG("enter close_connection: %d (%s)",conn_id,force? "forced": "not forced");
    unsigned int i;
    if(conn_tbl[conn_id].child_pid > 0)
    {
        if(force == 0)
        {
            LOLDEBUG("close_connection: child_pid > 0, returning");
            return;
        }
    }
    LOLDEBUG("closing connection");

#ifdef DEBUG
    dump_connection(conn_id,0);
#endif

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
    conn_tbl[conn_id].child_stdin_ready = 0;
    conn_tbl[conn_id].client_out_buffer_pos = 0;
    conn_tbl[conn_id].client_in_buffer_pos = 0;

    /* only thing that resets a child_pid is the SIGCHLD handler */
    for(i=0; i<conn_tbl[conn_id].child_argc; i++)
    {
        alloc_free(genalloc_s(char **,&(conn_tbl[conn_id].child_argv))[i]);
    }
    conn_tbl[conn_id].child_argc = 0;
    conn_tbl[conn_id].child_exit_code = -1;
    conn_tbl[conn_id].child_exit_signal = -1;
    conn_tbl[conn_id].sigsent = 0;

    conn_tbl[conn_id].deadline = tain_zero;

    stralloc_free(&(conn_tbl[conn_id].client_in_buffer));
    stralloc_free(&(conn_tbl[conn_id].client_out_buffer));
    stralloc_free(&(conn_tbl[conn_id].child_stdin));
    stralloc_free(&(conn_tbl[conn_id].child_stdout));
    stralloc_free(&(conn_tbl[conn_id].child_stderr));
    genalloc_free(char **,&(conn_tbl[conn_id].child_argv));

}
