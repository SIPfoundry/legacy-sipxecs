/*
 *  res_print.c
 *
 *  Print out a DNS response tree
 *
 *  v1.0
 *
 *  Dave Shield         January 1994
 *
 *  v1.1        Cleaner handling of header pointer
 *                      February 1994
 *
 *  v1.3        Protect non-standard 'primary' header flag
 *                      July 1997
 */

#include <stdio.h>
#include "resparse/rr.h"
#include "resparse/res_info.h"

void print_header(FILE* fp, HEADER *hp, int verbose);
void print_question(FILE *fp, s_question *ptr);
void print_rr(FILE *fp, s_rr *ptr);
void print_response(FILE* fp, res_response* resp);

#define FALSE   0
#define TRUE    1



        /*
         *  print_header:
         *      Print out the header of a DNS response
         */
void
print_header(
        FILE            *fp,
        HEADER          *hp,
        int             verbose)        /* Long or short header flags */
{
        int     comma = FALSE;

        fprintf(fp, "\topcode = %s, id = %d, rcode = %s\n",
                res_opcode(hp->opcode),
                hp->id,
                res_rcode(hp->rcode));

                /* Decode header flags */
        fprintf(fp, "\theader flags: ");
        if (verbose) {
                if (hp->qr) {
                        fprintf(fp, "%s%s", (comma ? ", " : ""), "response");
                        comma = TRUE;
                }
                if (hp->aa) {
                        fprintf(fp, "%s%s", (comma ? ", " : ""), "auth. answer");
                        comma = TRUE;
                }
                if (hp->tc) {
                        fprintf(fp, "%s%s", (comma ? ", " : ""), "truncated");
                        comma = TRUE;
                }
                if (hp->rd) {
                        fprintf(fp, "%s%s", (comma ? ", " : ""), "want recursion");
                        comma = TRUE;
                }
                if (hp->ra) {
                        fprintf(fp, "%s%s", (comma ? ", " : ""), "recursion avail.");
                        comma = TRUE;
                }
#ifdef HEADER_HAS_PR
                        /* Non-standard header flag */
                if (hp->pr) {
                        fprintf(fp, "%s%s", (comma ? ", " : ""), "want primary");
                        comma = TRUE;
                }
#endif
        }
        else {
                if (hp->qr) {
                        fprintf(fp, "%s%s", (comma ? "  " : ""), "qr");
                        comma = TRUE;
                }
                if (hp->aa) {
                        fprintf(fp, "%s%s", (comma ? "  " : ""), "aa");
                        comma = TRUE;
                }
                if (hp->tc) {
                        fprintf(fp, "%s%s", (comma ? "  " : ""), "tr");
                        comma = TRUE;
                }
                if (hp->rd) {
                        fprintf(fp, "%s%s", (comma ? "  " : ""), "rd");
                        comma = TRUE;
                }
                if (hp->ra) {
                        fprintf(fp, "%s%s", (comma ? "  " : ""), "ra");
                        comma = TRUE;
                }
#ifdef HEADER_HAS_PR
                        /* Non-standard header flag */
                if (hp->pr) {
                        fprintf(fp, "%s%s", (comma ? "  " : ""), "pr");
                        comma = TRUE;
                }
#endif
        }
        fprintf(fp, "\n");

                /* Number of records */
        fprintf(fp, "\tquestions = %d,",        hp->qdcount);
        fprintf(fp, " answers = %d,",           hp->ancount);
        fprintf(fp, " authority records = %d,", hp->nscount);
        fprintf(fp, " additional = %d\n",       hp->arcount);
        fprintf(fp, "\n");
}


        /*
         *  print_question:
         *      Print out a question record
         */
void
print_question(
        FILE            *fp,
        s_question      *ptr)
{
        fprintf(fp, "\t%s, type = %s, class = %s\n",
                ptr->qname, res_type(ptr->qtype), res_class(ptr->qclass));
}



        /*
         *  print_rr:
         *      Print out a resource record
         */
void
print_rr(FILE           *fp,
         s_rr           *ptr)
{
        unsigned int    dlen;
        unsigned int    i;
        union u_rdata   *rd;


                /*
                 *  Print the RR-independent information
                 */
        fprintf(fp, "    ->  %s\n", ptr->name);
        fprintf(fp, "\ttype = %s, class = %s, dlen = %d\n",
                res_type(ptr->type), res_class(ptr->rclass), (int)ptr->dlen);
        if ( ptr->type == T_SOA )
                fprintf(fp, "\tttl = %d (%s)\n", (int)ptr->ttl, res_time(ptr->ttl));


        dlen = ptr->dlen;
        rd = &ptr->rdata;


                /*
                 *  Print RR-specifics
                 */
        switch(ptr->type) {
        case T_A:                               /* Address */
                switch (ptr->rclass) {
                case C_IN:
                        fprintf(fp, "\tinternet address = %s\n", inet_ntoa(rd->address));
                        break;
                default:
                        /* Can't really do anything with */
                        fprintf(fp, "\taddress = ???\n");
                }
                break;

        case T_NS:                              /* Name Server */
                fprintf(fp, "\tnameserver = %s\n", rd->string);
                break;

        case T_MD:                              /* Mail Destination (OBS) */
        case T_MF:                              /* Mail Forwarder   (OBS) */
                fprintf(fp, "\tname = %s\n", rd->string);
                break;

        case T_CNAME:                           /* Canonical Name */
                fprintf(fp, "\tcanonical name = %s\n", rd->string);
                break;

        case T_SOA:                             /* Start of Authority */
                fprintf(fp, "\torigin = %s\n", rd->soa.mname);
                fprintf(fp, "\tmail addr = %s\n", rd->soa.rname);
                fprintf(fp, "\tserial = %d\n", (int)rd->soa.serial);
                fprintf(fp, "\trefresh = %d (%s)\n",
                        (int)rd->soa.refresh, res_time(rd->soa.refresh));
                fprintf(fp, "\tretry = %d (%s)\n",
                        (int)rd->soa.retry, res_time(rd->soa.retry));
                fprintf(fp, "\texpire = %d (%s)\n",
                        (int)rd->soa.expire, res_time(rd->soa.expire));
                fprintf(fp, "\tminimum ttl = %d (%s)\n",
                        (int)rd->soa.minimum, res_time(rd->soa.minimum));
                break;

        case T_MB:                              /* Mail Box  */
                fprintf(fp, "\tmail box = %s\n", rd->string);
                break;

        case T_MG:                              /* Mail Group */
                fprintf(fp, "\tmail group member = %s\n", rd->string);
                break;

        case T_MR:                              /* Mail Rename */
                fprintf(fp, "\tmailbox rename = %s\n", rd->string);
                break;

/* Following modification taken from VxWorks --GAT */
/* 01b,29apr97,jag Changed T_NULL to T_NULL_RR to fix conflict with loadCoffLib.h */
        case T_NULL_RR:                         /* Null RR */
                fprintf(fp, "\tdata = \"" );
                for ( i=0 ; i < rd->null.length ; i++ ) {
                        if ( isprint(rd->null.anything[i] )) {
                                if ( rd->null.anything[i] == '\n' )
                                        putc('\\', fp);
                                putc( rd->null.anything[i], fp );
                        }
                        else
                                fprintf(fp, "\\%o", rd->null.anything[i]);
                }
                fprintf(fp, "\"\n" );
                break;

        case T_WKS:                             /* Well Known Services */
                fprintf(fp, "\tinet address = %s, protocol = %s\n",
                        inet_ntoa(rd->wks.address), res_proto(rd->wks.protocol));
                fprintf(fp, "\t");
                for ( i=0 ; i<rd->wks.maplength ; i++ ) {
                        int     j;
                        char    c;

                        c = rd->wks.bitmap[i];
                        for ( j=0 ; j<8 ; j++) {
                                if ( c & 0200 )
                                        fprintf(fp, "  %s", res_wks(i*8 + j));
                                c <<= 1;
                        }
                }
                fprintf(fp, "\n");
                break;

        case T_PTR:                             /* Domain Name Pointer */
                fprintf(fp, "\tname = %s\n", rd->string);
                break;

        case T_HINFO:                           /* Host Info */
                fprintf(fp, "\tcpu = %s,", rd->hinfo.cpu);
                fprintf(fp, "  os = %s\n", rd->hinfo.os);
                break;

        case T_MINFO:                           /* Mailbox Info */
                fprintf(fp, "\trequests = %s\n", rd->minfo.rmailbx);
                fprintf(fp, "\terrors = %s\n", rd->minfo.emailbx);
                break;

        case T_MX:                              /* Mail Exchanger */
                fprintf(fp, "\tpreference = %d,", rd->mx.preference);
                fprintf(fp, "\tmail exchanger = %s\n", rd->mx.exchange);
                break;

        case T_SRV:                             /* Server Location */
                fprintf(fp, "\tpriority = %d, ", rd->srv.priority);
                fprintf(fp, "weight = %d, ", rd->srv.weight);
                fprintf(fp, "port = %d, ", rd->srv.port);
                fprintf(fp, "target = %s\n", rd->srv.target);
                break;

        case T_TXT:                             /* Text string */
                fprintf(fp, "\t\"%s\"\n", rd->txt.text);
                if ( rd->txt.next != NULL ) {
                        struct s_TXT    *txtp;

                        txtp = &(rd->txt);
                        while ( txtp->next != NULL ) {
                                txtp = txtp->next;
                                fprintf(fp, "\t\"%s\"\n", txtp->text);
                        }
                }
                break;



                        /*
                         *  RFC 1183  Additional types
                         */
        case T_AFSDB:                           /* AFS Server */
                fprintf(fp, "\tsubtype = %d,", rd->afsdb.subtype);
                fprintf(fp, "\thostname = %s\n", rd->afsdb.hostname);
                break;


        case T_RP:                              /* Responsible Person */
                fprintf(fp, "\tmailbox dname = %s\n", rd->rp.mbox_dname);
                fprintf(fp, "\ttext dname = %s\n", rd->rp.txt_dname);
                break;

        case T_X25:                             /* X25 Address */
                fprintf(fp, "\tX.25 address = %s\n", rd->string);
                break;

        case T_ISDN:                            /* ISDN Address */
                fprintf(fp, "\tISDN address = %s", rd->isdn.address);
                if ( rd->isdn.sa[0] != '.' || rd->isdn.sa[1] != '\0' )
                        fprintf(fp, " %s", rd->isdn.sa);
                fprintf(fp, "\n");
                break;

        case T_RT:                              /* Route Through */
                fprintf(fp, "\tpreference = %d,", rd->rt.preference);
                fprintf(fp, "\tintermediate host = %s\n", rd->rt.int_host);
                break;

                        /*
                         *  Additional Non-standard types
                         */
        case T_UINFO:                           /* User (finger) info */
                fprintf(fp, "\tuser info = %s\n", rd->string);
                break;

        case T_UID:                             /* User ID */
                fprintf(fp, "\tuid = %d\n", (int)rd->number);
                break;

        case T_GID:                             /* Group ID */
                fprintf(fp, "\tgid = %d\n", (int)rd->number);
                break;

        case T_UNSPEC:                          /* Unspecified info */
        default:                                /* Unrecognised */
                fprintf(fp, "\tdata = \"" );
                for ( i=0 ; i < dlen ; i++ ) {
                        if ( isprint(rd->string[i] )) {
                                if ( rd->string[i] == '\n' )
                                        putc('\\', fp);
                                putc( rd->string[i], fp );
                        }
                        else
                                fprintf(fp, "\\%o", rd->string[i]);
                }
                fprintf(fp, "\"\n" );
                break;
        }


                /*
                 *  finish off the RR-independent information
                 */
        if ( ptr->type != T_SOA )
                fprintf(fp, "\tttl = %d (%s)\n", (int)ptr->ttl, res_time(ptr->ttl));

}




        /*
         *  print_response:
         *      Print a DNS response buffer
         */
void
print_response(FILE* fp, res_response* resp)
{
        unsigned int    i;
        u_short n;
        HEADER  *hp;

        /*
         * Print out the header fields.
         */
        hp=&(resp->header);
        fprintf(fp, ";;  HEADER:\n");
        print_header(fp, hp, TRUE);


        /*
         * Print question records.
         */
        if ((n = hp->qdcount)) {
                fprintf(fp, ";;  QUESTIONS:\n");
                for ( i=0 ; i<n ; i++ )
                        print_question(fp, resp->question[i]);
        }

        /*
         * Print authoritative answer records
         */
        if ((n = hp->ancount)) {
                fprintf(fp, ";;  ANSWERS:\n");
                for ( i=0 ; i<n ; i++ )
                        print_rr(fp, resp->answer[i]);
        }


        /*
         * Print name server records
         */
        if ((n = hp->nscount)) {
                fprintf(fp, ";;  AUTHORITY RECORDS:\n");
                for ( i=0 ; i<n ; i++ )
                        print_rr(fp, resp->authority[i]);
        }


        /*
         * Print additional records
         */
        if ((n = hp->arcount)) {
                fprintf(fp, ";;  ADDITIONAL RECORDS:\n");
                for ( i=0 ; i<n ; i++ )
                        print_rr(fp, resp->additional[i]);
        }
}



        /*
         *  res_print:
         *      Print a DNS response buffer
         */
void
res_print(res_response  *resp)
{
        print_response(stdout, resp);
}
