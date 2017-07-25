#include "common.h"

#include <fcntl.h>

int child_read(conn_id, fd, except)
int conn_id;
int fd;
int except;
{
    char buffer[BUF_SIZE] = { 0 };
    int bytes_read = 0;
    int is_stdout = 0;

    if(debug)
    {
        fprintf(stderr,"Connection %d: reading process data (fd %d)\n",conn_id,fd);
    }
    errno = 0;

    if(conn_tbl[conn_id].child_stdout_fd > 0 && conn_tbl[conn_id].child_stdout_fd == fd)
    {
        is_stdout = 1;
    }

    if(except)
    {
        goto child_read_close;
    }


    bytes_read = fd_read(fd,buffer,BUF_SIZE);

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
            goto child_read_close;
        }
    }
    if(bytes_read == 0)
    {
        child_read_close:
        if(debug)
        {
            fprintf(stderr,"Connection %d: closing child %s fd (%d)\n",conn_id,is_stdout ? "stdout":"stderr", fd);
        }
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


