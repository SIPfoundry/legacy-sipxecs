/**
 * 
 */
package org.sipfoundry.sipxbridge.performance;

import gov.nist.javax.sip.ListeningPointExt;
import gov.nist.javax.sip.ServerTransactionExt;
import gov.nist.javax.sip.SipProviderExt;

import java.util.TimerTask;

import javax.sdp.SessionDescription;
import javax.sip.header.ContactHeader;
import javax.sip.header.ContentTypeHeader;
import javax.sip.message.Request;
import javax.sip.message.Response;

class RingingTimerTask extends TimerTask {

    private ServerTransactionExt serverTransaction;
    private Request request;
    private ListeningPointExt listeningPoint;

    public RingingTimerTask(ServerTransactionExt serverTransaction,
            ListeningPointExt lp) throws Exception {
        this.serverTransaction = serverTransaction;
        this.request = serverTransaction.getRequest();
        this.listeningPoint = lp;
        SipProviderExt sipProvider = (SipProviderExt) this.serverTransaction
                .getSipProvider();
        Response response = PerformanceTester.messageFactory.createResponse(Response.RINGING,
                request);
        serverTransaction.sendResponse(response);
    }

    @Override
    public void run() {
        try {
            Response response = PerformanceTester.messageFactory.createResponse(Response.OK,
                    request);
            String ipAddress = listeningPoint.getIPAddress();
            SessionDescription sdp = PerformanceTester.createSessionDescription(ipAddress);
            ContentTypeHeader cth = PerformanceTester.headerFactory.createContentTypeHeader(
                    "application", "sdp");
            response.setContent(sdp.toString().getBytes(), cth);
            ContactHeader contactHeader = listeningPoint
                    .createContactHeader();
            response.setHeader(contactHeader);
            serverTransaction.sendResponse(response);

        } catch (Exception ex) {
            ex.printStackTrace();
            System.exit(0);
        }

    }

}