#include "common.h"

#include <fcntl.h>

int child_write(conn_id, fd)
int conn_id;
int fd;
{
    int bytes_sent = 0;

    if(debug)
    {
        fprintf(stderr,"Connection %d: writing data to child stdin\n",conn_id);
    }

    if(conn_tbl[conn_id].child_stdin_pos < conn_tbl[conn_id].child_stdin.len)
    {
        bytes_sent = fd_write(
          conn_tbl[conn_id].child_stdin_fd,
          conn_tbl[conn_id].child_stdin.s + conn_tbl[conn_id].child_stdin_pos,
          conn_tbl[conn_id].child_stdin.len - conn_tbl[conn_id].child_stdin_pos);
        if(bytes_sent > 0)
        {
             conn_tbl[conn_id].child_stdin_pos += bytes_sent;
        }
        if(bytes_sent < 0 )
        {
            fprintf(stderr,"WARNING: Connection %d: writing data to child failed (%s)\n",conn_id,strerror(errno));
            return 0;
        }
    }

    if (conn_tbl[conn_id].child_stdin_pos == conn_tbl[conn_id].child_stdin.len)
    {
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


