package org.sipfoundry.siptester;

import gov.nist.javax.sip.DialogExt;
import gov.nist.javax.sip.message.RequestExt;
import gov.nist.javax.sip.message.ResponseExt;
import gov.nist.javax.sip.message.SIPRequest;
import gov.nist.javax.sip.message.SIPResponse;

import java.util.HashSet;
import java.util.Hashtable;
import java.util.Iterator;
import java.util.LinkedList;
import java.util.concurrent.ConcurrentSkipListSet;
import java.util.concurrent.TimeUnit;

import javax.sip.Dialog;
import javax.sip.ServerTransaction;
import javax.sip.header.RequireHeader;
import javax.sip.header.ViaHeader;
import javax.sip.message.Request;
import javax.sip.message.Response;

import org.apache.log4j.Logger;

public class SipServerTransaction extends SipTransaction implements
        Comparable<SipServerTransaction> {
    private static Logger logger = Logger.getLogger(SipServerTransaction.class);

    private ConcurrentSkipListSet<SipResponse> responses = new ConcurrentSkipListSet<SipResponse>();

    /*
     * The endpoint to which the server transaction belongs.
     */
    private EmulatedEndpoint endpoint;

    /*
     * The Mock dialog to which the server transaction belongs.
     */
    private SipDialog dialog;

    /*
     * The client transaction branch that maps to this server transaction.
     */
    private String branch;

    /*
     * The emulated server transaction.
     */
    private ServerTransaction serverTransaction;

    public SipServerTransaction(SipRequest sipRequest) {
        this.sipRequest = sipRequest;
        if (sipRequest.getTime() < minDelay) {
            minDelay = sipRequest.getTime();
        }
        super.delay = sipRequest.getTime() - minDelay;
    }

    public int compareTo(SipServerTransaction serverTx) {
        if (this.getDelay() == serverTx.getDelay())
            return 0;
        else if (this.getDelay() < serverTx.getDelay())
            return -1;
        else
            return 1;

    }

    public void addRequest(SipRequest sipRequest) {
        logger.debug("retransmittedRequest -- ignoring " + sipRequest.getSipRequest().getFirstLine());
    }

    public void addResponse(SipResponse sipResponse) {
        boolean added =  this.responses.add(sipResponse);
        if ( !added ) {
            logger.debug("response already added");
        }
    }

    /**
     * Get the dialog ID for this server transaction.
     * 
     * @return
     */
    public String getDialogId() {
        if (sipRequest.getSipRequest().getFromHeader().getTag() != null
                && sipRequest.getSipRequest().getToHeader().getTag() != null) {  
              return ((SIPRequest) sipRequest.getSipRequest()).getDialogId(true);
        }
        for (SipResponse sipResponse : responses) {
            ResponseExt response = sipResponse.getSipResponse();
            if (response.getFromHeader().getTag() != null
                    && response.getToHeader().getTag() != null) {
                  return ((SIPResponse) response).getDialogId(true);
            }
        }
        
        return null;
    }

    /**
     * @param endpoint the endpoint to set
     */
    public void setEndpoint(EmulatedEndpoint endpoint) {
        this.endpoint = endpoint;
    }

    /**
     * @return the endpoint
     */
    public EmulatedEndpoint getEndpoint() {
        return endpoint;
    }

    public void setDialog(SipDialog sipDialog) {
        this.dialog = sipDialog;
    }

    public SipDialog getSipDialog() {
        return this.dialog;
    }

    public void setBranch(String branch) {
        this.branch = branch;
        logger.debug("setBranch " + branch);
    }

    public String getBranch() {
        return this.branch;
    }

    public void printServerTransaction() {
        SipTester.getPrintWriter().println("<server-transaction>");
        SipTester.getPrintWriter().println("<transaction-id>" + this.getTransactionId() + "</transaction-id>");
        
        SipTester.getPrintWriter().println("<sip-request><![CDATA[");
        SipTester.getPrintWriter().print(this.sipRequest.getSipRequest());
        SipTester.getPrintWriter().println("]]></sip-request>");
        SipTester.getPrintWriter().println("<responses>");
        
        for (SipResponse response : this.responses) {
            SipTester.getPrintWriter().println("<sip-response><![CDATA[");
            SipTester.getPrintWriter().println(response.getSipResponse() + "]]></sip-response>" );
        }
        SipTester.getPrintWriter().println("</responses>");
        SipTester.getPrintWriter().println("</server-transaction>");
    }

    public void sendResponses() {
        try {
            System.out.println("serverTransactionId " + this.getBranch() + " tid = " + this.getTransactionId());
            RequestExt request = (RequestExt) serverTransaction.getRequest();
            if (responses.isEmpty()) {
                System.out.println("no Responses to SEND ");
            }
            Iterator<SipResponse> it = this.responses.iterator();
            while (it.hasNext()) {
                ResponseExt newResponse = SipUtilities.createResponse(endpoint, request, it.next());
                it.remove();

                Iterator headerIterator = newResponse.getHeaders(RequireHeader.NAME);
                boolean isReliableResponse = false;
                while (headerIterator != null && headerIterator.hasNext()) {
                    RequireHeader requireHeader = (RequireHeader) headerIterator.next();
                    if (requireHeader.getOptionTag().equals("100rel")) {
                        isReliableResponse = true;
                    }
                }
               
                if (!isReliableResponse || newResponse.getStatusCode() / 100 >= 2) {
                    serverTransaction.sendResponse(newResponse);
                } else {
                    serverTransaction.getDialog().sendReliableProvisionalResponse(newResponse);
                    new Thread(new Runnable() {

                        @Override
                        public void run() {
                            try {      
                                String dialogId = getDialogId();
                                SipDialog dialog = SipTester.getDialog(dialogId);
                                dialog.waitForPrack();
                                sendResponses();
                            } catch (Exception ex) {
                                SipTester.fail("Unexpected exception", ex);
                            }
                        }
                    }).start();
                    return;
                }

            }
        } catch (Exception ex) {
            SipTester.fail("Unexpected exception ", ex);

        }

    }

    public void setServerTransaction(ServerTransaction serverTransaction) {
        this.serverTransaction = serverTransaction;
        serverTransaction.setApplicationData(this);
        DialogExt dialog = (DialogExt) this.serverTransaction.getDialog();
        String dialogId = this.getDialogId();
        logger.debug("setServerTransaction: dialogId = " +  dialogId);
        if ( this.dialog != null ) {
             SipDialog sipDialog = this.dialog;
             dialog.setApplicationData(sipDialog);
             sipDialog.setDialog(dialog);
        } else {
            logger.debug("dialog is not set for SipServerTransaction ");
        }
    }

}
