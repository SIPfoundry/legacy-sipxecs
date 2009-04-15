#ifndef SYSDEP_H
#define SYSDEP_H


#if defined(unix) || defined(__unix) || defined (__unix__)
/* Code for Unix.  Any Unix compiler should define one of the above three
 * symbols. */

#if HAVE_CRYPT_H
#include <crypt.h>
#endif

#ifndef HAVE_GETHOSTBYNAME_R
struct hostent *gethostbyname_r (const char *name, struct hostent
                                 *result, char *buffer, int buflen,
                                 int *h_errnop);
#endif

#ifndef HAVE_STRTOK_R
char *strtok_r(char *s1, char *s2, char **lasts);
#endif

#if !defined(ETIMEDOUT) && defined(ETIME)
#define ETIMEDOUT ETIME
#endif

#if !defined(ETIME) && defined(ETIMEDOUT)
#define ETIME ETIMEDOUT
#endif

#ifndef DIR_SEPARATOR
#define DIR_SEPARATOR "/"
#endif

#ifndef TRUE
#define TRUE 1
#endif

#include <limits.h>
#ifndef INFINITE
#define INFINITE ULONG_MAX
#endif

#ifndef   WaitForMultipleObjects
#define   WaitForMultipleObjects(n, tid, b, f) \
do { \
  int _i; \
  for (_i = 0; _i < n; _i++) { \
    if (tid[_i]) { \
      pthread_join(tid[_i], NULL); \
    } \
  } \
} while (0)
#endif


#ifndef startupSocket
#define startupSocket()
#endif


#ifndef closesocket
#define closesocket close
#endif

#ifndef FILE_SOCKET
#define FILE_SOCKET FILE
#endif

#ifndef fdopen_socket
#define fdopen_socket(f, g) fdopen(f, g)
#endif

#ifndef fclose_socket
#define fclose_socket(f) fclose(f)
#endif

#ifndef getc_socket
#define getc_socket(f) getc(f)
#endif

#ifndef write_socket
#define write_socket(r, s, l) write(r, s, l)
#endif

/* end of 'if unix' */

#elif defined(WIN32)

#define NOLONGLONG
#define WNOHANG   0  /* used in waitpid */

#include <stdio.h>     /* required for compilation on win32.
                        * Probably types.h does not include WCHAR */

#include <winsock.h> /* For NT socket */
#include <sys/timeb.h> /* For _ftime() */
#include <sys/stat.h>  /* S_IWRITE */
#include <process.h> /* For _getpid() */
#include <resparse/wnt/crypt.h>
#include <io.h>  /* open, write, read */
#include <signal.h>    /* SIGINT */
#include <errno.h>
#include <resparse/wnt/nterrno.h>  /* Additional errors not in errno.h --GAT */
#include <direct.h>     /* chdir() */


#include <resparse/wnt/utilNT.h> /* For function and struct in UNIX but not in NT */
#include "resparse/types.h"

#ifndef stat
#define stat _stat
#endif

#ifndef strcasecmp
#define strcasecmp _stricmp
#endif

#ifndef strncasecmp
#define strncasecmp _strnicmp
#endif

#ifndef snprintf
#define snprintf _snprintf
#endif

#ifndef usleep
#define usleep(s) _sleep((s)/1000)
#endif

#ifndef open
#define open _open
#endif

#ifndef write
#define write _write
#endif

#ifndef close
#define close _close
#endif

#ifndef popen
#define popen _popen
#endif

#ifndef pclose
#define pclose _pclose
#endif

#ifndef _WIN32          // Conflicts with glib under windows
#ifndef pipe
#define pipe(p) _pipe(p, 256, O_BINARY)
#endif
#endif

#ifndef S_IRWXU
#define S_IRWXU S_IWRITE
#endif

#ifndef waitpid
#define waitpid(pid, status, act) _cwait(status, pid, act)
#endif

#ifndef dup2
#define dup2 _dup2
#endif

#ifndef getpid  /* used in random32.c */
#define getpid _getpid
#endif

#ifndef bcopy
#define bcopy(s, t, n) memcpy(t, s, n)
#endif

struct utsname {
  char uname[16];
};

#ifndef PATH_MAX
#define PATH_MAX        1024    /* For NT */
#endif

#ifndef DIR_SEPARATOR
#define DIR_SEPARATOR "\\"
#endif

#ifndef VERSION
#define VERSION "1.0"
#endif

#ifndef SIGBUS
#define SIGBUS SIGINT
#endif

#ifndef SIGHUP
#define SIGHUP SIGINT
#endif

#ifndef SIGPIPE
#define SIGPIPE SIGINT
#endif

#ifndef _WIN32                  /* Conflicts with glib under windows */
typedef long pid_t;
#endif
typedef long gid_t;
typedef long uid_t;

#ifndef HAVE_BOOLEAN_T
#define HAVE_BOOLEAN_T
/* Define boolean_t if macro HAVE_BOOLEAN_T is not defined */
typedef enum {
  B_FALSE = FALSE,
  B_TRUE = TRUE
} boolean_t;

#endif

typedef unsigned long u_long;
typedef unsigned int u_int;
typedef unsigned short u_short;
typedef unsigned char u_char;
typedef int     ssize_t;
typedef char *   caddr_t;        /* core address */
typedef long    fd_mask;
#define NBBY    8               /* number of bits in a byte */
#define NFDBITS (sizeof(fd_mask) * NBBY)        /* bits per mask */
#ifndef howmany
#define howmany(x, y)   (((x) + ((y) - 1)) / (y))
#endif

struct passwd {
        char    *pw_name;
        char    *pw_passwd;
        uid_t   pw_uid;
        gid_t   pw_gid;
        char    *pw_age;
        char    *pw_comment;
        char    *pw_gecos;
        char    *pw_dir;
        char    *pw_shell;
};

#ifndef _TIMESPEC_T
#define _TIMESPEC_T
typedef struct  timespec {              /* definition per POSIX.4 */
        time_t          tv_sec;         /* seconds */
        long            tv_nsec;        /* and nanoseconds */
} timespec_t;
#endif  /* _TIMESPEC_T */

#ifndef ETIME
#define ETIME 1
#endif

#ifndef SIGKILL
#define SIGKILL SIGTERM
#endif

#define fork() 0
#define setsid() {}

#ifndef startupSocket
#define startupSocket() {WSADATA wsaData;  \
  if (WSAStartup(0x0101, &wsaData))    \
    printf("Could not open start up windows socket.\n"); }
#endif

#ifndef FILE_SOCKET
#define FILE_SOCKET int
#endif

#ifndef fdopen_socket
#define fdopen_socket(f, g) &f
#endif

#ifndef fclose_socket
#define fclose_socket(f) closesocket(*f)
#endif

#ifndef gethostbyname_r
#define gethostbyname_r(h,a,b,c,d) gethostbyname(h)
#endif

extern int getc_socket(FILE_SOCKET *f);
extern ssize_t write_socket(int fildes, const void *buf, size_t nbyte);

#define S_IRUSR      0400  /* User read permission */
#define S_IWUSR      0200  /* User write permission */
#define S_IXUSR      0100  /* User execute permission */

#define S_IRGRP      0040  /* Groupread permission */
#define S_IWGRP      0020  /* groupwrite permission */
#define S_IXGRP      0010  /* groupexecute permission */

#define S_IROTH      0004  /* otherread permission */
#define S_IWOTH      0002  /* otherwrite permission */
#define S_IXOTH      0001  /* otherexecute permission */


/* end of 'ifdef WIN32' */
#else
#error "Not Unix or WIN32 -- what system is this?"
#endif

#define DO_NOTHING   2
#define ADD_PROXY    1
#define REMOVE_PROXY 0

#endif /* end of ifdef SYSDEP_H */
