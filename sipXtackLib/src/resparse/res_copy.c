/*
 *  res_copy.c
 *
 *  Create a copy of a DNS response tree
 *
 *  v1.0
 *
 *  Dave Shield         January 1994
 *
 *  v1.1        Cast all malloc calls
 *                      February 1994
 *
 *  v1.2        Fix string copying
 *              Missing function return
 *              Use memcpy rather than bcopy
 *                      November 1995
 *
 *  v1.3        Combine 'res_copy()/copy_response()' routines
 *              Network/Host ordering handled in 'res_parse()'
 *                      July 1997
 */

#ifndef __pingtel_on_posix__
#include        "resparse/rr.h"


        /*
         *  copy_question:
         *      Copy an individual question record
         *
         *  returns a pointer to the new question
         */
s_question *
copy_question(oldq)
        s_question      *oldq;
{
        s_question      *newq;

        if ((newq = (s_question *)malloc(sizeof(s_question))) == NULL )
                return(NULL);

        if ((newq->qname = (char *)malloc(strlen(oldq->qname)+1)) == NULL ) {
                free(newq);
                return(NULL);
        }

        strcpy(newq->qname, oldq->qname);
        newq->qtype = oldq->qtype;
        newq->qclass = oldq->qclass;
        return(newq);
}



        /*
         *  copy_rr:
         *      Copy an individual Resource Record record
         *
         *  returns a pointer to the new record
         */
s_rr *
copy_rr(oldr)
        s_rr    *oldr;
{
        s_rr    *newr;
        int     dlen;
        union u_rdata  *ord;
        union u_rdata  *nrd;


                /*
                 *  Copy the RR-independent information
                 */
        if ((newr = (s_rr *)malloc(sizeof(s_rr))) == NULL )
                return(NULL);
        if ((newr->name = (char *)malloc(strlen(oldr->name)+1)) == NULL ) {
                free(newr);
                return(NULL);
        }
        strcpy(newr->name, oldr->name);
        newr->type = oldr->type;
        newr->rclass = oldr->rclass;
        newr->ttl = oldr->ttl;
        dlen = newr->dlen = oldr->dlen;
        ord = &oldr->rdata;
        nrd = &newr->rdata;


                /*
                 *  Copy RR-specifics
                 *
                 */
        switch(oldr->type) {
        case T_A:                               /* Address */
                switch (oldr->rclass) {
                case C_IN:
                        memcpy((void *)&nrd->address, (void *)&ord->address,
                                sizeof(struct in_addr));
                        break;

                default:
                        /* Can't really handle this - just skip it */
                        break;
                }
                break;

        case T_NS:                              /* Name Server */
        case T_MD:                              /* Mail Destination (OBS) */
        case T_MF:                              /* Mail Forwarder   (OBS) */
        case T_CNAME:                           /* Canonical Name */
                if ((nrd->string = (char *)malloc(strlen(ord->string)+1)) == NULL ) {
                        free_rr(newr);
                        return(NULL);
                }
                strcpy(nrd->string, ord->string);
                break;

        case T_SOA:                             /* Start of Authority */
                if ((nrd->soa.mname = (char *)malloc(strlen(ord->soa.mname)+1)) == NULL ) {
                        nrd->soa.rname = NULL;
                        free_rr(newr);
                        return(NULL);
                }
                if ((nrd->soa.rname = (char *)malloc(strlen(ord->soa.rname)+1)) == NULL ) {
                        free_rr(newr);
                        return(NULL);
                }
                strcpy(nrd->soa.mname, ord->soa.mname);
                strcpy(nrd->soa.rname, ord->soa.rname);

                nrd->soa.serial  = ord->soa.serial;
                nrd->soa.refresh = ord->soa.refresh;
                nrd->soa.retry   = ord->soa.retry ;
                nrd->soa.expire  = ord->soa.expire;
                nrd->soa.minimum = ord->soa.minimum;
                break;

        case T_MB:                              /* Mail Box  */
        case T_MG:                              /* Mail Group */
        case T_MR:                              /* Mail Rename */
                if ((nrd->string = (char *)malloc(strlen(ord->string)+1)) == NULL ) {
                        free_rr(newr);
                        return(NULL);
                }
                strcpy(nrd->string, ord->string);
                break;

/* Following modification taken from VxWorks --GAT */
/* 01b,29apr97,jag Changed T_NULL to T_NULL_RR to fix conflict with loadCoffLib.h */
        case T_NULL_RR:                         /* Null RR */
                if ((nrd->null.anything = (char *)malloc(dlen)) == NULL ) {
                        free_rr(newr);
                        return(NULL);
                }
                memcpy((void *)nrd->null.anything, (void *)ord->null.anything, dlen);
                nrd->null.length = dlen;
                break;

        case T_WKS:                             /* Well Known Services */
                nrd->wks.maplength = dlen-(sizeof(struct in_addr) +1);
                if ((nrd->wks.bitmap = (char *)malloc(nrd->wks.maplength)) == NULL ) {
                        free_rr(newr);
                        return(NULL);
                }
                memcpy((void *)&nrd->wks.address, (void *)&ord->wks.address,
                                sizeof(struct in_addr));
                nrd->wks.protocol = ord->wks.protocol;
                memcpy((void *)&nrd->wks.bitmap, (void *)&ord->wks.bitmap,
                                nrd->wks.maplength);
                break;

        case T_PTR:                             /* Domain Name Pointer */
                if ((nrd->string = (char *)malloc(strlen(ord->string)+1)) == NULL ) {
                        free_rr(newr);
                        return(NULL);
                }
                strcpy(nrd->string, ord->string);
                break;

        case T_HINFO:                           /* Host Info */
                if ((nrd->hinfo.cpu = (char *)malloc(strlen(ord->hinfo.cpu)+1)) == NULL ) {
                        nrd->hinfo.os = NULL;
                        free_rr(newr);
                        return(NULL);
                }
                if ((nrd->hinfo.os = (char *)malloc(strlen(ord->hinfo.os)+1)) == NULL ) {
                        free_rr(newr);
                        return(NULL);
                }
                strcpy(nrd->hinfo.cpu, ord->hinfo.cpu);
                strcpy(nrd->hinfo.os, ord->hinfo.os);
                break;

        case T_MINFO:                           /* Mailbox Info */
                if ((nrd->minfo.rmailbx = (char *)malloc(strlen(ord->minfo.rmailbx)+1)) == NULL ) {
                        nrd->minfo.emailbx = NULL;
                        free_rr(newr);
                        return(NULL);
                }
                if ((nrd->minfo.emailbx = (char *)malloc(strlen(ord->minfo.emailbx)+1)) == NULL ) {
                        free_rr(newr);
                        return(NULL);
                }
                strcpy(nrd->minfo.rmailbx, ord->minfo.rmailbx);
                strcpy(nrd->minfo.emailbx, ord->minfo.emailbx);
                break;

        case T_MX:                              /* Mail Exchanger */
                if ((nrd->mx.exchange = (char *)malloc(strlen(ord->mx.exchange)+1)) == NULL ) {
                        free_rr(newr);
                        return(NULL);
                }
                nrd->mx.preference  = ord->mx.preference;
                strcpy(nrd->mx.exchange, ord->mx.exchange);
                break;

        case T_SRV:                             /* Server */
                if ((nrd->srv.target = (char *)malloc(strlen(ord->srv.target)+1)) == NULL ) {
                        free_rr(newr);
                        return(NULL);
                }
                nrd->srv.priority = ord->srv.priority;
                nrd->srv.weight   = ord->srv.weight;
                nrd->srv.port     = ord->srv.port;
                strcpy(nrd->srv.target, ord->srv.target);
                break;

        case T_NAPTR:
                nrd->naptr.order = ord->naptr.order;
                nrd->naptr.preference = ord->naptr.preference;
                if ((nrd->naptr.flags = (char *)malloc(strlen(ord->naptr.flags)+1)) == NULL ) {
                        free_rr(newr);
                        return(NULL);
                }
                strcpy(nrd->naptr.services, ord->naptr.services);
                if ((nrd->naptr.services = (char *)malloc(strlen(ord->naptr.services)+1)) == NULL ) {
                        free_rr(newr);
                        return(NULL);
                }
                strcpy(nrd->naptr.regexp, ord->naptr.regexp);
                if ((nrd->naptr.regexp = (char *)malloc(strlen(ord->naptr.regexp)+1)) == NULL ) {
                        free_rr(newr);
                        return(NULL);
                }
                strcpy(nrd->naptr.regexp, ord->naptr.regexp);
                if ((nrd->naptr.replacement = (char *)malloc(strlen(ord->naptr.replacement)+1)) == NULL ) {
                        free_rr(newr);
                        return(NULL);
                }
                strcpy(nrd->naptr.replacement, ord->naptr.replacement);
                break;

        case T_TXT:                             /* Text string */
                nrd->txt.next = NULL;
                if ((nrd->txt.text = (char *)malloc(ord->txt.len)) == NULL ) {
                        free_rr(newr);
                        return(NULL);
                }
                nrd->txt.len  = ord->txt.len;
                strcpy( nrd->txt.text, ord->txt.text );

                if ( ord->txt.next != NULL ) {
                        struct s_TXT    *otxtp;
                        struct s_TXT    *ntxtp;

                        otxtp = &(ord->txt);
                        ntxtp = &(nrd->txt);
                        while ( otxtp->next != NULL ) {
                                if (( ntxtp->next = (struct s_TXT *)malloc(sizeof(struct s_TXT))) == NULL ) {
                                        free(newr);
                                        return(NULL);
                                }
                                otxtp = otxtp->next;
                                ntxtp = ntxtp->next;
                                ntxtp->next = NULL;;
                                if (( ntxtp->text = (char *)malloc(otxtp->len)) == NULL ) {
                                        free(newr);
                                        return(NULL);
                                }
                                ntxtp->len  = otxtp->len;
                                strcpy( ntxtp->text, otxtp->text );
                        }
                }
                break;


                        /*
                         *  RFC 1183  Additional types
                         */
        case T_AFSDB:                           /* AFS Server */
                if ((nrd->afsdb.hostname = (char *)malloc(strlen(ord->afsdb.hostname)+1)) == NULL ) {
                        free_rr(newr);
                        return(NULL);
                }

                nrd->afsdb.subtype  = ord->afsdb.subtype;
                strcpy(nrd->afsdb.hostname, ord->afsdb.hostname);
                break;


        case T_RP:                              /* Responsible Person */
                if ((nrd->rp.mbox_dname = (char *)malloc(strlen(ord->rp.mbox_dname)+1)) == NULL ) {
                        nrd->minfo.emailbx = NULL;
                        free_rr(newr);
                        return(NULL);
                }
                if ((nrd->rp.txt_dname = (char *)malloc(strlen(ord->rp.txt_dname)+1)) == NULL ) {
                        free_rr(newr);
                        return(NULL);
                }
                strcpy(nrd->rp.mbox_dname, ord->rp.mbox_dname);
                strcpy(nrd->rp.txt_dname, ord->rp.txt_dname);
                break;

        case T_X25:                             /* X25 Address */
                if ((nrd->string = (char *)malloc(strlen(ord->string)+1)) == NULL ) {
                        free_rr(newr);
                        return(NULL);
                }
                strcpy(nrd->string, ord->string);
                break;

        case T_ISDN:                            /* ISDN Address */
                if ((nrd->isdn.address = (char *)malloc(strlen(ord->isdn.address)+1)) == NULL ) {
                        nrd->isdn.sa = NULL;
                        free_rr(newr);
                        return(NULL);
                }
                if ((nrd->isdn.sa = (char *)malloc(strlen(ord->isdn.sa)+1)) == NULL ) {
                        free_rr(newr);
                        return(NULL);
                }
                strcpy(nrd->isdn.address, ord->isdn.address);
                strcpy(nrd->isdn.sa, ord->isdn.sa);
                break;

        case T_RT:                              /* Route Through */
                if ((nrd->rt.int_host = (char *)malloc(strlen(ord->rt.int_host)+1)) == NULL ) {
                        free_rr(newr);
                        return(NULL);
                }

                nrd->rt.preference  = ord->rt.preference;
                strcpy(nrd->rt.int_host, ord->rt.int_host);
                break;



                        /*
                         *  Additional Non-standard types
                         */
        case T_UINFO:                           /* User (finger) info */
                if ((nrd->string = (char *)malloc(dlen+1)) == NULL ) {
                        free_rr(newr);
                        return(NULL);
                }
                memcpy((void *)&nrd->string, (void *)&ord->string, dlen);
                nrd->string[dlen] = '\0';
                break;

        case T_UID:                             /* User ID */
        case T_GID:                             /* Group ID */
                nrd->number = ord->number;
                break;

        case T_UNSPEC:                          /* Unspecified info */
        default:                                /* Unrecognised */
                if ((nrd->string = (char *)malloc(dlen+1)) == NULL ) {
                        free_rr(newr);
                        return(NULL);
                }
                memcpy((void *)&nrd->string, (void *)&ord->string, dlen);
                nrd->string[dlen] = '\0';
                break;

        }


        return(newr);
}




        /*
         *  res_copy:
         *      Ceate a copy of a DNS response tree
         *
         *  returns a pointer to the new respose
         */

res_response*
res_copy( resp )
        res_response    *resp ;
{
        int     i, n;
        res_response    *new;

        /*
         *  Create the new response structure,
         *  and copy the header fields
         *      (clear record counts in case of problems)
         */
        if ((new = (res_response *)malloc(sizeof(res_response))) == NULL )
                return(NULL);
        memcpy((void *)&(new->header), (void *)&(resp ->header), sizeof(HEADER));
        new->header.qdcount=0;
        new->header.ancount=0;
        new->header.nscount=0;
        new->header.arcount=0;
        new->question = NULL;
        new->answer = NULL;
        new->authority = NULL;
        new->additional = NULL;


        /*
         *  Copy question records
         */
        if ((n = resp ->header.qdcount)) {
                new->header.qdcount = resp ->header.qdcount;
                if ((new->question = (s_question **)malloc(n*sizeof(s_question*))) == NULL)
                        return(NULL);
                for ( i=0 ; i<n ; i++ )
                        new->question[i] = NULL;
                for ( i=0 ; i<n ; i++ )
                        if ((new->question[i] = copy_question(resp ->question[i])) == NULL ) {
                                free_response(new);
                                return(NULL);
                        }
        }


        /*
         *  Copy authoritative answer records
         */
        if ((n = resp ->header.ancount)) {
                new->header.ancount = resp ->header.ancount;
                if ((new->answer = (s_rr **)malloc(n*sizeof(s_rr*))) == NULL) {
                        new->header.ancount = 0;
                        free_response(new);
                        return(NULL);
                }
                for ( i=0 ; i<n ; i++ )
                        new->answer[i] = NULL;
                for ( i=0 ; i<n ; i++ )
                        if ((new->answer[i] = copy_rr(resp ->answer[i])) == NULL ) {
                                free_response(new);
                                return(NULL);
                        }
        }


        /*
         *  Copy name server records
         */
        if ((n = resp ->header.nscount)) {
                new->header.nscount = resp ->header.nscount;
                if ((new->authority = (s_rr **)malloc(n*sizeof(s_rr*))) == NULL) {
                        new->header.nscount = 0;
                        free_response(new);
                        return(NULL);
                }
                for ( i=0 ; i<n ; i++ )
                        new->authority[i] = NULL;
                for ( i=0 ; i<n ; i++ )
                        if ((new->authority[i] = copy_rr(resp ->authority[i])) == NULL ) {
                                free_response(new);
                                return(NULL);
                }
        }


        /*
         *  Copy additional records
         */
        if ((n = resp ->header.arcount)) {
                new->header.arcount = resp ->header.arcount;
                if ((new->additional = (s_rr **)malloc(n*sizeof(s_rr*))) == NULL) {
                        new->header.arcount = 0;
                        free_response(new);
                        return(NULL);
                }
                for ( i=0 ; i<n ; i++ )
                        new->additional[i] = NULL;
                for ( i=0 ; i<n ; i++ )
                        if ((new->additional[i] = copy_rr(resp ->additional[i])) == NULL ) {
                                free_response(new);
                                return(NULL);
                }
        }

        return( new );
}
#endif /* __pingtel_on_posix__ */
