#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <signal.h>
#include <unistd.h>

#include "sipXecsService/daemon.h"

void daemonize(const char* pidfile) {
    if (getppid() == 1) {
        return; // already a daemon
    }
    pid_t i = fork();
    if (i < 0) {
        exit(1);  // child cannot fork!
    }
    if (i > 0) {
        exit(0); // parent exits
    }
    // child (daemon) continues
    setsid(); // obtain a new process group
    for (int i = getdtablesize(); i >= 0; --i) {
        // fork need to squeltch all STDIO or parent is left w/open resources
        close(i);
    }

    // reopen stdout, err and in incase spawned process need stub fds
    i = open("/dev/null", O_RDWR);
    dup(i); 
    dup(i);

    umask(027); // set newly created file permissions

    // safe when runner was root
    //chdir(RUNNING_DIR);

    int lockfp = open(pidfile, O_RDWR|O_CREAT, 0640);
    if (lockfp < 0) {
        // cannot even reach lock file
        exit(1);
    }

    if (lockf(lockfp, F_TLOCK, 0) < 0) {
        // trying to run a second instance, no need, abort
        exit(0);
    }

    /* first instance continues */
    char str[10];
    sprintf(str, "%d\n", getpid());
    write(lockfp, str, strlen(str)); /* record pid to lockfile */

    signal(SIGCHLD, SIG_IGN); // ignore child - May be a problem on BSD
    signal(SIGTSTP, SIG_IGN); // ignore tty signals 
    signal(SIGTTOU, SIG_IGN);
    signal(SIGTTIN, SIG_IGN);
}
