#include "common.h"
#include "common.h"

int client_read(conn_id,except)
int conn_id;
int except;
{
    char buffer[BUF_SIZE] = { 0 };
    int bytes_read = 0;

    if(debug)
    {
        fprintf(stderr,"Connection %d: reading client data\n",conn_id);
    }
    if(except)
    {
        goto client_read_close;
    }

    while( (bytes_read = ipc_recv(conn_tbl[conn_id].client,buffer,BUF_SIZE,0)) > 0 )
    {
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
            goto client_read_close;
        }
    }

    if(bytes_read == 0)
    {
        client_read_close:
        if(debug) {
            fprintf(stderr,"Connection %d: client sent 0 bytes, closing connection\n",conn_id);
        }
        close_connection(conn_id,1,0);
        return 1;
    }
    return 1;
}

