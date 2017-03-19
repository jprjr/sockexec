#include "common.h"
#include <fcntl.h>

int update_child(conn_id)
int conn_id;
{
    LOLDEBUG("entering update_child: conn_id %d",conn_id);

    int bytes_read = 0;

    if(conn_tbl[conn_id].child_argc == 0)
    {
        LOLDEBUG("update_child: getting argument count");
        stralloc tmp = STRALLOC_ZERO;
        bytes_read = netstring_decode(
          &tmp,
          conn_tbl[conn_id].client_in_buffer.s,
          conn_tbl[conn_id].client_in_buffer.len);

        if(bytes_read >= 0)
        {
            conn_tbl[conn_id].client_in_buffer_pos += bytes_read;
            uint_scan(tmp.s,&(conn_tbl[conn_id].child_argc));
            if(conn_tbl[conn_id].child_argc <= 0)
            {
                return -1;
            }
            LOLDEBUG("update_child: argc = %d",conn_tbl[conn_id].child_argc);
            genalloc_ready(char **,&(conn_tbl[conn_id].child_argv),conn_tbl[conn_id].child_argc + 1);
        }
        stralloc_free(&tmp);
    }

    while(
        conn_tbl[conn_id].client_in_buffer_pos < conn_tbl[conn_id].client_in_buffer.len
        &&
        conn_tbl[conn_id].child_argc > genalloc_len( char **,&(conn_tbl[conn_id].child_argv))
    )
    {
        LOLDEBUG("update_child: attempting to grab argument");
        stralloc arg = STRALLOC_ZERO;
        bytes_read = netstring_decode(
            &arg,
            conn_tbl[conn_id].client_in_buffer.s + conn_tbl[conn_id].client_in_buffer_pos,
            conn_tbl[conn_id].client_in_buffer.len);
        if(bytes_read > 0)
        {
            stralloc_append(&arg,0);
            genalloc_append(char **,&(conn_tbl[conn_id].child_argv),&(arg.s));
            conn_tbl[conn_id].client_in_buffer_pos += bytes_read;
        }
        else
        {
            break;
        }
    }

#ifdef DEBUG
{
    unsigned int i;
    for(i=0; i<genalloc_len(char**, &(conn_tbl[conn_id].child_argv)); i++) {
        LOLDEBUG("argv[%d]: %s",i,genalloc_s(char **,&(conn_tbl[conn_id].child_argv))[i]);
    }
}
#endif

    if(conn_tbl[conn_id].child_argc > 0 && genalloc_len(char **,&(conn_tbl[conn_id].child_argv)) == conn_tbl[conn_id].child_argc) {
        /* done reading args, try reading stdin */
        LOLDEBUG("update_child: attempting to grab stdin");
        LOLDEBUG("update_child: cur stdin pos: %d",conn_tbl[conn_id].child_stdin_pos);
        while(conn_tbl[conn_id].client_in_buffer_pos < conn_tbl[conn_id].client_in_buffer.len) {
            stralloc tmp = STRALLOC_ZERO;
            bytes_read = netstring_decode(
              &tmp,
              conn_tbl[conn_id].client_in_buffer.s + conn_tbl[conn_id].client_in_buffer_pos,
              conn_tbl[conn_id].client_in_buffer.len);
            if(bytes_read > 0) {
                conn_tbl[conn_id].client_in_buffer_pos += bytes_read;
                LOLDEBUG("update_child: grabbed stdin, length %d",tmp.len);
                if(tmp.len > 0) {
                    stralloc_cat(&(conn_tbl[conn_id].child_stdin), &tmp);
                }
                else {
                    /* 0-byte netstring */
                    conn_tbl[conn_id].child_stdin_done = 1;
                }
                stralloc_free(&tmp);
                if(conn_tbl[conn_id].child_pid > 0 && fds_tbl[conn_tbl[conn_id].child_stdin_fd].fd == -1) {
                    fds_tbl[conn_tbl[conn_id].child_stdin_fd].fd = conn_tbl[conn_id].child_stdin_fd;
                    fds_tbl[conn_tbl[conn_id].child_stdin_fd].events = IOPAUSE_WRITE;
                }
            }
            else
            {
                stralloc_free(&tmp);
                break;
            }
        }
    }

    if(conn_tbl[conn_id].client_in_buffer_pos == conn_tbl[conn_id].client_in_buffer.len)
    {
        stralloc_free(&(conn_tbl[conn_id].client_in_buffer));
        conn_tbl[conn_id].client_in_buffer_pos = 0;
    }
    LOLDEBUG("update_child: child_pid: %d",conn_tbl[conn_id].child_pid);

    if(conn_tbl[conn_id].child_pid == 0 && genalloc_len(char **,&(conn_tbl[conn_id].child_argv)) == conn_tbl[conn_id].child_argc)
    {
        LOLDEBUG("update_child: spawning process");

        /* char **argv;[conn_tbl[conn_id].child_argc + 1]; */
        genalloc_s(char **,&(conn_tbl[conn_id].child_argv))[conn_tbl[conn_id].child_argc] = 0;

        conn_tbl[conn_id].child_pid = child_spawn3(
                (const char *)genalloc_s(char **, &(conn_tbl[conn_id].child_argv))[0],
                (const char * const*)genalloc_s(char **, &(conn_tbl[conn_id].child_argv)),
                environ,
                &(conn_tbl[conn_id].child_stdin_fd),
                &(conn_tbl[conn_id].child_stdout_fd),
                &(conn_tbl[conn_id].child_stderr_fd));

        LOLDEBUG("update_child: pid %d",conn_tbl[conn_id].child_pid);
        LOLDEBUG("update_child: stdin_fd %d",conn_tbl[conn_id].child_stdin_fd);
        LOLDEBUG("update_child: stdout_fd %d",conn_tbl[conn_id].child_stdout_fd);
        LOLDEBUG("update_child: stderr_fd %d",conn_tbl[conn_id].child_stderr_fd);

        if(conn_tbl[conn_id].child_pid <= 0)
        {
            close_connection(conn_id,1,0);
            return 0;
        }

        if(conn_tbl[conn_id].child_stdin_pos < conn_tbl[conn_id].child_stdin.len) {
            fds_tbl[conn_tbl[conn_id].child_stdin_fd].fd = conn_tbl[conn_id].child_stdin_fd;
        }
        else {
            fds_tbl[conn_tbl[conn_id].child_stdin_fd].fd = -1;
        }

        fd_tbl[conn_tbl[conn_id].child_stdin_fd] = conn_id;
        fd_tbl[conn_tbl[conn_id].child_stdout_fd] = conn_id;
        fd_tbl[conn_tbl[conn_id].child_stderr_fd] = conn_id;

        fds_tbl[conn_tbl[conn_id].child_stdout_fd].fd = conn_tbl[conn_id].child_stdout_fd;
        fds_tbl[conn_tbl[conn_id].child_stderr_fd].fd = conn_tbl[conn_id].child_stderr_fd;

        fds_tbl[conn_tbl[conn_id].child_stdout_fd].events = IOPAUSE_READ;
        fds_tbl[conn_tbl[conn_id].child_stderr_fd].events = IOPAUSE_READ;
        fds_tbl[conn_tbl[conn_id].child_stdin_fd].events = IOPAUSE_WRITE;

        fds_tbl[conn_tbl[conn_id].child_stdin_fd].revents = 0;
        fds_tbl[conn_tbl[conn_id].child_stdout_fd].revents = 0;
        fds_tbl[conn_tbl[conn_id].child_stderr_fd].revents = 0;
    }


    return 1;
}

