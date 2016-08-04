#include "common.h"

void dump_connection(conn_id,display)
int conn_id;
int display;
{
    if(display ||
       conn_tbl[conn_id].client > 0 ||
       conn_tbl[conn_id].child_stdin_fd > 0 ||
       conn_tbl[conn_id].child_stdout_fd > 0 ||
       conn_tbl[conn_id].child_stderr_fd > 0)
    {
        fprintf(stderr,"------------------------------\n");
        fprintf(stderr,"conn_id: %d\n",conn_id);
        fprintf(stderr,"  client_fd:  %d\n",conn_tbl[conn_id].client);
        fprintf(stderr,"  stdin_fd:   %d\n",conn_tbl[conn_id].child_stdin_fd);
        fprintf(stderr,"  stdout_fd:  %d\n",conn_tbl[conn_id].child_stdout_fd);
        fprintf(stderr,"  stderr_fd:  %d\n",conn_tbl[conn_id].child_stderr_fd);
        fprintf(stderr,"  child_pid:  %d\n",conn_tbl[conn_id].child_pid);
        fprintf(stderr,"  child_exit_code:  %d\n",conn_tbl[conn_id].child_exit_code);
        fprintf(stderr,"  child_exit_signal:  %d\n",conn_tbl[conn_id].child_exit_signal);
    }
}
