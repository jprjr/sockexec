#include "common.h"
#include "common.h"

int client_read(conn_id)
int conn_id;
{
    char buffer[BUF_SIZE] = { 0 };
    int bytes_read = 0;

    if(debug)
    {
        fprintf(stderr,"Connection %d: reading client data\n",conn_id);
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
            fprintf(stderr,"ERROR: Connection %d: unable to read from client (%s)\n",conn_id,strerror(errno));
            return 0;
        }
    }

    if(bytes_read == 0)
    {
        if(debug) {
            fprintf(stderr,"Connection %d: client sent 0 bytes, closing connection\n",conn_id);
        }
        close_connection(conn_id,1,0);
        return 1;
    }
    return 1;
}

