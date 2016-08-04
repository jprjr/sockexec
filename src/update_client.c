#include "common.h"

int update_client(conn_id)
int conn_id;
{
    LOLDEBUG("inside update_client");
    int things_to_do = 0;

    if(conn_tbl[conn_id].child_stdout.len)
    {
        if(conn_tbl[conn_id].client > -1)
        {
            LOLDEBUG("update_client: adding stdout to output buffer");
            netstring_encode(&(conn_tbl[conn_id].client_out_buffer),
              "stdout",
              6);
            netstring_encode(&(conn_tbl[conn_id].client_out_buffer),
              conn_tbl[conn_id].child_stdout.s,
              conn_tbl[conn_id].child_stdout.len);
            things_to_do = 1;
        }
        stralloc_free(&(conn_tbl[conn_id].child_stdout));
    }

    if(conn_tbl[conn_id].child_stderr.len)
    {
        if(conn_tbl[conn_id].client > -1)
        {
            LOLDEBUG("update_client: adding stderr to output buffer");
            netstring_encode(&(conn_tbl[conn_id].client_out_buffer),
              "stderr",
              6);
            netstring_encode(&(conn_tbl[conn_id].client_out_buffer),
              conn_tbl[conn_id].child_stderr.s,
              conn_tbl[conn_id].child_stderr.len);
            things_to_do = 1;
        }
        stralloc_free(&(conn_tbl[conn_id].child_stderr));
    }
    if(conn_tbl[conn_id].child_pid == 0 && conn_tbl[conn_id].child_exit_code >= 0)
    {
        if(conn_tbl[conn_id].client > -1)
        {
            LOLDEBUG("update_client: adding child exit code to output buffer");
            char ecode[UINT_FMT] = {0};
            int len = uint_fmt(ecode,conn_tbl[conn_id].child_exit_code);

            netstring_encode(&(conn_tbl[conn_id].client_out_buffer),
              "exitcode",
              8);
            netstring_encode(&(conn_tbl[conn_id].client_out_buffer),
              ecode,
              len);
            things_to_do = 1;
        }
        conn_tbl[conn_id].child_exit_code = -1;
    }

    if(conn_tbl[conn_id].child_pid == 0 && conn_tbl[conn_id].child_exit_signal >= 0)
    {
        if(conn_tbl[conn_id].client > -1)
        {
            LOLDEBUG("update_client: adding child exit signal to output buffer");

            netstring_encode(&(conn_tbl[conn_id].client_out_buffer),
              "termsig",
              7);
            netstring_encode(&(conn_tbl[conn_id].client_out_buffer),
              sig_name(conn_tbl[conn_id].child_exit_signal),
              str_len(sig_name(conn_tbl[conn_id].child_exit_signal)));
            things_to_do = 1;
        }
        conn_tbl[conn_id].child_exit_signal = -1;
    }


    if(things_to_do)
    {
        LOLDEBUG("update_client: setting client_fd to IOPAUSE_WRITE");
        fds_tbl[conn_tbl[conn_id].client].fd = conn_tbl[conn_id].client;
        fds_tbl[conn_tbl[conn_id].client].events = IOPAUSE_WRITE;
    }
#ifdef DEBUG
    else {
        LOLDEBUG("end_client: exit, no action");
    }
#endif
    return 1;
}
