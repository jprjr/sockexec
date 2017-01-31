#include "common.h"

#include <fcntl.h>

int child_write(conn_id, fd)
int conn_id;
int fd;
{
    int bytes_sent = 0;

    LOLDEBUG("entering child_write");
    LOLDEBUG("child_write: stdin_pos: %d, stdin_len: %d",
      conn_tbl[conn_id].child_stdin_pos,
      conn_tbl[conn_id].child_stdin.len);

    if(conn_tbl[conn_id].child_stdin_pos < conn_tbl[conn_id].child_stdin.len)
    {
        LOLDEBUG("child_write: writing data to child stdin");
        bytes_sent = fd_write(
          conn_tbl[conn_id].child_stdin_fd,
          conn_tbl[conn_id].child_stdin.s + conn_tbl[conn_id].child_stdin_pos,
          conn_tbl[conn_id].child_stdin.len - conn_tbl[conn_id].child_stdin_pos);
        LOLDEBUG("child_write: wrote %d bytes",bytes_sent);
        if(bytes_sent > 0)
        {
             conn_tbl[conn_id].child_stdin_pos += bytes_sent;
        }
        if(bytes_sent < 0 )
        {
            LOLDEBUG("child_write: error %d (%s)",errno,strerror(errno));
            return 0;
        }
    }

    if (conn_tbl[conn_id].child_stdin_pos == conn_tbl[conn_id].child_stdin.len)
    {
        /* take stdin out of the pollfd loop */
        fds_tbl[conn_tbl[conn_id].child_stdin_fd].fd = -1;

        if(conn_tbl[conn_id].child_stdin_done == 1)
        {
            LOLDEBUG("child_write: closing stdin");
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


