#include <stdio.h>
/*
 *  res_parse.c
 *
 *  Parse the result of a DNS query
 *      into an accessible structure
 *
 *  v1.0
 *
 *  Dave Shield         January 1994
 *
 *  v1.1        Cast all malloc calls
 *              Cleaner handling of header
 *                      February 1994
 *
 *  v1.2        Fixed incorrect calls to free_response
 *              Use memcpy rather than bcopy
 *                      November 1995
 *
 *  v1.3        Allocate response dynamically
 *              Handle network/host ordering
 *                      July 1997
 */

#include "resparse/rr.h"
#ifdef __pingtel_on_posix__
#include <resolv.h>
/* These are a cheezy hack from ns_netint.c */
#define _getshort _pingtel_getshort
#define _getlong _pingtel_getlong

uint16_t _getshort(const u_char *src);
uint32_t _getlong(const u_char *src);
char * expand_cdname(char **cpp, char *msg);
char * expand_charstring(char **cpp, char *msg);
s_question * parse_question(char **cpp, char *msg);
s_rr * parse_rr( char **cpp, char *msg);

uint16_t _getshort(const u_char *src)
{
        uint16_t dst;
        NS_GET16(dst, src);
        return dst;
}
uint32_t _getlong(const u_char *src)
{
        uint32_t dst;
        NS_GET32(dst, src);
        return dst;
}
#endif



extern int dn_expand(const u_char *a,
                        const u_char *b, const u_char *c, char *d, int e);




        /*
         *  expand_cdname:
         *      Extract and uncompress a domain name
         *
         *  returns a pointer to the domain name (or NULL on failure).
         */
char *
expand_cdname(
        char    **cpp,
        char    *msg)
{
        char    name[MAXDNAME];
        int     i;
        char    *ptr;

        if ((i = dn_expand((const u_char *) msg,
                           (const u_char *) msg+512,
                           (const u_char *) *cpp,
                           name, MAXDNAME)) < 0)
                return(NULL);
        if (name[0] == '\0' ) {
                name[0] = '.';
                name[1] = '\0';
        }
        *cpp += i;      /* Step over the name */

        if ((ptr=(char *)malloc(strlen(name)+1)) == NULL ) {
                        /* Failed - undo everything */
                *cpp -= i;
                return(NULL);
        }
        strcpy(ptr, name);
        return(ptr);
}


        /*
         *  expand_charstring:
         *      Extract a character string
         *
         *  returns a pointer to the string (or NULL on failure).
         */
char *
expand_charstring(
        char    **cpp,
        char    *msg)
{
        int     i;
        char    *ptr;

        i = *(unsigned char*)*cpp;      /* Extract length of string */
        (*cpp)++;       /*  and move on */

        if ((ptr=(char *)malloc(i+1)) == NULL ) {
                        /* Failed - undo everything */
                (*cpp)--;
                return(NULL);
        }
        strncpy(ptr, *cpp, i);
        ptr[i] = '\0';
        *cpp += i;

        return(ptr);
}


        /*
         *  parse_question:
         *      Extract and parse a question record
         *
         *  returns a pointer to the question (or NULL on failure).
         */
s_question *
parse_question(
        char    **cpp,
        char    *msg)
{
        s_question      *ptr;

        if ((ptr = (s_question *)malloc(sizeof(s_question))) == NULL )
                return(NULL);
        if ((ptr->qname = expand_cdname(cpp,msg)) == NULL ) {
                free( ptr );
                return(NULL);
        }
        ptr->qtype = _getshort((const u_char *) *cpp);
        *cpp += sizeof(uint16_t);
        ptr->qclass = _getshort((const u_char *) *cpp);
        *cpp += sizeof(uint16_t);

        return(ptr);
}



        /*
         *  parse_rr:
         *      Extract and parse a resource record
         *
         *  returns a pointer to the RR (or NULL on failure).
         */
s_rr *
parse_rr(
        char    **cpp,
        char    *msg)
{
        s_rr            *ptr;
        int             dlen;
        union u_rdata   *rd;


                /*
                 *  Set up the RR-independent information
                 */
        if ((ptr = (s_rr *)malloc(sizeof(s_rr))) == NULL )
                return(NULL);
        if ((ptr->name = expand_cdname(cpp,msg)) == NULL ) {
                free(ptr);
                return(NULL);
        }
        ptr->type = _getshort((const u_char *) *cpp);
        *cpp += sizeof(uint16_t);
        ptr->rclass = _getshort((const u_char *) *cpp);
        *cpp += sizeof(uint16_t);
        ptr->ttl = _getlong((const u_char *) *cpp);
        *cpp += sizeof(uint32_t);

        dlen = _getshort((const u_char *) *cpp);
        ptr->dlen = dlen;
        *cpp += sizeof(uint16_t);

        rd = &ptr->rdata;


                /*
                 *  Handle RR-specifics
                 *
                 *    (No indication of failures here is
                 *      passed back to the calling procedure)
                 */
        switch(ptr->type) {
        case T_A:                               /* Address */
                switch (ptr->rclass) {
                case C_IN:
                        memcpy((void *)&rd->address, (void *)*cpp, sizeof(struct in_addr));
                        *cpp += dlen;
                        break;

                default:
                        /* Can't really handle this - just skip it */
                        *cpp += dlen;
                }
                break;

        case T_NS:                              /* Name Server */
        case T_MD:                              /* Mail Destination (OBS) */
        case T_MF:                              /* Mail Forwarder   (OBS) */
        case T_CNAME:                           /* Canonical Name */
                rd->string = expand_cdname(cpp, msg);
                break;

        case T_SOA:                             /* Start of Authority */
                rd->soa.mname = expand_cdname(cpp, msg);
                rd->soa.rname = expand_cdname(cpp, msg);
                rd->soa.serial = _getlong((const u_char *) *cpp);
                *cpp += sizeof(uint32_t);
                rd->soa.refresh = _getlong((const u_char *) *cpp);
                *cpp += sizeof(uint32_t);
                rd->soa.retry = _getlong((const u_char *) *cpp);
                *cpp += sizeof(uint32_t);
                rd->soa.expire = _getlong((const u_char *) *cpp);
                *cpp += sizeof(uint32_t);
                rd->soa.minimum = _getlong((const u_char *) *cpp);
                *cpp += sizeof(uint32_t);
                break;

        case T_MB:                              /* Mail Box  */
        case T_MG:                              /* Mail Group */
        case T_MR:                              /* Mail Rename */
                rd->string = expand_cdname(cpp, msg);
                break;

/* Following modification taken from VxWorks --GAT */
/* 01b,29apr97,jag Changed T_NULL to T_NULL_RR to fix conflict with loadCoffLib.h */
        case T_NULL_RR:                         /* Null RR */
                if ((rd->null.anything = (char *)malloc(dlen)) != NULL )
                        memcpy((void *)rd->null.anything, (void *)*cpp, dlen);
                rd->null.length = dlen;
                *cpp += dlen;
                break;

        case T_WKS:                             /* Well Known Services */
                memcpy((void *)&rd->wks.address, (void *)*cpp, sizeof(struct in_addr));
                *cpp += sizeof(struct in_addr);
                rd->wks.protocol = **cpp;
                (*cpp)++;
                rd->wks.maplength = dlen-(sizeof(struct in_addr) +1);
                if ((rd->wks.bitmap = (char *)malloc(rd->wks.maplength)) != NULL )
                        memcpy((void *)rd->wks.bitmap, (void *)*cpp, rd->wks.maplength);
                *cpp += rd->wks.maplength;
                break;

        case T_PTR:                             /* Domain Name Pointer */
                rd->string = expand_cdname(cpp, msg);
                break;

        case T_HINFO:                           /* Host Info */
                if (( rd->hinfo.cpu = expand_charstring(cpp, msg)) == NULL ) {
                        cpp += dlen;
                        break;
                }
                if (( rd->hinfo.os = expand_charstring(cpp, msg)) == NULL ) {
                        cpp -= (strlen(rd->hinfo.cpu) +1);
                        cpp += dlen;
                        break;
                }
                break;

        case T_MINFO:                           /* Mailbox Info */
                rd->minfo.rmailbx = expand_cdname(cpp, msg);
                rd->minfo.emailbx = expand_cdname(cpp, msg);
                break;

        case T_MX:                              /* Mail Exchanger */
                rd->mx.preference = _getshort((const u_char *) *cpp);
                *cpp += sizeof(uint16_t);
                rd->mx.exchange = expand_cdname(cpp, msg);
                break;

        case T_SRV:                             /* Service location */
                rd->srv.priority = _getshort((const u_char *) *cpp);
                *cpp += sizeof(uint16_t);
                rd->srv.weight = _getshort((const u_char *) *cpp);
                *cpp += sizeof(uint16_t);
                rd->srv.port = _getshort((const u_char *) *cpp);
                *cpp += sizeof(uint16_t);
                rd->srv.target = expand_cdname(cpp, msg);
                break;

        case T_NAPTR:                           /* Naming authority pointer */
        {
                // In case we have to abort parsing later, clear all the pointer fields.
                rd->naptr.flags = NULL;
                rd->naptr.services = NULL;
                rd->naptr.regexp = NULL;
                rd->naptr.replacement = NULL;
                // This is copied from the other cases, but I think that
                // all cases should be setting *cpp += dlen, not
                // cpp += dlen.
                char **cpp_end = cpp + dlen;
                rd->naptr.order = _getshort((const u_char *) *cpp);
                *cpp += sizeof(uint16_t);
                rd->naptr.preference = _getshort((const u_char *) *cpp);
                *cpp += sizeof(uint16_t);
                if (( rd->naptr.flags = expand_charstring(cpp, msg)) == NULL ) {
                        cpp = cpp_end;
                        break;
                }
                if (( rd->naptr.services = expand_charstring(cpp, msg)) == NULL ) {
                        cpp = cpp_end;
                        break;
                }
                if (( rd->naptr.regexp = expand_charstring(cpp, msg)) == NULL ) {
                        cpp = cpp_end;
                        break;
                }
                rd->naptr.replacement = expand_cdname(cpp, msg);
        }
                break;

        case T_TXT:                             /* Text string */
                rd->txt.len = **cpp;
                rd->txt.next = NULL;
                if ((rd->txt.text = expand_charstring(cpp, msg)) == NULL ) {
                        cpp += dlen;
                        break;
                }

                        /*
                         *  Multiple strings
                         *      add onto a linked list
                         */
                if ( rd->txt.len+1 < dlen ) {
                        struct s_TXT    *txtp;
                        char            **cpp2;
                        int             n;

                        n = rd->txt.len+1;
                        txtp = &(rd->txt);
                        cpp2 = cpp;
                        while ( n < dlen ) {
                                if ((txtp->next = (struct s_TXT *)malloc(sizeof(struct s_TXT))) == NULL ) {
                                        cpp += (dlen - rd->txt.len);
                                        break;
                                }
                                txtp = txtp->next;
                                txtp->len = **cpp2;
                                n += txtp->len+1;
                                txtp->next = NULL;
                                if ((txtp->text = expand_charstring(cpp2, msg)) == NULL ) {
                                        cpp += (dlen - rd->txt.len);
                                        break;
                                }
                        }
                }
                break;


                        /*
                         *  RFC 1183  Additional types
                         */
        case T_AFSDB:                           /* AFS Server */
                rd->afsdb.subtype = _getshort((const u_char *) *cpp);
                *cpp += sizeof(uint16_t);
                rd->afsdb.hostname = expand_cdname(cpp, msg);
                break;


        case T_RP:                              /* Responsible Person */
                rd->rp.mbox_dname = expand_cdname(cpp, msg);
                rd->rp.txt_dname = expand_cdname(cpp, msg);
                break;

        case T_X25:                             /* X25 Address */
                rd->string = expand_charstring(cpp, msg);
                break;

        case T_ISDN:                            /* ISDN Address */
                if ( **cpp == dlen ) {
                        rd->isdn.address = expand_charstring(cpp, msg);
                        rd->isdn.sa = NULL;
                }
                else {
                        rd->isdn.address = expand_charstring(cpp, msg);
                        rd->isdn.sa = expand_charstring(cpp, msg);
                }
                break;

        case T_RT:                              /* Route Through */
                rd->rt.preference = _getshort((const u_char *) *cpp);
                *cpp += sizeof(uint16_t);
                rd->rt.int_host = expand_cdname(cpp, msg);
                break;

                        /*
                         *  Additional Non-standard types
                         */
        case T_UINFO:                           /* User (finger) info */
                if ((rd->string = (char *)malloc(dlen+1)) != NULL ) {
                        memcpy((void *)rd->string, (void *)*cpp, dlen);
                        rd->string[dlen] = '\0';
                }
                *cpp += dlen;
                break;

        case T_UID:                             /* User ID */
        case T_GID:                             /* Group ID */
                rd->number = _getlong((const u_char *) *cpp);
                *cpp += sizeof(uint32_t);
                break;

        case T_UNSPEC:                          /* Unspecified info */
        default:                                /* Unrecognised */
                if ((rd->string = (char *)malloc(dlen+1)) != NULL )
                        memcpy((void *)rd->string, (void *)*cpp, dlen);
                *cpp += dlen;
                break;

        }


        return(ptr);
}




        /*
         *  res_parse:
         *      Parse a DNS response buffer
         *
         *  returns a pointer to the expanded tree (or NULL on failure).
         */
res_response *
res_parse(char *msg)
{
        char *cp;
        HEADER *hp;
        res_response *resp;
        int i;

        uint16_t qdcount, ancount, nscount, arcount;


        /*
         * Set up the response structure,
         *  and copy across the header fields.
         */
        if ((resp = (res_response *)malloc(sizeof(res_response))) == NULL )
                return(NULL);
        memcpy((void *)&(resp->header), (void *)msg, sizeof(HEADER));
        hp = &(resp->header);
        cp = msg + sizeof(HEADER);


        /*
         * Temporarily clear the number of records to expect
         *    (allows the tree to be freed in case of problems)
         * Also handle network/host ordering
         */
        qdcount = ntohs((uint16_t)resp->header.qdcount); resp->header.qdcount = 0;
        ancount = ntohs((uint16_t)resp->header.ancount); resp->header.ancount = 0;
        nscount = ntohs((uint16_t)resp->header.nscount); resp->header.nscount = 0;
        arcount = ntohs((uint16_t)resp->header.arcount); resp->header.arcount = 0;
        resp->question = NULL;
        resp->answer = NULL;
        resp->authority = NULL;
        resp->additional = NULL;

        /*
         * Handle question records.
         */
        if ( qdcount ) {
                if ((resp->question = (s_question **)malloc(qdcount*sizeof(s_question*))) == NULL )
                        return(NULL);
                for ( i=0 ; i<qdcount ; i++ )           /* Clear, in case of free */
                        resp->question[i] = NULL;
                resp->header.qdcount = qdcount;  /* Stores swapped byte order!  Requires change to free_response. --GAT */
                for ( i=0 ; i<qdcount ; i++ )
                        if ((resp->question[i] = parse_question(&cp, msg)) == NULL ) {
                                free_response(resp);
                                free(resp);
                                return(NULL);
                        }
        }


        /*
         * Handle authoritative answer records
         */
        if ( ancount ) {
                if ((resp->answer = (s_rr **)malloc(ancount*sizeof(s_rr*))) == NULL ) {
                        resp->header.ancount = 0;
                        free_response(resp);
                        free(resp);
                        return(NULL);
                }
                for ( i=0 ; i<ancount ; i++ )
                        resp->answer[i] = NULL;
                resp->header.ancount = ancount;  /* Stores swapped byte order!  Requires change to free_response. --GAT */
                for ( i=0 ; i<ancount ; i++ )
                        if ((resp->answer[i] = parse_rr(&cp, msg)) == NULL ) {
                                free_response(resp);
                                free(resp);
                                return(NULL);
                        }
        }


        /*
         * Handle name server records
         */
        if ( nscount ) {
                if ((resp->authority = (s_rr **)malloc(nscount*sizeof(s_rr*))) == NULL ) {
                        resp->header.nscount = 0;
                        free_response(resp);
                        free(resp);
                        return(NULL);
                }
                for ( i=0 ; i<nscount ; i++ )
                        resp->authority[i] = NULL;
                resp->header.nscount = nscount;  /* Stores swapped byte order!  Requires change to free_response. --GAT */
                for ( i=0 ; i<nscount ; i++ )
                        if ((resp->authority[i] = parse_rr(&cp, msg)) == NULL ) {
                                free_response(resp);
                                free(resp);
                                return(NULL);
                        }
        }


        /*
         * Handle additional records
         */
        if ( arcount ) {
                if ((resp->additional = (s_rr **)malloc(arcount*sizeof(s_rr*))) == NULL ) {
                        resp->header.arcount = 0;
                        free_response(resp);
                        free(resp);
                        return(NULL);
                }
                for ( i=0 ; i<arcount ; i++ )
                        resp->additional[i] = NULL;
                resp->header.arcount = arcount;  /* Stores swapped byte order!  Requires change to free_response. --GAT */
                for ( i=0 ; i<arcount ; i++ )
                        if ((resp->additional[i] = parse_rr(&cp, msg)) == NULL ) {
                                free_response(resp);
                                free(resp);
                                return(NULL);
                        }
        }

        return(resp);
}
