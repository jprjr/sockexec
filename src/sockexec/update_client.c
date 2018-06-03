#include "common.h"

int update_client(conn_id)
int conn_id;
{
    int things_to_do = 0;

    enum CHILD_OUTPUT_TYPE output = CHILD_UNKNOWN;

    for(output = CHILD_UNKNOWN; output < CHILD_OUTPUT_NULL; output++)
    {
      if(conn_tbl[conn_id].child_outputs[output].len)
      {
          if(conn_tbl[conn_id].client > -1)
          {
              netstring_encode(&(conn_tbl[conn_id].client_out_buffer),
                CHILD_OUTPUT_TYPES[output],
                strlen(CHILD_OUTPUT_TYPES[output]));
              netstring_encode(&(conn_tbl[conn_id].client_out_buffer),
                conn_tbl[conn_id].child_outputs[output].s,
                conn_tbl[conn_id].child_outputs[output].len);
              things_to_do = 1;
          }
          stralloc_free(&(conn_tbl[conn_id].child_outputs[output]));
      }
    }

    if(conn_tbl[conn_id].child_pid == 0 && conn_tbl[conn_id].child_exit_code >= 0)
    {
        if(conn_tbl[conn_id].client > -1)
        {
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
            netstring_encode(&(conn_tbl[conn_id].client_out_buffer),
              "termsig",
              7);
            netstring_encode(&(conn_tbl[conn_id].client_out_buffer),
              sig_name(conn_tbl[conn_id].child_exit_signal),
              strlen(sig_name(conn_tbl[conn_id].child_exit_signal)));
            things_to_do = 1;
        }
        conn_tbl[conn_id].child_exit_signal = -1;
    }

    if(things_to_do)
    {
        /* there's stuff to send */
        fds_tbl[conn_tbl[conn_id].client].fd = conn_tbl[conn_id].client;
        fds_tbl[conn_tbl[conn_id].client].events = IOPAUSE_WRITE;
    }
    else {
        /* the child closed stdout/stderr - see if connection can be closed */
        close_connection(conn_id,0,0);
    }

    return 1;
}
