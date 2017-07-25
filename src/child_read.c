#include "common.h"

#include <fcntl.h>

int child_read(conn_id, fd, except)
int conn_id;
int fd;
int except;
{
    char buffer[BUF_SIZE] = { 0 };
    int bytes_read = 0;
    enum CHILD_OUTPUT_TYPE output_type = CHILD_UNKNOWN;

    if(debug)
    {
        fprintf(stderr,"Connection %d: reading process data (fd %d)\n",conn_id,fd);
    }
    errno = 0;

    if(conn_tbl[conn_id].child_stdout_fd == fd)
    {
        output_type = CHILD_STDOUT;
    }
    else if(conn_tbl[conn_id].child_stderr_fd == fd)
    {
        output_type = CHILD_STDERR;
    }

    if(output_type == CHILD_UNKNOWN)
    {
        fprintf(stderr,"WARNING: Connection %d: unable to determine fd type (fd %d)\n",conn_id,fd);
    }

    if(except)
    {
        goto child_read_close;
    }


    bytes_read = fd_read(fd,buffer,BUF_SIZE);

    if(bytes_read > 0)
    {
        if(debug)
        {
            fprintf(stderr,"Connection %d: read %d bytes from %s (fd %d)\n",conn_id,bytes_read,CHILD_OUTPUT_TYPES[output_type],fd);
        }
        stralloc_catb(&(conn_tbl[conn_id].child_outputs[output_type]),buffer,bytes_read);
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
            fprintf(stderr,"Connection %d: closing process %s (fd %d)\n",conn_id,CHILD_OUTPUT_TYPES[output_type], fd);
        }
        fd_close(fd);
        fds_tbl[fd].fd = -1;
        fd_tbl[fd] = -1;
        if(output_type == CHILD_STDOUT)
        {
            conn_tbl[conn_id].child_stdout_fd = -1;
        }
        else if(output_type == CHILD_STDERR)
        {
            conn_tbl[conn_id].child_stderr_fd = -1;
        }
    }
    update_client(conn_id);
    return 1;
}


