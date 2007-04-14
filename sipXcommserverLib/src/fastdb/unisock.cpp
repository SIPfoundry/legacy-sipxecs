//-< UNISOCK.CPP >---------------------------------------------------*--------*
// FastDB                    Version 1.0         (c) 1999  GARRET    *     ?  *
// (Main Memory Database Management System)                          *   /\|  *
//                                                                   *  /  \  *
//                          Created:      8-Feb-97    K.A. Knizhnik  * / [] \ *
//                          Last update: 18-May-97    K.A. Knizhnik  * GARRET *
//-------------------------------------------------------------------*--------*
// Unix sockets  
//-------------------------------------------------------------------*--------*

#if defined(__FreeBSD__) || defined(__linux__)
#include <sys/ioctl.h>
#else
#include <stropts.h>
#endif
#include <fcntl.h>
#include <sys/time.h>
#include <sys/errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/utsname.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stddef.h>
#include <assert.h>
#include <errno.h>

extern "C" {
#include <netdb.h>
}

#include "unisock.h"

#include <signal.h>

#define MAX_HOST_NAME     256

const char* unix_socket::unix_socket_dir = SIPX_TMPDIR;

class unix_socket_library { 
  public: 
    unix_socket_library() { 
	static struct sigaction sigpipe_ignore; 
        sigpipe_ignore.sa_handler = SIG_IGN;
	sigaction(SIGPIPE, &sigpipe_ignore, NULL);
    }
};

static unix_socket_library unisock_lib;

bool unix_socket::open(int listen_queue_size)
{
    char hostname[MAX_HOST_NAME];
    unsigned short port;
    char* p;

    assert(address != NULL);

    if ((p = strchr(address, ':')) == NULL 
	|| unsigned(p - address) >= sizeof(hostname) 
	|| sscanf(p+1, "%hu", &port) != 1) 
    {
	errcode = bad_address;
	return false;
    }
    memcpy(hostname, address, p - address);
    hostname[p - address] = '\0';
    
    create_file = false; 
    union { 
	sockaddr    sock;
	sockaddr_in sock_inet;
	char        name[MAX_HOST_NAME];
    } u;
    int len;

    if (domain == sock_local_domain) { 
	u.sock.sa_family = AF_UNIX;

	assert(strlen(unix_socket_dir) + strlen(address) 
	       < MAX_HOST_NAME - offsetof(sockaddr,sa_data)); 
	
	len = offsetof(sockaddr,sa_data) + 
	    sprintf(u.sock.sa_data, "%s%s", unix_socket_dir, address);

	unlink(u.sock.sa_data); // remove file if existed
	create_file = true; 
    } else {
	u.sock_inet.sin_family = AF_INET;
	if (*hostname && strcmp(hostname, "localhost") != 0) {
	    struct hostent* hp;  // entry in hosts table
	    if ((hp = gethostbyname(hostname)) == NULL 
		|| hp->h_addrtype != AF_INET) 
	    {
		errcode = bad_address;
		return false;
	    }
	    memcpy(&u.sock_inet.sin_addr, hp->h_addr, 
		   sizeof u.sock_inet.sin_addr);
	} else {
	    u.sock_inet.sin_addr.s_addr = htonl(INADDR_ANY);
	}
	u.sock_inet.sin_port = htons(port);
	len = sizeof(sockaddr_in);	
    } 
    if ((fd = socket(u.sock.sa_family, SOCK_STREAM, 0)) < 0) { 
	errcode = errno;
	return false;
    }
    int on = 1;
    setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, (char*)&on, sizeof on);

    if (bind(fd, &u.sock, len) < 0) {
	errcode = errno;
	::close(fd);
	return false;
    }
    if (listen(fd, listen_queue_size) < 0) {
	errcode = errno;
	::close(fd);
	return false;
    }
    errcode = ok;
    state = ss_open;
    return true;
}

char* unix_socket::get_peer_name()
{
    if (state != ss_open) { 
	errcode = not_opened;
	return NULL;
    }
    struct sockaddr_in insock;
#if defined(__linux__) || (defined(__FreeBSD__) && __FreeBSD__ > 3) || defined(__svr4__) || defined(__sun)
    socklen_t len = sizeof(insock);
#elif defined(_AIX)
    size_t len = sizeof(insock);
#else
    int len = sizeof(insock);
#endif
    if (getpeername(fd, (struct sockaddr*)&insock, &len) != 0) { 
	errcode = errno;
	return NULL;
    }
    char* addr = inet_ntoa(insock.sin_addr);
    if (addr == NULL) { 
	errcode = errno;
	return NULL;
    }
    char* addr_copy = new char[strlen(addr)+1];
    strcpy(addr_copy, addr);
    errcode = ok;
    return addr_copy;
}

bool  unix_socket::is_ok()
{
    return errcode == ok;
}

void unix_socket::get_error_text(char* buf, size_t buf_size)
{
    const char* msg; 
    switch(errcode) { 
      case ok:
        msg = "ok";
	break;
      case not_opened:
	msg = "socket not opened";
	break;
      case bad_address: 
	msg = "bad address";
        break;
      case connection_failed: 
	msg = "exceed limit of attempts of connection to server";
	break;
      case broken_pipe:
	msg = "connection is broken";
	break; 
      case invalid_access_mode:
        msg = "invalid access mode";
	break;
      default: 
	msg = strerror(errcode);
    }
    strncpy(buf, msg, buf_size);
}

socket_t* unix_socket::accept()
{
    int s;

    if (state != ss_open) { 
	errcode = not_opened;
	return NULL;
    }

    while((s = ::accept(fd, NULL, NULL )) < 0 && errno == EINTR);

    if (s < 0) { 
	errcode = errno;
	return NULL;
    } else if (state != ss_open) {
	errcode = not_opened;
	return NULL;
    } else { 
	static struct linger l = {1, LINGER_TIME};
#if SOCK_NO_DELAY
	if (domain == sock_global_domain) { 
	    int enabled = 1;
	    if (setsockopt(s, IPPROTO_TCP, TCP_NODELAY, (char*)&enabled, 
			   sizeof enabled) != 0)
	    {
		errcode = errno;
		::close(s);	
		return NULL;
	    }
	}
#endif
	if (setsockopt(s, SOL_SOCKET, SO_LINGER, (char*)&l, sizeof l) != 0) { 
	    errcode = invalid_access_mode; 
	    ::close(s);
	    return NULL; 
	}
#if SOCK_SNDBUF_SIZE > 0
	int size = SOCK_SNDBUF_SIZE;
	setsockopt(s, SOL_SOCKET, SO_SNDBUF, (char*)&size, sizeof size);
#endif
	errcode = ok;
	return new unix_socket(s); 
    }
}

bool unix_socket::cancel_accept() 
{
    bool result = close();
    // Wakeup listener
    delete socket_t::connect(address, domain, 1, 0);
    return result;
}    


bool unix_socket::connect(int max_attempts, time_t timeout)
{
    int   rc;
    char* p;
    struct utsname local_host;
    char hostname[MAX_HOST_NAME];
    unsigned short port;

    assert(address != NULL);

    if ((p = strchr(address, ':')) == NULL 
	|| unsigned(p - address) >= sizeof(hostname) 
	|| sscanf(p+1, "%hu", &port) != 1) 
    {
	errcode = bad_address;
	return false;
    }
    memcpy(hostname, address, p - address);
    hostname[p - address] = '\0';
    
    create_file = false; 
    uname(&local_host);

    if (domain == sock_local_domain || (domain == sock_any_domain && 
	(strcmp(hostname, local_host.nodename) == 0
	 || strcmp(hostname, "localhost") == 0)))
    {
	// connect UNIX socket
	union { 
	    sockaddr sock;
	    char     name[MAX_HOST_NAME];
	} u;
	u.sock.sa_family = AF_UNIX;

	assert(strlen(unix_socket_dir) + strlen(address) 
	       < MAX_HOST_NAME - offsetof(sockaddr,sa_data)); 
 
	int len = offsetof(sockaddr,sa_data) +
	    sprintf(u.sock.sa_data, "%s%s", unix_socket_dir, address);
	
	while (true) {
	    if ((fd = socket(u.sock.sa_family, SOCK_STREAM, 0)) < 0) { 
		errcode = errno;
		return false;
	    }
	    do { 
		rc = ::connect(fd, &u.sock, len);
	    } while (rc < 0 && errno == EINTR);
	    
	    if (rc < 0) { 
		errcode = errno;
		::close(fd);
		if (errcode == ENOENT || errcode == ECONNREFUSED) {
		    if (--max_attempts > 0) { 
			sleep(timeout);
		    } else { 
			break;
		    }
		} else {
		    return false;
		}
	    } else {
		errcode = ok;
		state = ss_open;
		return true;
	    }
	}
    } else { 
	sockaddr_in sock_inet;
	struct hostent* hp;  // entry in hosts table

	if ((hp=gethostbyname(hostname)) == NULL || hp->h_addrtype != AF_INET)
	{
	    errcode = bad_address;
	    return false;
	}
	sock_inet.sin_family = AF_INET;  
	sock_inet.sin_port = htons(port);
	
	while (true) {
	    for (int i = 0; hp->h_addr_list[i] != NULL; i++) { 
		memcpy(&sock_inet.sin_addr, hp->h_addr_list[i],
		       sizeof sock_inet.sin_addr);
		if ((fd = socket(sock_inet.sin_family, SOCK_STREAM, 0)) < 0) { 
		    errcode = errno;
		    return false;
		}
		do { 
		    rc = ::connect(fd,(sockaddr*)&sock_inet,sizeof(sock_inet));
		} while (rc < 0 && errno == EINTR);
		
		if (rc < 0) { 
		    errcode = errno;
		    ::close(fd);
		    if (errcode != ENOENT && errcode != ECONNREFUSED) {
			return false;
		    }
		} else {
		    int enabled = 1;
		    if (setsockopt(fd, IPPROTO_TCP, TCP_NODELAY, 
				   (char*)&enabled, sizeof enabled) != 0)
		    {
			errcode = errno;
			::close(fd);	
			return false;
		    }
		    errcode = ok;
		    state = ss_open;
		    return true;
		}
	    }
	    if (--max_attempts > 0) { 
		sleep(timeout);
	    } else { 
		break;
	    }
	}
    }
    errcode = connection_failed;
    return false;
}

int unix_socket::read(void* buf, size_t min_size, size_t max_size, 
		      time_t timeout)
{ 
    size_t size = 0;
    time_t start = 0;
    if (state != ss_open) { 
	errcode = not_opened;
	return -1;
    }
    if (timeout != WAIT_FOREVER) { 
	start = time(NULL); 
    }
    do { 
	ssize_t rc; 
	if (timeout != WAIT_FOREVER) { 
	    fd_set events;
	    struct timeval tm;
	    FD_ZERO(&events);
	    FD_SET(fd, &events);
	    tm.tv_sec = timeout;
	    tm.tv_usec = 0;
	    while ((rc = select(fd+1, &events, NULL, NULL, &tm)) < 0 
		   && errno == EINTR);
	    if (rc < 0) { 
		errcode = errno;
		return -1;
	    }
	    if (rc == 0) {
		return size;
	    }
	    time_t now = time(NULL);
	    timeout = start + timeout >= now ? timeout + start - now : 0;  
	}
	while ((rc = ::read(fd, (char*)buf + size, max_size - size)) < 0 
	       && errno == EINTR); 
	if (rc < 0) { 
	    errcode = errno;
	    return -1;
	} else if (rc == 0) {
	    errcode = broken_pipe;
	    return -1; 
	} else { 
	    size += rc; 
	}
    } while (size < min_size); 

    return (int)size;
}

	
bool unix_socket::write(void const* buf, size_t size)
{ 
    if (state != ss_open) { 
	errcode = not_opened;
	return false;
    }
    
    do { 
	ssize_t rc; 
	while ((rc = ::write(fd, buf, size)) < 0 && errno == EINTR); 
	if (rc < 0) { 
	    errcode = errno;
	    return false;
	} else if (rc == 0) {
	    errcode = broken_pipe;
	    return false; 
	} else { 
	    buf = (char*)buf + rc; 
	    size -= rc; 
	}
    } while (size != 0); 

    //
    // errcode is not assigned 'ok' value beacuse write function 
    // can be called in parallel with other socket operations, so
    // we want to preserve old error code here.
    //
    return true;
}
	
bool unix_socket::close()
{
    if (state != ss_close) {
	state = ss_close;
	if (::close(fd) == 0) {
	    errcode = ok;
            return true;
	} else { 
	    errcode = errno;
	    return false;
	}
    }
    errcode = ok;
    return true;
}

bool unix_socket::shutdown()
{
    if (state == ss_open) { 
	state = ss_shutdown;
	int rc = ::shutdown(fd, 2);
	if (rc != 0) { 
	    errcode = errno;
	    return false;
	} 
    } 
    return true;
}

unix_socket::~unix_socket()
{
    close();
    if (create_file) { 
	char name[MAX_HOST_NAME];
	sprintf(name, "%s%s", unix_socket_dir, address);
	unlink(name);
    }
    delete[] address;
}

unix_socket::unix_socket(const char* addr, socket_domain domain)
{ 
    address = new char[strlen(addr)+1]; 
    strcpy(address, addr);
    this->domain = domain;
    create_file = false;
    errcode = ok;
}

unix_socket::unix_socket(int new_fd) 
{ 
    fd = new_fd; 
    address = NULL; 
    create_file = false;
    state = ss_open; 
    errcode = ok;
}

socket_t* socket_t::create_local(char const* address, int listen_queue_size)
{
    unix_socket* sock = new unix_socket(address, sock_local_domain);
    sock->open(listen_queue_size); 
    return sock;
}

socket_t* socket_t::create_global(char const* address, int listen_queue_size)
{
    unix_socket* sock = new unix_socket(address, sock_global_domain);
    sock->open(listen_queue_size); 
    return sock;
}

socket_t* socket_t::connect(char const* address, 
			    socket_domain domain, 
			    int max_attempts, 
			    time_t timeout)
{
    unix_socket* sock = new unix_socket(address, domain);
    sock->connect(max_attempts, timeout); 
    return sock;
}

int unix_socket::get_handle()
{
    return fd;
}
