#include "common.h"

void kill_processes(force)
int force;
{
    unsigned int i;
    int signal = SIGTERM;
    for(i=0; i < conn_tbl_len; i++)
    {
        if(force || tain_lesseq(&(conn_tbl[i].deadline),now))
        {
            if(conn_tbl[i].child_pid > 0)
            {
                if(conn_tbl[i].sigsent)
                {
                    signal = SIGKILL;
                }
                LOLDEBUG("kill_processes: sending %s to %d",sig_name(signal),conn_tbl[i].child_pid);
                kill(conn_tbl[i].child_pid, signal);
                conn_tbl[i].sigsent = 1;
            }
            else if(conn_tbl[i].client > -1)
            {
                close_connection(i,0);
            }
        }
        else
        {
            if(deadline == 0 || tain_less(deadline,&(conn_tbl[i].deadline)))
            {
                LOLDEBUG("kill_processes: setting new global deadline");
                deadline = &(conn_tbl[i].deadline);
            }
        }
    }
}
