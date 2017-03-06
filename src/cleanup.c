#include "common.h"

void cleanup(void)
{
    unsigned int i=0;

    /* close file descriptors, quit */
    while(i<conn_tbl_len)
    {
        close_connection(i,1);
        i++;
    }
    unlink(sockname);

    genalloc_free(iopause_fd,&_fds_tbl);
    genalloc_free(int,&_fd_tbl);
    genalloc_free(connection_t,&_conn_tbl);

    exit(0);
}

