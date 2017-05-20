#include "common.h"

int route_event(fd)
int fd;
{
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

    if(fds_tbl[fd].revents & IOPAUSE_READ)
    {
        read = 1;
    }
    if(fds_tbl[fd].revents & IOPAUSE_WRITE)
    {
        write = 1;
    }

    if(read == 0 && write == 0)
    {
        fprintf(stderr,"WARNING: IOPAUSE_EXCEPT, closing fd %d\n",fd);
        fd_close(fd);
        fds_tbl[fd].fd = -1;
        return 0;
    }

    if(fd == client)
    {
        if(read)  { return client_read(conn_id); }
        if(write) { return client_write(conn_id); }
    }
    if(read)  { return child_read(conn_id,fd); }
    if(write) { return child_write(conn_id,fd); }
    return 1;
}

