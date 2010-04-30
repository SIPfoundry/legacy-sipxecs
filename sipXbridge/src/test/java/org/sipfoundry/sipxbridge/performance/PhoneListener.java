/**
 * 
 */
package org.sipfoundry.sipxbridge.performance;

import gov.nist.javax.sip.ClientTransactionExt;
import gov.nist.javax.sip.DialogExt;
import gov.nist.javax.sip.DialogTimeoutEvent;
import gov.nist.javax.sip.ListeningPointExt;
import gov.nist.javax.sip.SipListenerExt;
import gov.nist.javax.sip.SipProviderExt;
import gov.nist.javax.sip.message.ResponseExt;

import java.util.ArrayList;
import java.util.Random;

import javax.sip.DialogTerminatedEvent;
import javax.sip.IOExceptionEvent;
import javax.sip.RequestEvent;
import javax.sip.ResponseEvent;
import javax.sip.TimeoutEvent;
import javax.sip.TransactionTerminatedEvent;
import javax.sip.address.Address;
import javax.sip.address.SipURI;
import javax.sip.header.CSeqHeader;
import javax.sip.header.CallIdHeader;
import javax.sip.header.ContactHeader;
import javax.sip.header.ContentTypeHeader;
import javax.sip.header.FromHeader;
import javax.sip.header.MaxForwardsHeader;
import javax.sip.header.RouteHeader;
import javax.sip.header.ToHeader;
import javax.sip.header.ViaHeader;
import javax.sip.message.Request;
import javax.sip.message.Response;


public class PhoneListener implements SipListenerExt {

  
      private ListeningPointExt listeningPoint;
    private SipProviderExt sipProvider;
    private String transport = "tcp";

    public PhoneListener(ListeningPointExt listeningPoint,
            SipProviderExt sipProvider) {
        this.listeningPoint = listeningPoint;
        this.sipProvider = sipProvider;
    }

    @Override
    public void processDialogTimeout(DialogTimeoutEvent dte) {
        System.out.println("Dialog Timeout occured - aborting test");
        System.exit(0);

    }

    @Override
    public void processDialogTerminated(DialogTerminatedEvent dte) {
        PerformanceTester.phoneDialogCompletions++;
    }

    @Override
    public void processIOException(IOExceptionEvent arg0) {
        System.err.println("unexpected IOEx");
        System.exit(0);
    }

    @Override
    public void processRequest(RequestEvent requestEvent) {
        // TODO Auto-generated method stub

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
                dialog.setApplicationData(new DialogContext((DialogExt)dialog));
                dialog.sendAck(ackRequest);
              
            }
        } catch (Exception ex) {
            ex.printStackTrace();
            System.exit(0);
        }
    }

    @Override
    public void processTimeout(TimeoutEvent arg0) {
        System.out.println("Timeout recorded");
        System.exit(0);

    }

    @Override
    public void processTransactionTerminated(TransactionTerminatedEvent arg0) {
        // TODO Auto-generated method stub

    }

    public void sendInvite() throws Exception {
        String fromName = "BigGuy";
        String fromSipAddress = "here.com";
        String fromDisplayName = "The Master Blaster";

        String toSipAddress = "ot.bandwidth.com";
        String toUser = "LittleGuy";
        String toDisplayName = "The Little Blister";

        // create >From Header
        SipURI fromAddress = PerformanceTester.addressFactory.createSipURI(fromName,
                fromSipAddress);

        Address fromNameAddress = PerformanceTester.addressFactory.createAddress(fromAddress);
        fromNameAddress.setDisplayName(fromDisplayName);
        String fromTag = Integer.toHexString(Math.abs(new Random()
                .nextInt()));
        FromHeader fromHeader = PerformanceTester.headerFactory.createFromHeader(
                fromNameAddress, fromTag);

        // create To Header
        SipURI toAddress = PerformanceTester.addressFactory
                .createSipURI(toUser, toSipAddress);
        Address toNameAddress = PerformanceTester.addressFactory.createAddress(toAddress);
        toNameAddress.setDisplayName(toDisplayName);
        ToHeader toHeader = PerformanceTester.headerFactory.createToHeader(toNameAddress,
                null);

        // create Request URI
        SipURI requestURI = PerformanceTester.addressFactory.createSipURI(toUser, "ot.bandwidth.com");

        // Create ViaHeaders

        ArrayList viaHeaders = new ArrayList();
        String ipAddress = listeningPoint.getIPAddress();
        ViaHeader viaHeader = PerformanceTester.headerFactory.createViaHeader(ipAddress,
                sipProvider.getListeningPoint(transport).getPort(),
                transport, null);

        // add via headers
        viaHeaders.add(viaHeader);

        // Create ContentTypeHeader
        ContentTypeHeader contentTypeHeader = PerformanceTester.headerFactory
                .createContentTypeHeader("application", "sdp");

        // Create a new CallId header
        CallIdHeader callIdHeader = sipProvider.getNewCallId();

        // Create a new Cseq header
        CSeqHeader cSeqHeader = PerformanceTester.headerFactory.createCSeqHeader(1L,
                Request.INVITE);

        // Create a new MaxForwardsHeader
        MaxForwardsHeader maxForwards = PerformanceTester.headerFactory
                .createMaxForwardsHeader(70);

        // Create the request.
        Request request = PerformanceTester.messageFactory.createRequest(requestURI,
                Request.INVITE, callIdHeader, cSeqHeader, fromHeader,
                toHeader, viaHeaders, maxForwards);
        // Create contact headers
        String host = ipAddress;

        SipURI contactUrl = PerformanceTester.addressFactory.createSipURI(fromName, host);
        contactUrl.setPort(listeningPoint.getPort());
      
        // Create the contact name address.
        SipURI contactURI = PerformanceTester.addressFactory.createSipURI(fromName, host);
        contactURI.setPort(sipProvider.getListeningPoint(transport)
                .getPort());

        Address contactAddress = PerformanceTester.addressFactory.createAddress(contactURI);

        // Add the contact address.
        contactAddress.setDisplayName(fromName);

        ContactHeader contactHeader = PerformanceTester.headerFactory
                .createContactHeader(contactAddress);
        request.addHeader(contactHeader);
        
        SipURI routeURI = PerformanceTester.addressFactory.createSipURI(null, PerformanceTester.bridgeConfiguration.getSipxProxyDomain());

        int proxyPort = PerformanceTester.bridgeConfiguration.getLocalPort();
        
        routeURI.setPort(proxyPort);
        routeURI.setLrParam();
        
        Address address = PerformanceTester.addressFactory.createAddress(routeURI);
        
        RouteHeader routeHeader = PerformanceTester.headerFactory.createRouteHeader(address);
        
        request.setHeader(routeHeader);
        
        String sdpData = PerformanceTester.createSessionDescription(listeningPoint.getIPAddress()).toString();
        byte[] contents = sdpData.getBytes();

        request.setContent(contents, contentTypeHeader);
       

        // Create the client transaction.
        ClientTransactionExt inviteTid = (ClientTransactionExt) sipProvider
                .getNewClientTransaction(request);

        // send the request out.
        inviteTid.sendRequest();
    }

}