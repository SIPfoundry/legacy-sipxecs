#ifndef _SYS_SIGNAL_H_
#define _SYS_SIGNAL_H_ 

#if defined(_WIN32) /* Use only for win32 code --GAT */

//DWW Not needed for WinNT. It's already in signal.h
//#define SIG_DFL         ((__sighandler_t *)0)
//#define SIG_IGN         ((__sighandler_t *)1)
//#define SIG_ERR         ((__sighandler_t *)-1) 

#define SIGSYS  12      /* non-existent system call invoked */

typedef void __sighandler_t (int);
typedef unsigned int sigset_t;

/* 
 * Signal vector "template" used in sigaction call.
 */
struct  sigaction {
        __sighandler_t *sa_handler;     /* signal handler */
        sigset_t sa_mask;               /* signal mask to apply */ 
        int     sa_flags;               /* see signal options below */
}; 

int sigaction(int sig, const struct sigaction *act, struct sigaction *oact);

#endif

#endif
