#include "common.h"

int accept_client(fd)
int fd;
{
    unsigned int i;
    tain_t c_now;

    for(i=0; i <= conn_tbl_len && (conn_tbl[i].client > -1 || conn_tbl[i].child_pid > 0); i++)
    {

    }

    if(i > conn_tbl_len)
    {
        fprintf(stderr,"WARNING: Unable to accept client - no available connections\n");
        int tmp_fd = ipc_accept_nb(fd,0,0,0);
        if(tmp_fd > 0) {
            fd_close(tmp_fd);
        }
        return 0;
    }

    if(debug)
    {
        fprintf(stderr,"Connection %d: accepted\n",i);
    }


    conn_tbl[i].client = ipc_accept_nb(fd,0,0,0);

    if(conn_tbl[i].client < 0) strerr_warnw1sys("ipc_accept") ;

    if(timeout > 0) {
      tain_now(&c_now);
      tain_addsec(&(conn_tbl[i].deadline),&c_now,timeout);
    }

    /* update global deadline */
    if(deadline == 0 || (timeout > 0 && tain_less(&(conn_tbl[i].deadline),deadline)))
    {
        deadline = &(conn_tbl[i].deadline);
    }

    fds_tbl[conn_tbl[i].client].fd      = conn_tbl[i].client;
    fds_tbl[conn_tbl[i].client].events  = IOPAUSE_READ;
    fds_tbl[conn_tbl[i].client].revents = 0;

    fd_tbl[conn_tbl[i].client] = i;

    return 1;
}

