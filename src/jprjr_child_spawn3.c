/*
 * Modified version of Laurent Bercot's child_spawn
 * function from skalibs.
*/

/* ISC license. */

/* MT-unsafe */

#include <skalibs/sysdeps.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <unistd.h>
#include <errno.h>
#include <signal.h>
#include <skalibs/allreadwrite.h>
#include <skalibs/bytestr.h>
#include <skalibs/env.h>
#include <skalibs/djbunix.h>
#include <skalibs/types.h>

#ifdef SKALIBS_HASPOSIXSPAWN

#include <stdlib.h>
#include <spawn.h>
#include <skalibs/config.h>

#else

#include <skalibs/sig.h>
#include <skalibs/strerr2.h>

#endif

pid_t jprjr_child_spawn3 (char const *prog, char const *const *argv, char const *const *envp, int *child_stdin, int *child_stdout, int *child_stderr)
{
#ifdef SKALIBS_HASPOSIXSPAWN
  posix_spawn_file_actions_t actions ;
  posix_spawnattr_t attr ;
#else
  int syncpipe[2] ;
#endif
  int p[3][2] ;
  pid_t pid ;
  int e ;
  unsigned int i = 0 ;
  for (; i < 3 ; i++) if (pipe(p[i]) < 0) { e = errno ; goto errp ; }
  for (i = 0 ; i < 3 ; i++)
  {
    if ((coe(p[i][i > 0]) < 0))
    {
      e = errno ; goto errp ;
    }
  }

  if(ndelay_on(p[0][1]) < 0)
  {
      e = errno; goto errp;
  }

#ifdef SKALIBS_HASPOSIXSPAWN

  e = posix_spawnattr_init(&attr) ;
  if (e) goto errsp ;
  {
    sigset_t set ;
    sigemptyset(&set) ;
    e = posix_spawnattr_setsigmask(&attr, &set) ;
    if (e) goto errattr ;
    e = posix_spawnattr_setflags(&attr, POSIX_SPAWN_SETSIGMASK) ;
    if (e) goto errattr ;
  }
  e = posix_spawn_file_actions_init(&actions) ;
  if (e) goto errattr ;

  e = posix_spawn_file_actions_adddup2(&actions, p[0][0], 0) ;
  if (e) goto erractions ;
  e = posix_spawn_file_actions_addclose(&actions, p[0][0]) ;
  if (e) goto erractions ;
  e = posix_spawn_file_actions_addclose(&actions, p[0][1]) ;
  if (e) goto erractions ;

  e = posix_spawn_file_actions_adddup2(&actions, p[1][1], 1) ;
  if (e) goto erractions ;
  e = posix_spawn_file_actions_addclose(&actions, p[1][1]) ;
  if (e) goto erractions ;
  e = posix_spawn_file_actions_addclose(&actions, p[1][0]) ;
  if (e) goto erractions ;

  e = posix_spawn_file_actions_adddup2(&actions, p[2][1], 2) ;
  if (e) goto erractions ;
  e = posix_spawn_file_actions_addclose(&actions, p[2][1]) ;
  if (e) goto erractions ;
  e = posix_spawn_file_actions_addclose(&actions, p[2][0]) ;
  if (e) goto erractions ;

  {
    e = posix_spawnp(&pid, prog, &actions, &attr, (char *const *)argv, (char *const *)envp) ;
    if (e) goto erractions ;
  }

  posix_spawn_file_actions_destroy(&actions) ;
  posix_spawnattr_destroy(&attr) ;

#else
  if (pipe(syncpipe) < 0) { e = errno ; goto errp ; }
  if (coe(syncpipe[1]) < 0) { e = errno ; goto errsp ; }

  pid = fork() ;
  if (pid < 0) { e = errno ; goto errsp ; }
  else if (!pid)
  {
    fd_close(syncpipe[0]) ;
    if(fd_move(0,p[0][0]) < 0) goto syncdie;
    if(fd_move(1,p[1][1]) < 0) goto syncdie;
    if(fd_move(2,p[2][1]) < 0) goto syncdie;
    sig_blocknone() ;
    pathexec_run(prog, argv, envp);

  syncdie:
    {
      unsigned char c = errno ;
      fd_write(syncpipe[1], (char const *)&c, 1) ;
    }
    _exit(127) ;
  }

  fd_close(syncpipe[1]) ;
  {
    char c ;
    syncpipe[1] = fd_read(syncpipe[0], &c, 1) ;
    if (syncpipe[1])
    {
      if (syncpipe[1] < 0) e = errno ;
      else
      {
        kill(pid, SIGKILL) ;
        e = c ;
      }
      wait_pid(pid, &syncpipe[1]) ;
      goto errsp0 ;
    }
  }
  fd_close(syncpipe[0]) ;
#endif

  fd_close(p[0][0]);
  fd_close(p[1][1]);
  fd_close(p[2][1]);

  *child_stdin  = p[0][1];
  *child_stdout = p[1][0];
  *child_stderr = p[2][0];
  return pid ;

#ifdef SKALIBS_HASPOSIXSPAWN
 erractions:
  posix_spawn_file_actions_destroy(&actions) ;
 errattr:
  posix_spawnattr_destroy(&attr) ;
#endif
 errsp:
#ifndef SKALIBS_HASPOSIXSPAWN
  fd_close(syncpipe[1]) ;
 errsp0:
  fd_close(syncpipe[0]) ;
#endif
 errp:
  fd_close(p[2][1]) ;
  fd_close(p[2][0]) ;
  fd_close(p[1][1]) ;
  fd_close(p[1][0]) ;
  fd_close(p[0][1]) ;
  fd_close(p[0][0]) ;
  errno = e ;
  return 0 ;
}
