package org.sipfoundry.siptester;

import java.util.Collection;

import gov.nist.javax.sip.DialogTimeoutEvent;
import gov.nist.javax.sip.ListeningPointExt;
import gov.nist.javax.sip.SipListenerExt;
import gov.nist.javax.sip.message.RequestExt;

import javax.sip.ClientTransaction;
import javax.sip.Dialog;
import javax.sip.DialogTerminatedEvent;
import javax.sip.IOExceptionEvent;
import javax.sip.RequestEvent;
import javax.sip.ResponseEvent;
import javax.sip.ServerTransaction;
import javax.sip.SipProvider;
import javax.sip.TimeoutEvent;
import javax.sip.TransactionTerminatedEvent;
import javax.sip.message.Request;
import javax.sip.message.Response;

import org.apache.log4j.Logger;

public class SipListenerImpl implements SipListenerExt {

    private static Logger logger = Logger.getLogger(SipListenerImpl.class);

    private EmulatedEndpoint endpoint;

    long wallClockTime;

    public SipListenerImpl(EmulatedEndpoint endpoint) {
        this.endpoint = endpoint;

    }

   

    public void processRequest(RequestEvent requestEvent) {
        try {
            RequestExt request = (RequestExt) requestEvent.getRequest();
            
            logger.debug("processRequest : " + request.getFirstLine());

            ServerTransaction serverTransaction = requestEvent.getServerTransaction();
            SipProvider sipProvider = (SipProvider) requestEvent.getSource();
            if (serverTransaction == null) {
                serverTransaction = sipProvider.getNewServerTransaction(request);
            }

            ListeningPointExt listeningPoint = (ListeningPointExt) sipProvider
                    .getListeningPoint(request.getTopmostViaHeader().getTransport());

            EmulatedEndpoint endpoint = SipTester.getEndpoint(listeningPoint);
            
            Collection<SipServerTransaction> transactions  = endpoint.findSipServerTransaction(request);
            
            if ( transactions.isEmpty())  {
                /*
                 * Could not find a matching server transaction. Find a ST where the From
                 * and To tags match. 
                 */
                 Dialog dialog = requestEvent.getDialog();
                 
                 if (dialog != null  ){
                     SipDialog sipDialog = (SipDialog) dialog.getApplicationData();
                     if ( sipDialog != null ) {
                         sipDialog.setLastRequestReceived(request);
                     }
                 } else {
                     logger.debug("Dialog is null");
                 }
                 
                 
                 if (! request.getMethod().equals(Request.ACK)) {
                     Response response = SipTester.getMessageFactory().createResponse(
                                Response.OK, request);
                     serverTransaction.sendResponse(response);
                 }
               
            }
            
           
            for (SipServerTransaction sst : transactions) {
                serverTransaction.setApplicationData(sst);
                sst.setServerTransaction(serverTransaction);
                String dialogId = sst.getDialogId();
                SipDialog sipDialog = SipTester.getDialog(dialogId);
                if (sipDialog != null) {
                    sipDialog.setLastRequestReceived(request);
                } else {
                    logger.debug("Could not find a sip dialog "  + request.getFirstLine());
                }
                sst.getSipRequest().setRequestEvent(requestEvent);
                
                sst.sendResponses();
                
                for (SipClientTransaction ct : sst.getSipRequest().getPostConditions()) {
                    ct.removePrecondition(sst.getSipRequest());
                }
               
            }

        } catch (Exception ex) {
            SipTester.fail("unexpected error processing request", ex);
        }

    }

    public void processResponse(ResponseEvent responseEvent) {
        try {
            Response response = responseEvent.getResponse();
            String method = SipUtilities.getCSeqMethod(response);
         
            
            if (method.equals(Request.INVITE) || method.equals(Request.SUBSCRIBE)
                    || method.equals(Request.NOTIFY) || method.equals(Request.PRACK)
                    || method.equals(Request.BYE) || method.equals(Request.REFER) || method.equals(Request.REGISTER)) {
                ClientTransaction ctx = responseEvent.getClientTransaction();
                if ( ctx == null ) {
                    System.out.println("retransmission -- ingoring");
                    return;
                }
                SipClientTransaction sipCtx = (SipClientTransaction) ctx.getApplicationData();
                sipCtx.processResponse(responseEvent);
            }
        } catch (Exception ex) {
            logger.error("Unexpected exception ", ex);
            SipTester.fail("Unexpected exception", ex);
        }

    }

    public void processTimeout(TimeoutEvent timeoutEvent) {
        if (timeoutEvent.getClientTransaction() != null) {
            System.err.println("Test Failed! "
                    + timeoutEvent.getClientTransaction().getRequest().getMethod() + " Time out");
        } else {
            System.err.println("Test Failed! "
                    + timeoutEvent.getServerTransaction().getRequest().getMethod() + " Time out");

        }
        
        
        SipTester.fail("Transaction time out ");

    }

    @Override
    public void processDialogTimeout(DialogTimeoutEvent timeoutEvent) {
        // TODO Auto-generated method stub

    }

    @Override
    public void processDialogTerminated(DialogTerminatedEvent dialogTerminatedEvent) {
        // TODO Auto-generated method stub

    }

    @Override
    public void processIOException(IOExceptionEvent exceptionEvent) {
        // TODO Auto-generated method stub

    }

    @Override
    public void processTransactionTerminated(TransactionTerminatedEvent transactionTerminatedEvent) {
        // TODO Auto-generated method stub

    }

}
