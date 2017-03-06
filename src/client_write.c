#include "common.h"

int client_write(conn_id)
int conn_id;
{
    int bytes_sent = 0;
    int bytes_to_send;
    char *outgoing_buffer;

    LOLDEBUG("entering client_write, conn_id %d",conn_id);

    LOLDEBUG("client_write: buffer len: %d",conn_tbl[conn_id].client_out_buffer.len);
    if(conn_tbl[conn_id].client_out_buffer.len <= 0)
    {
        /* make sure everything is reading data */
        fds_tbl[conn_tbl[conn_id].client].events = IOPAUSE_READ;
        fds_tbl[conn_tbl[conn_id].child_stdout_fd].events = IOPAUSE_READ;
        fds_tbl[conn_tbl[conn_id].child_stderr_fd].events = IOPAUSE_READ;
        return 1;
    }

    outgoing_buffer = conn_tbl[conn_id].client_out_buffer.s + conn_tbl[conn_id].client_out_buffer_pos;
    bytes_to_send = conn_tbl[conn_id].client_out_buffer.len - conn_tbl[conn_id].client_out_buffer_pos;
    LOLDEBUG("client_write: sending %d bytes", bytes_to_send);

    bytes_sent = fd_send(conn_tbl[conn_id].client,outgoing_buffer,bytes_to_send,0);
    LOLDEBUG("client_write: sent %d bytes", bytes_sent);

    if(bytes_sent < 0)
    {
        return 0;
    }
    conn_tbl[conn_id].client_out_buffer_pos += bytes_sent;

    if(conn_tbl[conn_id].client_out_buffer_pos == conn_tbl[conn_id].client_out_buffer.len)
    {
        /* done sending data */
        if(!close_connection(conn_id,0)) {
            /* connection didn't close (child pid still running) */
            fds_tbl[conn_tbl[conn_id].client].events = IOPAUSE_READ;
            fds_tbl[conn_tbl[conn_id].child_stdout_fd].events = IOPAUSE_READ;
            fds_tbl[conn_tbl[conn_id].child_stderr_fd].events = IOPAUSE_READ;
        }
        return 1;
    }
    return 1;
}

