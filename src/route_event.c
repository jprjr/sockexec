#include "common.h"

int route_event(fd)
int fd;
{
    int except = 0;
    int read = 0;
    int write = 0;
    int conn_id = fd_tbl[fd];


    if(conn_id < 0)
    {
        fprintf(stderr,"WARNING: No known connection for fd %d, closing\n",fd);
        fd_close(fd);
        return 0;
    }

    int client = conn_tbl[conn_id].client;

    if(fds_tbl[fd].revents & IOPAUSE_EXCEPT)
    {
        except = 1;
    }
    if(fds_tbl[fd].revents & IOPAUSE_READ)
    {
        read = 1;
    }
    if(fds_tbl[fd].revents & IOPAUSE_WRITE)
    {
        write = 1;
    }

    if(fd == client)
    {
        if(read == 1 || (read == 0 && write == 0))
        {
            return client_read(conn_id,except);
        }
        return client_write(conn_id,except);
    }

    if(fd == conn_tbl[conn_id].child_stdin_fd) {
        return child_write(conn_id,fd,except);
    }

    return child_read(conn_id,fd,except);
}

