#include <skalibs/strerr2.h>
#include <skalibs/stralloc.h>
#include <skalibs/netstring.h>
#include <skalibs/uint64.h>
#include <skalibs/webipc.h>
#include <skalibs/allreadwrite.h>
#include <skalibs/types.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define USAGE "sockexec.client /path/to/socket arg1 arg2 ... argn"
#define dieusage() strerr_dieusage(100, USAGE)
#define dienomem()    strerr_diefu1sys(111, "stralloc_catb")
#define dienosocket() strerr_diefu1sys(111, "ipc_stream_nb")
#define dienoconnect()   strerr_diefu1sys(111, "ipc_connect")

int main(int argc, char const *const *argv) {
    stralloc sa = STRALLOC_ZERO;
    PROG = "sockexec.client";
    char num_args[UINT64_FMT];
    char buffer[4096];
    char *b = buffer;
    ssize_t pos = 0;
    size_t num_len = 0;
    int fd = 0;
    int tag = 0;
    int data = 0;
    // 0 = stdout
    // 1 = stderr
    // 2 = exitcode
    // 3 = termsig

    if(argc < 3) {
        dieusage();
    }

    unsigned int exitcode = 0;
    unsigned int termsig = 0;

    argv++;
    argc--;
    char const *p = argv[0];

    fd = ipc_stream_b();
    if(fd < 0) {
        dienosocket();
    }
    if(!ipc_connect(fd,p)) {
        dienoconnect();
    }

    argv++;
    argc--;

    num_len = uint64_fmt(num_args,argc);

    netstring_encode(&sa,num_args,num_len);

    while(argc > 0) {
        netstring_encode(&sa,argv[0],strlen(argv[0]));
        argv++;
        argc--;
    }

    if(!fd_send(fd,sa.s,sa.len,0)) {
        strerr_diefu1sys(111,"ipc_send");
    }
    stralloc_free(&sa);

    num_len = 0;
    if(!isatty(fileno(stdin))) {
        while((num_len = fd_read(fileno(stdin),buffer,4096)) > 0) {
            netstring_encode(&sa,buffer,num_len);

            if(!fd_send(fd,sa.s,sa.len,0)) {
                strerr_diefu1sys(111,"ipc_send");
            }
            stralloc_free(&sa);
        }
        num_len = 0;
    }

    netstring_encode(&sa,"",0);

    if(!fd_send(fd,sa.s,sa.len,0)) {
        strerr_diefu1sys(111,"ipc_send");
    }

    stralloc_free(&sa);

    while((num_len = fd_recv(fd,buffer,4096,0)) > 0) {
        b = buffer;
        while((pos = netstring_decode(&sa,b,num_len)) > 0) {
            if(tag == 0) {
                stralloc_0(&sa);
                if(strcmp(sa.s,"stdout") == 0) {
                    data = 0;
                }
                else if(strcmp(sa.s,"stderr") == 0) {
                    data = 1;
                }
                else if(strcmp(sa.s,"exitcode") == 0) {
                    data = 2;
                }
                else if(strcmp(sa.s,"termsig") == 0) {
                    data = 3;
                }
                tag = 1;
            }
            else {
                if(data == 0) {
                    fd_write(1,sa.s,sa.len);
                }
                else if(data == 1) {
                    fd_write(2,sa.s,sa.len);
                }
                else if (data == 2) {
                    stralloc_0(&sa);
                    uint0_scan(sa.s,&exitcode);
                }
                else if (data == 3) {
                    stralloc_0(&sa);
                    uint0_scan(sa.s,&termsig);
                }
                tag = 0;
            }
            b += pos;
            num_len -= pos;
            stralloc_free(&sa);
        }

    }

    if(termsig != 0) {
        fprintf(stderr,"// killed by signal %d\n",termsig);
    }

    return exitcode;


}

