#include "common.h"

#include <fcntl.h>

int child_read(conn_id, fd)
int conn_id;
int fd;
{
    char buffer[BUF_SIZE] = { 0 };
    int bytes_read = 0;
    int is_stdout = 0;

    LOLDEBUG("entering child_read, conn_id=%d, fd=%d",conn_id,fd);
    errno = 0;

    if(conn_tbl[conn_id].child_stdout_fd > 0 && conn_tbl[conn_id].child_stdout_fd == fd)
    {
        is_stdout = 1;
    }

    bytes_read = fd_read(fd,buffer,BUF_SIZE);
    LOLDEBUG("child_read: %d bytes on %s",bytes_read, is_stdout? "stdout" : "stderr");
    if(bytes_read > 0)
    {
        if(is_stdout)
        {
            stralloc_catb(&(conn_tbl[conn_id].child_stdout),buffer,bytes_read);
        }
        else
        {
            stralloc_catb(&(conn_tbl[conn_id].child_stderr),buffer,bytes_read);
        }
    }
    if(bytes_read < 0)
    {
        if(errno == EAGAIN || errno == EWOULDBLOCK)
        {
            /* nbd */
        }
        else
        {
            LOLDEBUG("Error: %s\n",strerror(errno));
            goto connclose;
        }
    }
    if(bytes_read == 0)
    {
        connclose:
        LOLDEBUG("child_read: closing %s", is_stdout ? "stdout" : "stderr");
        fd_close(fd);
        fds_tbl[fd].fd = -1;
        fd_tbl[fd] = -1;
        if(is_stdout)
        {
            conn_tbl[conn_id].child_stdout_fd = -1;
        }
        else
        {
            conn_tbl[conn_id].child_stderr_fd = -1;
        }
    }
    update_client(conn_id);
    return 1;
}


