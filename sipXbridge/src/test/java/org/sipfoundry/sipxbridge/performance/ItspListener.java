/**
 * 
 */
package org.sipfoundry.sipxbridge.performance;

import gov.nist.javax.sip.DialogExt;
import gov.nist.javax.sip.DialogTimeoutEvent;
import gov.nist.javax.sip.ListeningPointExt;
import gov.nist.javax.sip.ServerTransactionExt;
import gov.nist.javax.sip.SipListenerExt;
import gov.nist.javax.sip.SipProviderExt;
import gov.nist.javax.sip.message.RequestExt;
import gov.nist.javax.sip.message.ResponseExt;

import javax.sip.DialogTerminatedEvent;
import javax.sip.IOExceptionEvent;
import javax.sip.RequestEvent;
import javax.sip.ResponseEvent;
import javax.sip.SipProvider;
import javax.sip.TimeoutEvent;
import javax.sip.Transaction;
import javax.sip.TransactionTerminatedEvent;
import javax.sip.header.CSeqHeader;
import javax.sip.message.Request;
import javax.sip.message.Response;


public class ItspListener implements SipListenerExt {

    /**
     * 
     */
    private SipProviderExt sipProvider;
    private ListeningPointExt listeningPoint;

    public ItspListener(ListeningPointExt listeningPoint, SipProviderExt sipProvider) {
        this.listeningPoint = listeningPoint;
        this.sipProvider = sipProvider;
    }

    @Override
    public void processDialogTimeout(DialogTimeoutEvent dte) {
        System.err.println("Dialog timed out. Test failed");
    }

    @Override
    public void processDialogTerminated(DialogTerminatedEvent dte) {
        PerformanceTester.itspCompletedCalls++;
       if ( PerformanceTester.itspCompletedCalls % 1000 == 0)
            System.out.println("Completed " + PerformanceTester.itspCompletedCalls);
      
    }

    @Override
    public void processIOException(IOExceptionEvent arg0) {
        // TODO Auto-generated method stub

    }

    @Override
    public void processRequest(RequestEvent requestEvent) {
        try {
            RequestExt request = (RequestExt) requestEvent.getRequest();
            ServerTransactionExt serverTransaction;
            serverTransaction = (ServerTransactionExt) requestEvent
                    .getServerTransaction();
           
            if (serverTransaction == null) {
                SipProvider sipProvider = (SipProvider) requestEvent
                        .getSource();
                serverTransaction = (ServerTransactionExt) sipProvider
                        .getNewServerTransaction(request);
            }
            
            if ( serverTransaction == null ) {
                 System.out.println("dropping request no server transaction ");
                 return;
            }
            if (request.getMethod().equals(Request.INVITE)) {
                
                if (requestEvent.getServerTransaction() == null) {
                    PerformanceTester.timer.schedule( new RingingTimerTask(serverTransaction,
                            listeningPoint),PerformanceTester.RING_TIME);
                }
            } else if (request.getMethod().equals(Request.BYE)) {
                Response response = PerformanceTester.messageFactory.createResponse(
                        Response.OK, request);
                serverTransaction.sendResponse(response);

            } else if ( request.getMethod().equals(Request.ACK)) {
                SipProvider sipProvider = (SipProvider) requestEvent
                .getSource();
                PerformanceTester.timer.schedule(new ByeTimerTask(requestEvent,
                            listeningPoint),PerformanceTester.CALL_TIME);
            }
        } catch (Exception ex) {
            ex.printStackTrace();
            System.exit(0);
        }
    }

    @Override
    public void processResponse(ResponseEvent responseEvent) {
        try {
            DialogExt dialog = (DialogExt) responseEvent.getDialog();
            ResponseExt response = (ResponseExt) responseEvent
                    .getResponse();
            CSeqHeader cseqHeader = response.getCSeqHeader();
            if (response.getStatusCode() == Response.OK
                    && cseqHeader.getMethod().equals(Request.INVITE)) {
                long cseqNo = cseqHeader.getSeqNumber();
                Request ackRequest = dialog.createAck(cseqNo);
                dialog.sendAck(ackRequest);

            }
        } catch (Exception ex) {
            ex.printStackTrace();
            System.exit(0);
        }
    }

    @Override
    public void processTimeout(TimeoutEvent timeout) {
        Transaction tr = timeout.getClientTransaction() == null ? timeout.getServerTransaction() :
                timeout.getClientTransaction();
        System.out.println("Timeout occured stopping test " + tr.getRequest() );
        System.exit(0);
    }

    @Override
    public void processTransactionTerminated(TransactionTerminatedEvent arg0) {
        // TODO Auto-generated method stub

    }

}