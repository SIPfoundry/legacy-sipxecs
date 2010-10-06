/*
 *  res_free.c
 *
 *  Free a DNS response tree
 *
 *  v1.0
 *
 *  Dave Shield         January 1994
 *
 *  v1.3        Correct a couple of resource-specific entries
 *              Free individual question structures
 *                      July 1997
 */

#include        "resparse/rr.h"

        /*
         *  free_rr:
         *      Free an individual Resource Record record
         */
void
free_rr(rrp)
        s_rr    *rrp;
{
  if (!rrp)
    return;
                /*
                 *  Free generic RR memory
                 */
        free(rrp->name);

                /*
                 *  Free RR-specific memory
                 */
        switch (rrp->type) {
        case T_NS:
        case T_MD:
        case T_MF:
        case T_CNAME:
                free(rrp->rdata.string);
                break;
        case T_SOA:
                free(rrp->rdata.soa.mname);
                free(rrp->rdata.soa.rname);
                break;
        case T_MB:
        case T_MG:
        case T_MR:
                free(rrp->rdata.string);
                break;
/* Following modification taken from VxWorks --GAT */
/* 01b,29apr97,jag Changed T_NULL to T_NULL_RR to fix conflict with loadCoffLib.h */
        case T_NULL_RR:
                free(rrp->rdata.null.anything);
                break;
        case T_WKS:
                free(rrp->rdata.wks.bitmap);
                break;
        case T_PTR:
                free(rrp->rdata.string);
                break;
        case T_HINFO:
                free(rrp->rdata.hinfo.cpu);
                free(rrp->rdata.hinfo.os);
                break;
        case T_MINFO:
                free(rrp->rdata.minfo.rmailbx);
                free(rrp->rdata.minfo.emailbx);
                break;
        case T_MX:
                free(rrp->rdata.mx.exchange);
                break;
        case T_SRV:
                free(rrp->rdata.srv.target);
                break;
        case T_NAPTR:
                free(rrp->rdata.naptr.flags);
                free(rrp->rdata.naptr.services);
                free(rrp->rdata.naptr.regexp);
                if (NULL != rrp->rdata.naptr.replacement)   // why reqd ???
                   free(rrp->rdata.naptr.replacement);
                break;
        case T_TXT:
                free(rrp->rdata.txt.text);
                if (rrp->rdata.txt.next != NULL) {
                        struct s_TXT    *txtp;
                        struct s_TXT    *txtp2;

                        txtp = &(rrp->rdata.txt);
                        while ( txtp->next != NULL ) {
                                free( txtp->text );
                                txtp2 = txtp->next;
                                free(txtp);
                                txtp = txtp2;
                        }
                }
                break;
        case T_AFSDB:
                free(rrp->rdata.afsdb.hostname);
                break;
        case T_RP:
                free(rrp->rdata.rp.mbox_dname);
                free(rrp->rdata.rp.txt_dname);
                break;
        case T_X25:
                free(rrp->rdata.string);
                break;
        case T_ISDN:
                free(rrp->rdata.isdn.address);
                free(rrp->rdata.isdn.sa);
                break;
        case T_RT:
                free(rrp->rdata.rt.int_host);
                break;
        case T_UINFO:
        case T_UNSPEC:
                free(rrp->rdata.string);
                break;
        }

                /*
                 *  Free RR itself
                 */
        free(rrp);
}



        /*
         *  free_response:
         *      Free the contents of a DNS response tree
         */
void
free_response(resp)
        res_response    *resp;
{
  if (!resp)
    return;
  
        int     i, n;

        /*
         *  Free question memory
         */

/* res_parse already reverses the byte order, remove swap --GAT 3/1/01  */
/*      if ((n = ntohs((uint16_t)resp->header.qdcount))) { remove ntohs --GAT */
        if ((n =       (uint16_t)resp->header.qdcount) ) {
                for ( i=0 ; i<n ; i++ ) {
                        free(resp->question[i]->qname);
                        resp->question[i]->qname = NULL;
                        free(resp->question[i]);
                        resp->question[i] = NULL;
                }
                free(resp->question);
                resp->question = NULL;
                resp->header.qdcount=0;
        }

        /*
         *  Free authoritative answer memory
         */
/* res_parse already reverses the byte order, remove swap --GAT 3/1/01  */
/*      if ((n = ntohs((uint16_t)resp->header.ancount))) { remove ntohs --GAT */
        if ((n =       (uint16_t)resp->header.ancount) ) {
                for ( i=0 ; i<n ; i++ ) {
                        free_rr(resp->answer[i]);
                        resp->answer[i] = NULL;
                }
                free(resp->answer);
                resp->answer = NULL;
                resp->header.ancount=0;
        }

        /*
         *  Free name server memory
         */
/* res_parse already reverses the byte order, remove swap --GAT 3/1/01  */
/*      if ((n = ntohs((uint16_t)resp->header.nscount))) { remove ntohs --GAT */
        if ((n =       (uint16_t)resp->header.nscount) ) {
                for ( i=0 ; i<n ; i++ ) {
                        if (NULL != resp->authority[i])   // why reqd ???
                           free_rr(resp->authority[i]);
                        resp->authority[i] = NULL;
                }
                free(resp->authority);
                resp->authority = NULL;
                resp->header.nscount=0;
        }

        /*
         *  Free additional memory
         */
/* res_parse already reverses the byte order, remove swap --GAT 3/1/01  */
/*      if ((n = ntohs((uint16_t)resp->header.arcount))) { remove ntohs --GAT */
        if ((n =       (uint16_t)resp->header.arcount) ) {
                for ( i=0 ; i<n ; i++ ) {
                        free_rr(resp->additional[i]);
                        resp->additional[i] = NULL;
                }
                free(resp->additional);
                resp->additional = NULL;
                resp->header.arcount=0;
        }
}



        /*
         *  res_free:
         *      Free a DNS response tree
         */
void
res_free(resp)
        res_response    *resp;
{
  if (!resp)
    return;
  
        free_response( resp );
        free( resp );
}
