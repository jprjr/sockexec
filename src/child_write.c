#include "common.h"

#include <fcntl.h>

int child_write(conn_id, fd, except)
int conn_id;
int fd;
int except;
{
    int bytes_sent = 0;
    int bytes_to_send = conn_tbl[conn_id].child_stdin.len - conn_tbl[conn_id].child_stdin_pos;
    if(except)
    {
        conn_tbl[conn_id].child_stdin_done = 1;
        goto child_write_close;
    }

    if(bytes_to_send > 0)
    {

        if(debug)
        {
            fprintf(stderr,"Connection %d: writing %d bytes of data to child stdin\n", conn_id, bytes_to_send);
        }
        bytes_sent = fd_write(
          conn_tbl[conn_id].child_stdin_fd,
          conn_tbl[conn_id].child_stdin.s + conn_tbl[conn_id].child_stdin_pos,
          conn_tbl[conn_id].child_stdin.len - conn_tbl[conn_id].child_stdin_pos);
        if(debug)
        {
            fprintf(stderr,"Connection %d: wrote %d bytes of data to child stdin\n",conn_id, bytes_sent);
        }
        if(bytes_sent > 0)
        {
            conn_tbl[conn_id].child_stdin_pos += bytes_sent;
        }
        if(bytes_sent < 0 )
        {
            conn_tbl[conn_id].child_stdin_done = 1;
            goto child_write_close;
        }
    }

    if (conn_tbl[conn_id].child_stdin_pos == conn_tbl[conn_id].child_stdin.len)
    {
        child_write_close:
        /* take stdin out of the pollfd loop */
        fds_tbl[conn_tbl[conn_id].child_stdin_fd].fd = -1;

        if(conn_tbl[conn_id].child_stdin_done == 1)
        {
            if(debug)
            {
                fprintf(stderr,"Connection %d: closing child stdin (fd %d)\n",conn_id,fd);
            }
            stralloc_free(&(conn_tbl[conn_id].child_stdin));
            conn_tbl[conn_id].child_stdin_pos = -1;

            int child_stdin_fd = conn_tbl[conn_id].child_stdin_fd;

            fd_tbl[child_stdin_fd] = -1;
            conn_tbl[conn_id].child_stdin_fd = -1;
            fd_close(fd);
        }
    }
    return 1;
}


