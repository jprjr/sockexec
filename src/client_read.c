#include "common.h"
#include "common.h"

int client_read(conn_id)
int conn_id;
{
    char buffer[BUF_SIZE] = { 0 };
    int bytes_read = 0;

    LOLDEBUG("entering client_read");

    while( (bytes_read = ipc_recv(conn_tbl[conn_id].client,buffer,BUF_SIZE,0)) > 0 )
    {
        LOLDEBUG("client_read: read %d bytes",bytes_read);
        stralloc_catb(&(conn_tbl[conn_id].client_in_buffer),buffer,bytes_read);
    }

    if(bytes_read < 0)
    {
        if(errno == EAGAIN || errno == EWOULDBLOCK)
        {
            /* no big deal */
            update_child(conn_id);
            return 1;
        }
        else
        {
            printf("Error: %s\n",strerror(errno));
            return 0;
        }
    }

    if(bytes_read == 0)
    {
        LOLDEBUG("client_read: closing connection");
        close_connection(conn_id,1);
        return 1;
    }
    return 1;
}

