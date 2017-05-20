#include "common.h"

void kill_processes(force)
int force;
{
    unsigned int i;
    tain_t c_now;
    int signal = SIGTERM;
    int has_deadline = 0;
    int deadline_reached = 0;

    for(i=0; i < conn_tbl_len; i++)
    {

        has_deadline = tain_less(&tain_zero,&conn_tbl[i].deadline);
        deadline_reached = tain_lesseq(&(conn_tbl[i].deadline),now);

        if(force || (has_deadline && deadline_reached))
        {
            if(conn_tbl[i].child_pid > 0)
            {
                if(conn_tbl[i].sigsent)
                {
                    signal = SIGKILL;
                }
                else
                {
                    tain_now(&c_now);
                    tain_addsec(&(conn_tbl[i].deadline),&c_now,kill_timeout);
                    if(deadline == 0 || tain_less(&(conn_tbl[i].deadline),deadline))
                    {
                        deadline = &(conn_tbl[i].deadline);
                    }
                }
                if(debug)
                {
                    fprintf(stderr,"Connection %d: sending %s to child process\n",i,sig_name(signal));
                }

                kill(conn_tbl[i].child_pid, signal);
                conn_tbl[i].sigsent = 1;
            }
            else if(conn_tbl[i].client > -1)
            {
                close_connection(i,0,0);
            }
        }
        else
        {
            if(has_deadline)
            {
                if( deadline == 0 || tain_less(deadline,&(conn_tbl[i].deadline)))
                {
                    deadline = &(conn_tbl[i].deadline);
                }
            }
        }
    }
}
