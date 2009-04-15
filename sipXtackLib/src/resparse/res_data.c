/*
 * Copyright (c) 1995,1996 by Internet Software Consortium.
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND INTERNET SOFTWARE CONSORTIUM DISCLAIMS
 * ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL INTERNET SOFTWARE
 * CONSORTIUM BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL
 * DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR
 * PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS
 * ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
 * SOFTWARE.
 */

#if defined(LIBC_SCCS) && !defined(lint)
static char rcsid[] = "";
#endif /* LIBC_SCCS and not lint */

#include <sys/types.h>
#include <time.h>

/* Reordered includes and separated into win/vx --GAT */
#if defined(_WIN32)
#       include <resparse/wnt/sys/param.h>
#       include <winsock.h>
#       include <resparse/wnt/netinet/in.h>
#       include <resparse/wnt/arpa/inet.h>
#       include <resparse/wnt/arpa/nameser.h>
#       include <resparse/wnt/resolv/resolv.h>
#elif defined(_VXWORKS)
#       include <sys/socket.h>
#       include <netinet/in.h>
#       include <arpa/inet.h>
/* Use local lnameser.h for info missing from VxWorks version --GAT */
/* lnameser.h is a subset of resparse/wnt/arpa/nameser.h                */
#       include <resolv/nameser.h>
#       include <resparse/vxw/arpa/lnameser.h>
/* Use local lresolv.h for info missing from VxWorks version --GAT */
/* lresolv.h is a subset of resparse/wnt/resolv/resolv.h               */
#       include <resolv/resolv.h>
#       include <resparse/vxw/resolv/lresolv.h>
#       include <unistd.h>
#endif
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "resparse/res_config.h"

#ifndef __pingtel_on_posix__
const char *_res_opcodes[] = {
        "QUERY",
        "IQUERY",
        "CQUERYM",
        "CQUERYU",      /* experimental */
        "NOTIFY",       /* experimental */
        "UPDATE",
        "6",
        "7",
        "8",
        "9",
        "10",
        "11",
        "12",
        "13",
        "ZONEINIT",
        "ZONEREF",
};
#endif

const char *_res_resultcodes[] = {
        "NOERROR",
        "FORMERR",
        "SERVFAIL",
        "NXDOMAIN",
        "NOTIMP",
        "REFUSED",
        "YXDOMAIN",
        "YXRRSET",
        "NXRRSET",
        "NOTAUTH",
        "ZONEERR",
        "11",
        "12",
        "13",
        "14",
        "NOCHANGE",
};

#ifdef BIND_UPDATE
const char *_res_sectioncodes[] = {
        "ZONE",
        "PREREQUISITES",
        "UPDATE",
        "ADDITIONAL",
};
#endif
