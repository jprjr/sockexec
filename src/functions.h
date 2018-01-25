#ifndef FUNCTIONS_H
#define FUNCTIONS_H

int process_signals(void);
int close_connection(int, int, int);
void cleanup(void);
void kill_processes(int);
void dump_connection(int,int);
void dump_fds(int);
int route_event(int);
int update_child(int);
int client_write(int, int);
int client_read(int, int);
int child_write(int, int, int);
int child_read(int, int);
int accept_client(int);
int update_client(int);
pid_t jprjr_child_spawn3 (char const *, char const *const *, char **, int *, int *, int *);

#endif

