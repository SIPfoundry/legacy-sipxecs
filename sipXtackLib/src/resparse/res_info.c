/*
 *  res_info.c
 *
 *  Utility routines,
 *      to translate various enumerated types
 *      into suitable strings (and similar)
 *
 *  v1.0
 *
 *  Dave Shield         January 1994
 *
 *  v1.1        Hide unneeded strings
 *              Added missing RFC 1183 types
 *                and restructured type routines
 *                      February 1994
 *
 *  v1.3        Change name of error string routine
 *                      July 1997
 */

#include        "resparse/rr.h"
#include        "resparse/res_info.h"

#if defined (_VXWORKS) /* required for compile --GAT */
#       include <stdlib.h>
#endif

/* #define h_reserrno WSAGetLastError() */

int h_reserrno;
static char     retbuf[40];

const char *_res_errors[] = {
        "Error 0",
        "Unknown host",
        "Host name lookup failure",
        "Unknown server error",
        "No address associated with name",
        "Unknown error",
};

const char* res_error_str()
{
        int i;
        i = h_reserrno;

        return(_res_errors[h_reserrno]);
}

#ifdef NEED_RES_OPCODES
extern const char *_res_opcodes[] = {
        "QUERY",
        "IQUERY",
        "CQUERYM",
        "CQUERYU",
        "4",
        "5",
        "6",
        "7",
        "8",
        "UPDATEA",
        "UPDATED",
        "UPDATEDA",
        "UPDATEM",
        "UPDATEMA",
        "ZONEINIT",
        "ZONEREF",
};
#else
extern const char *_res_opcodes[];
#endif

const char *
res_opcode(i)
        int i;
{
        return(_res_opcodes[i]);
}



#ifdef NEED_RES_RESULTCODES
const char *_res_resultcodes[] = {
        "NOERROR",
        "FORMERR",
        "SERVFAIL",
        "NXDOMAIN",
        "NOTIMP",
        "REFUSED",
        "6",
        "7",
        "8",
        "9",
        "10",
        "11",
        "12",
        "13",
        "14",
        "NOCHANGE",
};
#else
extern const char *_res_resultcodes[];
#endif

const char *
res_rcode(int i)
{
        return(_res_resultcodes[i]);
}




const char *
res_wks(int wks)
{
        switch (wks) {
        case 5: return("rje");
        case 7: return("echo");
        case 9: return("discard");
        case 11: return("systat");
        case 13: return("daytime");
        case 15: return("netstat");
        case 17: return("qotd");
        case 19: return("chargen");
        case 20: return("ftp-data");
        case 21: return("ftp");
        case 23: return("telnet");
        case 25: return("smtp");
        case 37: return("time");
        case 39: return("rlp");
        case 42: return("name");
        case 43: return("whois");
        case 53: return("domain");
        case 57: return("apts");
        case 59: return("apfs");
        case 67: return("bootps");
        case 68: return("bootpc");
        case 69: return("tftp");
        case 77: return("rje");
        case 79: return("finger");
        case 87: return("link");
        case 95: return("supdup");
        case 100: return("newacct");
        case 101: return("hostnames");
        case 102: return("iso-tsap");
        case 103: return("x400");
        case 104: return("x400-snd");
        case 105: return("csnet-ns");
        case 109: return("pop-2");
        case 111: return("sunrpc");
        case 113: return("auth");
        case 115: return("sftp");
        case 117: return("uucp-path");
        case 119: return("nntp");
        case 121: return("erpc");
        case 123: return("ntp");
        case 133: return("statsrv");
        case 136: return("profile");
        case 144: return("NeWS");
        case 161: return("snmp");
        case 162: return("snmp-trap");
        case 170: return("print-srv");
        default: (void) sprintf(retbuf, "%d", wks); return(retbuf);
        }
}


const char *
res_proto(int proto)
{
        switch (proto) {
        case 1: return("icmp");
        case 2: return("igmp");
        case 3: return("ggp");
        case 5: return("st");
        case 6: return("tcp");
        case 7: return("ucl");
        case 8: return("egp");
        case 9: return("igp");
        case 11: return("nvp-II");
        case 12: return("pup");
        case 16: return("chaos");
        case 17: return("udp");
        default: (void) sprintf(retbuf, "%d", proto); return(retbuf);
        }
}


const char *
res_type(int type)
{
        switch (type) {
                        /* RFC 1035 types */
        case T_A:
                return("A");
        case T_CNAME:           /* canonical name */
                return("CNAME");
        case T_HINFO:           /* host information */
                return("HINFO");
        case T_MB:              /* mailbox domain name */
                return("MB");
        case T_MG:              /* mail group member */
                return("MG");
        case T_MINFO:           /* mailbox information */
                return("MINFO");
        case T_MR:              /* mail rename name */
                return("MR");
        case T_MX:              /* mail routing info */
                return("MX");
        case T_NS:              /* authoritative server */
                return("NS");
/* Following modification taken from VxWorks --GAT */
/* 01b,29apr97,jag Changed T_NULL to T_NULL_RR to fix conflict with loadCoffLib.h */
        case T_NULL_RR:         /* null resource record */
                return("NULL");
        case T_PTR:             /* domain name pointer */
                return("PTR");
        case T_SOA:             /* start of authority zone */
                return("SOA");
        case T_SRV:             /* service */
                return("SRV");
        case T_NAPTR:           /* naming authority pointer */
                return("NAPTR");
        case T_TXT:             /* text */
                return("TXT");
        case T_WKS:             /* well known service */
                return("WKS");

                        /* RFC 1183 types */
        case T_AFSDB:           /* AFS database server */
                return("AFSDB");
        case T_ISDN:            /* ISDN address */
                return("ISDN");
        case T_RP:              /* responsible person */
                return("RP");
        case T_RT:              /* route through */
                return("RT");
        case T_X25:             /* X25 address */
                return("X25");

                        /* Question types */
        case T_AXFR:            /* zone transfer */
                return("AXFR");
        case T_MAILB:           /* mail box */
                return("MAILB");
        case T_MAILA:           /* mail address */
                return("MAILA");

                        /* Other types */
        case T_ANY:             /* matches any type */
                return("ANY");
        case T_UINFO:
                return("UINFO");
        case T_UID:
                return("UID");
        case T_GID:
                return("GID");
        case T_UNSPEC:
                return("UNSPEC");
        default:
                (void)sprintf(retbuf, "%d", type);
                return(retbuf);
        }
}


int
which_res_type(const char* str)
{

#ifdef  __cplusplus
extern "C" {
#endif

        int i;

        if ((i = atoi(str)) != 0 )
                return(i);

                        /* RFC 1035 types */
        if ( !strcmp( str, "A" ))
                return(T_A);
        if ( !strcmp( str, "CNAME" ))
                return(T_CNAME);
        if ( !strcmp( str, "HINFO" ))
                return(T_HINFO);
        if ( !strcmp( str, "MB" ))
                return(T_MB);
        if ( !strcmp( str, "MG" ))
                return(T_MG);
        if ( !strcmp( str, "MINFO" ))
                return(T_MINFO);
        if ( !strcmp( str, "MR" ))
                return(T_MR);
        if ( !strcmp( str, "MX" ))
                return(T_MX);
        if ( !strcmp( str, "NS" ))
                return(T_NS);
/* Following modification taken from VxWorks --GAT */
/* 01b,29apr97,jag Changed T_NULL to T_NULL_RR to fix conflict with loadCoffLib.h */
        if ( !strcmp( str, "NULL" ))
                return(T_NULL_RR);
        if ( !strcmp( str, "PTR" ))
                return(T_PTR);
        if ( !strcmp( str, "SOA" ))
                return(T_SOA);
        if ( !strcmp( str, "SRV" ))
                return(T_SRV);
        if ( !strcmp( str, "NAPTR" ))
                return(T_NAPTR);
        if ( !strcmp( str, "TXT" ))
                return(T_TXT);
        if ( !strcmp( str, "WKS" ))
                return(T_WKS);

                        /* RFC 1183 types */
        if ( !strcmp( str, "AFSDB" ))
                return(T_AFSDB);
        if ( !strcmp( str, "ISDN" ))
                return(T_ISDN);
        if ( !strcmp( str, "RP" ))
                return(T_RP);
        if ( !strcmp( str, "RT" ))
                return(T_RT);
        if ( !strcmp( str, "X25" ))
                return(T_X25);

                        /* Question types */
        if ( !strcmp( str, "AXFR" ))
                return(T_AXFR);
        if ( !strcmp( str, "MAILB" ))
                return(T_MAILB);
        if ( !strcmp( str, "MAILA" ))
                return(T_MAILA);

                        /* Question types */
        if ( !strcmp( str, "ANY" ))
                return(T_ANY);
        if ( !strcmp( str, "UINFO" ))
                return(T_UINFO);
        if ( !strcmp( str, "UID" ))
                return(T_UID);
        if ( !strcmp( str, "GID" ))
                return(T_GID);
        if ( !strcmp( str, "UNSPEC" ))
                return(T_UNSPEC);

        return(T_ANY);

#ifdef  __cplusplus
}
#endif

}



const char *
res_class(int class)
{

        switch (class) {
        case C_IN:              /* internet class */
                return("IN");
        case C_CHAOS:           /* chaos class */
                return("CHAOS");
#ifdef C_HS
        case C_HS:              /* hesiod class */
                return("HS");
#endif
        case C_ANY:             /* matches any class */
                return("ANY");
        default:
                (void)sprintf(retbuf, "%d", class);
                return(retbuf);
        }
}



const char *
res_time(int value)
{
        int secs, mins, hours, days;
        register char *p;

        if (value == 0) {
                strcpy(retbuf, "0 secs");
                return(retbuf);
        }

        secs = value % 60;
        value /= 60;
        mins = value % 60;
        value /= 60;
        hours = value % 24;
        value /= 24;
        days = value;
        value = 0;

#define PLURALIZE(x)    x, (x == 1) ? "" : "s"
        p = retbuf;
        if (days) {
                (void)sprintf(p, "%d day%s", PLURALIZE(days));
                while (*++p);
        }
        if (hours) {
                if (days)
                        *p++ = ' ';
                (void)sprintf(p, "%d hour%s", PLURALIZE(hours));
                while (*++p);
        }
        if (mins) {
                if (days || hours)
                        *p++ = ' ';
                (void)sprintf(p, "%d min%s", PLURALIZE(mins));
                while (*++p);
        }
        if (secs || ! (days || hours || mins)) {
                if (days || hours || mins)
                        *p++ = ' ';
                (void)sprintf(p, "%d sec%s", PLURALIZE(secs));
        }
        return(retbuf);
}
