/* wwwconf.h.  Generated automatically by configure.  */
/* wwwconf.h.in.  Generated automatically from configure.in by autoheader.  */

/* Define if type char is unsigned and you are not using gcc.  */
#ifndef __CHAR_UNSIGNED__
/* #undef __CHAR_UNSIGNED__ */
#endif

/* Define to empty if the keyword does not work.  */
/* #undef const */

/* Define to the type of elements in the array set by `getgroups'.
   Usually this is either `int' or `gid_t'.  */
#define GETGROUPS_T gid_t

/* Define to `int' if <sys/types.h> doesn't define.  */
/* #undef gid_t */

/* Define if you don't have vprintf but do have _doprnt.  */
/* #undef HAVE_DOPRNT */

/* Define if the `long double' type works.  */
#define HAVE_LONG_DOUBLE 1

/* Define if you have the strftime function.  */
#define HAVE_STRFTIME 1

/* Define if you have <sys/wait.h> that is POSIX.1 compatible.  */
#define HAVE_SYS_WAIT_H 1

/* Define if your struct tm has tm_zone.  */
#define HAVE_TM_ZONE 1

/* Define if you don't have tm_zone but do have the external array
   tzname.  */
/* #undef HAVE_TZNAME */

/* Define if you have the vprintf function.  */
#define HAVE_VPRINTF 1

/* Define to `int' if <sys/types.h> doesn't define.  */
/* #undef mode_t */

/* Define to `int' if <sys/types.h> doesn't define.  */
/* #undef pid_t */

/* Define if you need to in order for stat and other things to work.  */
/* #undef _POSIX_SOURCE */

/* Define as the return type of signal handlers (int or void).  */
#define RETSIGTYPE void

/* Define to `unsigned' if <sys/types.h> doesn't define.  */
/* #undef size_t */

/* Define if the `S_IS*' macros in <sys/stat.h> do not work properly.  */
/* #undef STAT_MACROS_BROKEN */

/* Define if you have the ANSI C header files.  */
#define STDC_HEADERS 1

/* Define if you can safely include both <sys/time.h> and <time.h>.  */
#define TIME_WITH_SYS_TIME 1

/* Define if your <sys/time.h> declares struct tm.  */
/* #undef TM_IN_SYS_TIME */

/* Define to `int' if <sys/types.h> doesn't define.  */
/* #undef uid_t */

/* Define if your processor stores words with the most significant
   byte first (like Motorola and SPARC, unlike Intel and VAX).  */
/* #undef WORDS_BIGENDIAN */

/* Define to enable direct WAIS access. */
/* #undef HT_DIRECT_WAIS */

/* Define to enable mysql access. */
/* #undef HT_MYSQL */

/* Define to enable MD5 for HTTP access authentication. */
#define HT_MD5 1

/* Define to enable expat XML parser. */
/* #undef HT_EXPAT */

/* Define to enable Zlib compression / decompression support. */
/* #undef HT_ZLIB */

/* Define to enable POSIX RegEx support. */
/* #undef HT_POSIX_REGEX */

/* Define to enable support for enabling a rules file w/o user interaction. */
/* #undef HT_AUTOMATIC_RULES */

/* Define to enable SOCKS firewall-breaching code. */
/* #undef SOCKS */
/* #undef SOCKS4 */
/* #undef SOCKS5 */

/* Define to build Cyrillic version. */
/* #undef CYRILLIC */

/* Define to use internal signal handler */
/* #undef WWWLIB_SIG */

/* Define to build using reentrant system calls. */
/* #undef HT_REENTRANT */

/* Define to use the two-argument variant of ctime_r */
/* #undef HAVE_CTIME_R_2 */

/* Define to use the three-argument variant of ctime_r */
/* #undef HAVE_CTIME_R_3 */

/* Define to use the two-argument variant of readdir_r */
/* #undef HAVE_READDIR_R_2 */

/* Define to use the three-argument variant of readdir_r */
/* #undef HAVE_READDIR_R_3 */

/* Define to use the three-argument variant of gethostbyname_r */
/* #undef HAVE_GETHOSTBYNAME_R_3 */

/* Define to use the five-argument variant of gethostbyname_r */
/* #undef HAVE_GETHOSTBYNAME_R_5 */

/* Define to use the six-argument variant of gethostbyname_r */
/* #undef HAVE_GETHOSTBYNAME_R_6 */

/* Define to use the five-argument variant of gethostbyaddr_r */
/* #undef HAVE_GETHOSTBYADDR_R_5 */

/* Define to use the seven-argument variant of gethostbyaddr_r */
/* #undef HAVE_GETHOSTBYADDR_R_7 */

/* Define to use the eight-argument variant of gethostbyaddr_r */
/* #undef HAVE_GETHOSTBYADDR_R_8 */

/* Define if getlogin_r returns an integer */
/* #undef GETLOGIN_R_RETURNS_INT */

/* Define if getlogin_r returns a pointer */
/* #undef GETLOGIN_R_RETURNS_POINTER */

/* Define to be the package name. */
#define W3C_PACKAGE "w3c-libwww"

/* Define to be the version. */
#define W3C_VERSION "5.3.2"

/* Define this to be the prefix for cache files. */
#define CACHE_FILE_PREFIX "/usr/wsrc/"

/* Define to disable Nagle's algorithm */
#define HT_NO_NAGLE 1

/* Define to disable HTTP/1.1 pipelining */
/* #undef HT_NO_PIPELINING */

/* Define to disable HTTP/1.1 pipelining */
/* #undef HT_NO_PIPELINING */

/* Define to enable MUX as HTTP transport */
/* #undef HT_MUX */

/* Define this if it's not typedef'd automatically. */
#define BOOLEAN char

/* Define this to be the location of the resolver configuration file. */
#define RESOLV_CONF "/etc/resolv.conf"

/* Define this to be the rlogin program name. */
#define RLOGIN_PROGRAM "rlogin"

/* Define this to be the telnet program name. */
#define TELNET_PROGRAM "telnet"

/* Define this to be the tn3270 program name. */
/* #undef TN3270_PROGRAM */

/* Define this if it isn't in the header files.  */
/* #undef u_char */

/* For removing a file */
/* #undef unlink */

/* Define this if it isn't in the header files.  */
/* #undef u_short */

/* Define this if it isn't in the header files.  */
/* #undef u_long */

/* Define if type_t is of type long */
#define HAVE_LONG_TIME_T 1

/* Define if size_t is of type long */
/* #undef HAVE_LONG_SIZE_T */

/* The number of bytes in a char.  */
#define SIZEOF_CHAR 1

/* The number of bytes in a char *.  */
#define SIZEOF_CHAR_P 4

/* The number of bytes in a int.  */
#define SIZEOF_INT 4

/* The number of bytes in a long.  */
#define SIZEOF_LONG 4

/* Define if you have the chdir function.  */
#define HAVE_CHDIR 1

/* Define if you have the fcntl function.  */
#define HAVE_FCNTL 1

/* Define if you have the getcwd function.  */
#define HAVE_GETCWD 1

/* Define if you have the getdomainname function.  */
#define HAVE_GETDOMAINNAME 1

/* Define if you have the gethostname function.  */
#define HAVE_GETHOSTNAME 1

/* Define if you have the getlogin function.  */
#define HAVE_GETLOGIN 1

/* Define if you have the getpass function.  */
#define HAVE_GETPASS 1

/* Define if you have the getpid function.  */
#define HAVE_GETPID 1

/* Define if you have the getsockopt function.  */
#define HAVE_GETSOCKOPT 1

/* Define if you have the gettimeofday function.  */
#define HAVE_GETTIMEOFDAY 1

/* Define if you have the getwd function.  */
#define HAVE_GETWD 1

/* Define if you have the ioctl function.  */
#define HAVE_IOCTL 1

/* Define if you have the memcpy function.  */
#define HAVE_MEMCPY 1

/* Define if you have the mktime function.  */
#define HAVE_MKTIME 1

/* Define if you have the opendir function.  */
#define HAVE_OPENDIR 1

/* Define if you have the readdir function.  */
#define HAVE_READDIR 1

/* Define if you have the select function.  */
#define HAVE_SELECT 1

/* Define if you have the setsockopt function.  */
#define HAVE_SETSOCKOPT 1

/* Define if you have the socket function.  */
#define HAVE_SOCKET 1

/* Define if you have the strchr function.  */
#define HAVE_STRCHR 1

/* Define if you have the strerror function.  */
#define HAVE_STRERROR 1

/* Define if you have the strtol function.  */
#define HAVE_STRTOL 1

/* Define if you have the sysinfo function.  */
#define HAVE_SYSINFO 1

/* Define if you have the tempnam function.  */
#define HAVE_TEMPNAM 1

/* Define if you have the timegm function.  */
#define HAVE_TIMEGM 1

/* Define if you have the tzset function.  */
#define HAVE_TZSET 1

/* Define if you have the <appkit.h> header file.  */
/* #undef HAVE_APPKIT_H */

/* Define if you have the <appkit/appkit.h> header file.  */
/* #undef HAVE_APPKIT_APPKIT_H */

/* Define if you have the <arpa/inet.h> header file.  */
#define HAVE_ARPA_INET_H 1

/* Define if you have the <bsdtime.h> header file.  */
/* #undef HAVE_BSDTIME_H */

/* Define if you have the <bsdtypes.h> header file.  */
/* #undef HAVE_BSDTYPES_H */

/* Define if you have the <ctype.h> header file.  */
#define HAVE_CTYPE_H 1

/* Define if you have the <curses.h> header file.  */
#define HAVE_CURSES_H 1

/* Define if you have the <cursesX.h> header file.  */
/* #undef HAVE_CURSESX_H */

/* Define if you have the <dir.h> header file.  */
/* #undef HAVE_DIR_H */

/* Define if you have the <direct.h> header file.  */
/* #undef HAVE_DIRECT_H */

/* Define if you have the <dirent.h> header file.  */
#define HAVE_DIRENT_H 1

/* Define if you have the <dn.h> header file.  */
/* #undef HAVE_DN_H */

/* Define if you have the <dnetdb.h> header file.  */
/* #undef HAVE_DNETDB_H */

/* Define if you have the <errno.h> header file.  */
#define HAVE_ERRNO_H 1

/* Define if you have the <fcntl.h> header file.  */
#define HAVE_FCNTL_H 1

/* Define if you have the <grp.h> header file.  */
#define HAVE_GRP_H 1

/* Define if you have the <in.h> header file.  */
/* #undef HAVE_IN_H */

/* Define if you have the <inet.h> header file.  */
/* #undef HAVE_INET_H */

/* Define if you have the <libc.h> header file.  */
/* #undef HAVE_LIBC_H */

/* Define if you have the <limits.h> header file.  */
#define HAVE_LIMITS_H 1

/* Define if you have the <malloc.h> header file.  */
#define HAVE_MALLOC_H 1

/* Define if you have the <manifest.h> header file.  */
/* #undef HAVE_MANIFEST_H */

/* Define if you have the <memory.h> header file.  */
#define HAVE_MEMORY_H 1

/* Define if you have the <ndir.h> header file.  */
/* #undef HAVE_NDIR_H */

/* Define if you have the <net/errno.h> header file.  */
/* #undef HAVE_NET_ERRNO_H */

/* Define if you have the <netdb.h> header file.  */
#define HAVE_NETDB_H 1

/* Define if you have the <netinet/in.h> header file.  */
#define HAVE_NETINET_IN_H 1

/* Define if you have the <netinet/tcp.h> header file.  */
#define HAVE_NETINET_TCP_H 1

/* Define if you have the <pwd.h> header file.  */
#define HAVE_PWD_H 1

/* Define if you have the <regex.h> header file.  */
#define HAVE_REGEX_H 1

/* Define if you have the <resource.h> header file.  */
/* #undef HAVE_RESOURCE_H */

/* Define if you have the <rxposix.h> header file.  */
/* #undef HAVE_RXPOSIX_H */

/* Define if you have the <select.h> header file.  */
/* #undef HAVE_SELECT_H */

/* Define if you have the <socket.h> header file.  */
/* #undef HAVE_SOCKET_H */

/* Define if you have the <stat.h> header file.  */
/* #undef HAVE_STAT_H */

/* Define if you have the <stdefs.h> header file.  */
/* #undef HAVE_STDEFS_H */

/* Define if you have the <stdio.h> header file.  */
#define HAVE_STDIO_H 1

/* Define if you have the <stdlib.h> header file.  */
#define HAVE_STDLIB_H 1

/* Define if you have the <string.h> header file.  */
#define HAVE_STRING_H 1

/* Define if you have the <strings.h> header file.  */
#define HAVE_STRINGS_H 1

/* Define if you have the <sys/dir.h> header file.  */
/* #undef HAVE_SYS_DIR_H */

/* Define if you have the <sys/errno.h> header file.  */
#define HAVE_SYS_ERRNO_H 1

/* Define if you have the <sys/fcntl.h> header file.  */
#define HAVE_SYS_FCNTL_H 1

/* Define if you have the <sys/file.h> header file.  */
#define HAVE_SYS_FILE_H 1

/* Define if you have the <sys/ioctl.h> header file.  */
#define HAVE_SYS_IOCTL_H 1

/* Define if you have the <sys/ipc.h> header file.  */
#define HAVE_SYS_IPC_H 1

/* Define if you have the <sys/limits.h> header file.  */
/* #undef HAVE_SYS_LIMITS_H */

/* Define if you have the <sys/machine.h> header file.  */
/* #undef HAVE_SYS_MACHINE_H */

/* Define if you have the <sys/ndir.h> header file.  */
/* #undef HAVE_SYS_NDIR_H */

/* Define if you have the <sys/resource.h> header file.  */
#define HAVE_SYS_RESOURCE_H 1

/* Define if you have the <sys/select.h> header file.  */
#define HAVE_SYS_SELECT_H 1

/* Define if you have the <sys/socket.h> header file.  */
#define HAVE_SYS_SOCKET_H 1

/* Define if you have the <sys/stat.h> header file.  */
#define HAVE_SYS_STAT_H 1

/* Define if you have the <sys/syslog> header file.  */
/* #undef HAVE_SYS_SYSLOG */

/* Define if you have the <sys/systeminfo.h> header file.  */
/* #undef HAVE_SYS_SYSTEMINFO_H */

/* Define if you have the <sys/time.h> header file.  */
#define HAVE_SYS_TIME_H 1

/* Define if you have the <sys/types.h> header file.  */
#define HAVE_SYS_TYPES_H 1

/* Define if you have the <sys/unistd.h> header file.  */
#define HAVE_SYS_UNISTD_H 1

/* Define if you have the <syslog.h> header file.  */
#define HAVE_SYSLOG_H 1

/* Define if you have the <tcp.h> header file.  */
/* #undef HAVE_TCP_H */

/* Define if you have the <termios.h> header file.  */
#define HAVE_TERMIOS_H 1

/* Define if you have the <time.h> header file.  */
#define HAVE_TIME_H 1

/* Define if you have the <types.h> header file.  */
/* #undef HAVE_TYPES_H */

/* Define if you have the <unistd.h> header file.  */
#define HAVE_UNISTD_H 1

/* Define if you have the <unixlib.h> header file.  */
/* #undef HAVE_UNIXLIB_H */

/* Define if you have the <wais.h> header file.  */
/* #undef HAVE_WAIS_H */

/* Define if you have the <wais/wais.h> header file.  */
/* #undef HAVE_WAIS_WAIS_H */

/* Define if you have the dl library (-ldl).  */
#define HAVE_LIBDL 1

/* Define if you have the inet library (-linet).  */
/* #undef HAVE_LIBINET */

/* Define if you have the nsl library (-lnsl).  */
/* #undef HAVE_LIBNSL */

/* Define if you have the socket library (-lsocket).  */
/* #undef HAVE_LIBSOCKET */

/* Name of package */
#define PACKAGE "w3c-libwww"

/* Version number of package */
#define VERSION "5.3.2"


/* Define this if you have the external variable 'socket_errno'. */
/* #undef HAVE_SOCKET_ERRNO */

/* Define this if you have the external variable 'sys_errlist'. */
/* #undef HAVE_SYS_ERRLIST */

/* Define this if you have the external variable 'sys_nerr'. */
/* #undef HAVE_SYS_NERR */

/* Define this if you have the external variable 'timezone' */
#define HAVE_TIMEZONE 1

/* Define this if you have the external variable 'altzone' */
/* #undef HAVE_ALTZONE */

/* Define this if you have the external variable 'altzone' */
#define HAVE_DAYLIGHT 1

/* Define this if you have the uxc$inetdef.h header file. */
/* #undef HAVE_UXC_INETDEF_H */

/* Define this if directory entries have inodes. */
/* #undef HAVE_DIRENT_INO */

/* Define this if tm structure includes a tm_gmtoff entry. */
#define HAVE_TM_GMTOFF 1

/* Define this if your ioctl understands the winsize structure. */
#define HAVE_WINSIZE 1

/* Define this if the extern timezone is negative. */
/* #undef NEGATIVE_TIMEZONE */

/* Define this if sys_errlist must be declared (if it exists). */
/* #undef NEED_SYS_ERRLIST_DECLARED */

/* Define this if sys_nerr must be declared (if it exists). */
/* #undef NEED_SYS_NERR_DECLARED */


