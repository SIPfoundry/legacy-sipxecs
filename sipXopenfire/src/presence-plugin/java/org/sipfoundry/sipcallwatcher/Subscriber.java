/*
 * Copyright (C) 2010 Avaya, certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 */
package org.sipfoundry.sipcallwatcher;

import gov.nist.javax.sip.SipStackExt;
import gov.nist.javax.sip.clientauthutils.AccountManager;
import gov.nist.javax.sip.clientauthutils.AuthenticationHelper;
import gov.nist.javax.sip.clientauthutils.SecureAccountManager;
import gov.nist.javax.sip.clientauthutils.UserCredentials;
import gov.nist.javax.sip.stack.SIPTransactionStack;

import java.text.ParseException;
import java.util.Collection;
import java.util.HashMap;
import java.util.Iterator;
import java.util.Map;
import java.util.Timer;
import java.util.TimerTask;
import java.util.ArrayList;

import javax.sip.ClientTransaction;
import javax.sip.Dialog;
import javax.sip.DialogTerminatedEvent;
import javax.sip.IOExceptionEvent;
import javax.sip.InvalidArgumentException;
import javax.sip.ListeningPoint;
import javax.sip.RequestEvent;
import javax.sip.ResponseEvent;
import javax.sip.ServerTransaction;
import javax.sip.SipException;
import javax.sip.SipFactory;
import javax.sip.SipListener;
import javax.sip.SipProvider;
import javax.sip.TimeoutEvent;
import javax.sip.TransactionTerminatedEvent;
import javax.sip.address.Address;
import javax.sip.address.AddressFactory;
import javax.sip.address.SipURI;
import javax.sip.header.AcceptHeader;
import javax.sip.header.CSeqHeader;
import javax.sip.header.CallIdHeader;
import javax.sip.header.ContactHeader;
import javax.sip.header.ContentTypeHeader;
import javax.sip.header.EventHeader;
import javax.sip.header.ExpiresHeader;
import javax.sip.header.FromHeader;
import javax.sip.header.HeaderFactory;
import javax.sip.header.MaxForwardsHeader;
import javax.sip.header.SubscriptionStateHeader;
import javax.sip.header.SupportedHeader;
import javax.sip.header.ToHeader;
import javax.sip.header.ViaHeader;
import javax.sip.message.MessageFactory;
import javax.sip.message.Request;
import javax.sip.message.Response;

import org.apache.log4j.Logger;
import org.sipfoundry.commons.userdb.ValidUsersXML;
import org.sipfoundry.openfire.config.WatcherConfig;
import org.sipfoundry.sipcallwatcher.SubscribeDialog.SubscribeDialog;
import org.sipfoundry.sipcallwatcher.DialogInfoMessagePart.EndpointInfo;


/**
 * Class implementing the SipListener interface and is responsible for creating and refreshing its
 * subscription with a resource list server and track the state of the monitored resources.
 */
public class Subscriber implements SipListener {
    // //////////////// //
    // static variables //
    // //////////////// //
    private static Logger logger = Logger.getLogger(Subscriber.class);
    private static AddressFactory addressFactory;
    private static MessageFactory messageFactory;
    private static HeaderFactory headerFactory;
    private static Collection<RlmiMultipartMessage.ResourceDialogsDescriptor> emptyCollection = new ArrayList<RlmiMultipartMessage.ResourceDialogsDescriptor>();

    // ////////////////// //
    // instance variables //
    // ////////////////// //
    private ContactHeader contactHeader;
    private String transport;
    private WatcherConfig watcherConfig;
    private ResourcesDialogInformation resourcesDialogInformation = new ResourcesDialogInformation();
    private SubscribeDialog activeSubscribeDialog = new SubscribeDialog( this );
    private ResourceStateChangeListener resourceStateChangeListener;
    private SipStackBean stackBean;
    private SipProvider sipProvider;

    /**
     * Object responsible for initiating and maintaining a resource list registration with a
     * resource list server and track the call state of the monitored resource based on the
     * content of the NOTIFYs received.
     * 
     * Note: this implementation does not handle downstream forks, i.e. it expects the
     * dialog-forming SUBSCRIBE it sends to establish a single dialog. The subscriptions here
     * subscribe to a resource list and those will not be downstream forked so the limitation is
     * expected to be acceptable.
     * 
     * @param sipProvider
     * @param watcherConfig
     */
    Subscriber(SipStackBean stackBean) {

        this.watcherConfig = CallWatcher.getConfig();
        this.transport = "tcp";
        this.stackBean = stackBean;

        headerFactory = this.stackBean.getHeaderFactory();
        addressFactory = this.stackBean.getAddressFactory();
        messageFactory = this.stackBean.getMessageFactory();
    }

    public void setProvider(SipProvider provider) {
        this.sipProvider = provider;
    }

    public SipProvider getProvider() {
        return this.sipProvider;
    }
    
    public SipStackBean getSipStackBean() {
        return this.stackBean;
    }
    
    public void start() {
        activeSubscribeDialog.start();        
    }

    public void stop() {
        // no need to explicitly stop activeSubscribeDialog - no real resources consumed
        activeSubscribeDialog = null;
    }

    public void processRequest(RequestEvent requestReceivedEvent) {
        if (requestReceivedEvent.getRequest().getMethod().equals(Request.NOTIFY)) {
            activeSubscribeDialog.processNotifyRequest(requestReceivedEvent);
        }
    }
                
    public void processResponse(ResponseEvent responseReceivedEvent) { 
        activeSubscribeDialog.processResponse(responseReceivedEvent);
    }
    
    public void processDialogTerminated(DialogTerminatedEvent dialogTerminatedEvent) {
        activeSubscribeDialog.processDialogTerminated(dialogTerminatedEvent);
    }

    public void processTransactionTerminated( TransactionTerminatedEvent transactionTerminatedEvent ){
        activeSubscribeDialog.processTransactionTerminated(transactionTerminatedEvent);
    }

    public int processNotifyBody( Request notify, int expectedRlmiVersion ) throws CallWatcherException{
        Map<String, SipResourceState> updatedSipUsersStates = null;
        // check if this is the NOTIFY carries the content type we are expecting...
        ContentTypeHeader contentType = (ContentTypeHeader) notify.getHeader(ContentTypeHeader.NAME);
        if (contentType.getContentType().equals("multipart") && 
            contentType.getContentSubType().equals("related") && 
            contentType.getParameter("type").equals("application/rlmi+xml")) 
        {
            
            RlmiMultipartMessage rlmiMultipartMessage;
            try
            {
                rlmiMultipartMessage = new RlmiMultipartMessage(new String(notify.getRawContent()), contentType.getParameter("boundary"));
            }
            catch (Exception e)
            {
                logger.info(e);
                return expectedRlmiVersion;
            }
            
            // seed RLMI messages version number if not done already
            if (expectedRlmiVersion == -1) {
                expectedRlmiVersion = rlmiMultipartMessage.getVersion();
                logger.debug("Synchronizing version to " + expectedRlmiVersion);
             } 
            else {
                // check if we have received the next version we were expecting
                if (expectedRlmiVersion != rlmiMultipartMessage.getVersion()) {
                    // there is a hole in the versions which means that we
                    // missed an update. If this is not a full state report
                    // then throw an exception as some information will be missing
                    if (!rlmiMultipartMessage.isFullState()) {
                        logger.error("Detected missing RLMI report: expected " + expectedRlmiVersion + " but received " + rlmiMultipartMessage.getVersion());
                        throw new CallWatcherException( "Detected missing RLMI report: expected " + expectedRlmiVersion + " but received " + rlmiMultipartMessage.getVersion() );
                    }
                    else{
                        // no loss of information - simply resync number
                        expectedRlmiVersion = rlmiMultipartMessage.getVersion();
                    }
                }
            }
            updatedSipUsersStates = resourcesDialogInformation.update(
                    rlmiMultipartMessage.isFullState(), rlmiMultipartMessage.getUpdatedEntitiesStates());
            for (String user : updatedSipUsersStates.keySet()) {
                SipResourceState resourceState = updatedSipUsersStates.get(user);
                // notify state change listener of change
                if (Subscriber.this.resourceStateChangeListener != null) {
                    EndpointInfo remote = resourceState.equals( SipResourceState.BUSY ) ? 
                            resourcesDialogInformation.getRemoteInfoForActiveDialog(user) : null;            
                    ResourceStateEvent resourceStateEvent = new ResourceStateEvent(
                            Subscriber.this, user, resourceState, remote);
                    Subscriber.this.resourceStateChangeListener
                            .handleResourceStateChange(resourceStateEvent);
                } else {
                    logger.debug("No listener registered " + user + " state "
                            + resourceState);
                }
            }
            dumpResourceStates(updatedSipUsersStates);
        }
        return expectedRlmiVersion;
    }

    private void dumpResourceStates(Map<String, SipResourceState> updatedSipUsersStates) {
        logger.debug("<StateDump>");
        for (Map.Entry<String, SipResourceState> kvPair : updatedSipUsersStates.entrySet()) {
            logger.debug("  Sip User:'" + kvPair.getKey() + "' -> new state="
                    + kvPair.getValue().name());
        }
        logger.debug("</StateDump>");
    }

    public ClientTransaction sendDialogFormingSubscribe() throws CallWatcherException{
        ClientTransaction clientTransaction = null;
        try {
            // create >From Header
            String fromName = watcherConfig.getUserName();
            String fromSipAddress = watcherConfig.getProxyDomain();
            SipURI fromAddress = addressFactory.createSipURI(fromName, fromSipAddress);
            Address fromNameAddress = addressFactory.createAddress(fromAddress);
            fromNameAddress.setDisplayName("Call Watcher");
            FromHeader fromHeader = headerFactory.createFromHeader(fromNameAddress, new Long(Math
                    .abs(new java.util.Random().nextLong())).toString());

            // create To Header
            // Typically this will be ~~rl~F~fromName
            String toUser = watcherConfig.getResourceList();
            String toSipAddress = watcherConfig.getProxyDomain();
            SipURI toAddress = addressFactory.createSipURI(toUser, toSipAddress);
            Address toNameAddress = addressFactory.createAddress(toAddress);
            ToHeader toHeader = headerFactory.createToHeader(toNameAddress, null);

            // create Request URI
            SipURI requestURI = addressFactory.createSipURI(toUser, toSipAddress);

            // Create ViaHeaders
            // TODO: do this right
            ArrayList viaHeaders = new ArrayList();
            ViaHeader viaHeader = headerFactory.createViaHeader(
                    watcherConfig.getWatcherAddress(), watcherConfig.getWatcherPort(), transport,
                    null);

            // add via headers
            viaHeaders.add(viaHeader);

            // Create a new CallId header
            CallIdHeader callIdHeader = sipProvider.getNewCallId();

            // Create a new Cseq header
            CSeqHeader cSeqHeader = headerFactory.createCSeqHeader(1L, Request.SUBSCRIBE);

            // Create a new MaxForwardsHeader
            MaxForwardsHeader maxForwards = headerFactory.createMaxForwardsHeader(70);

            // Create the request.
            Request request = messageFactory.createRequest(requestURI, Request.SUBSCRIBE,
                    callIdHeader, cSeqHeader, fromHeader, toHeader, viaHeaders, maxForwards);

            // Create contact headers
            String host = CallWatcher.getConfig().getWatcherAddress();

            SipURI contactUrl = addressFactory.createSipURI(fromName, host);
            contactUrl.setPort(CallWatcher.getConfig().getWatcherPort());

            // Create the contact name address.
            SipURI contactURI = addressFactory.createSipURI(fromName, host);
            contactURI.setTransportParam(transport);
            contactURI.setPort(CallWatcher.getConfig().getWatcherPort());

            Address contactAddress = addressFactory.createAddress(contactURI);

            // Add the contact address.
            contactAddress.setDisplayName(fromName);

            contactHeader = headerFactory.createContactHeader(contactAddress);
            request.addHeader(contactHeader);

            addDialogEventHeaders(request, 3600);

            // Create the client transaction.
            clientTransaction = sipProvider.getNewClientTransaction(request);
            // send the request out.
            TransactionContext.attach(clientTransaction, Operator.SEND_SUBSCRIBE);
            clientTransaction.sendRequest();
        } catch (SipException ex) {
            logger.error("Error sending Subscribe -- tearing down dialog");
            if (clientTransaction != null) {
                clientTransaction.getDialog().delete();
            }
            throw new CallWatcherException(ex);
        } catch (ParseException ex) {
            logger.fatal("Unexpected parse exception", ex);
            throw new CallWatcherException(ex);
        } catch (InvalidArgumentException ex) {
            logger.fatal("Unexpected argument exception", ex);
            throw new CallWatcherException(ex);
        }
        return clientTransaction;
    }
    
    public void constructAndSendResponseForRequest( Request request, int statusCode, ServerTransaction serverTransaction ) throws CallWatcherException{
        try{
            Response response = messageFactory.createResponse(statusCode, request);
            ContactHeader contact = (ContactHeader) contactHeader.clone();
            response.addHeader(contact);
            serverTransaction.sendResponse(response);
        }
        catch( Exception e){
            throw new CallWatcherException(e);
        }
    }

    public ClientTransaction sendInDialogSubscribe( Dialog dialog, int expiresValueInSecs) throws CallWatcherException{
        ClientTransaction clientTransaction = null;
        try {
            Request request = dialog.createRequest(Request.SUBSCRIBE);
            addDialogEventHeaders(request, expiresValueInSecs);
            clientTransaction = sipProvider.getNewClientTransaction(request);
            dialog.sendRequest(clientTransaction);
        } catch (Exception ex) {
            logger.debug("Subscriber::sendInDialogSubscribe caught exception:", ex );
            throw new CallWatcherException("Failed to send in-dialog Subscribe:" + ex );
        }
        return clientTransaction;
    }

    private static void addDialogEventHeaders(Request request, int expiresValueInSecs) {
        try {
            // Add a expires header
            ExpiresHeader expiresHeader = headerFactory.createExpiresHeader(expiresValueInSecs);
            request.addHeader(expiresHeader);

            // Add supported header
            SupportedHeader supportedHeader = headerFactory.createSupportedHeader("eventlist");
            request.addHeader(supportedHeader);

            // Add event header
            EventHeader eventHeader = headerFactory.createEventHeader("dialog");
            request.addHeader(eventHeader);

            // Add Accept headers
            AcceptHeader acceptHeader = headerFactory.createAcceptHeader("application",
                    "dialog-info+xml");
            request.addHeader(acceptHeader);
            acceptHeader = headerFactory.createAcceptHeader("application", "rlmi+xml");
            request.addHeader(acceptHeader);
            acceptHeader = headerFactory.createAcceptHeader("multipart", "related");
            request.addHeader(acceptHeader);
        } catch (Exception ex) {
            logger.error("Caught: ", ex);
            throw new CallWatcherException(ex);
        }
    }


    /**
     * @param resourceStateChangeListener the resourceStateChangeListener to set
     */
    public void setResourceStateChangeListener(
            ResourceStateChangeListener resourceStateChangeListener) {
        this.resourceStateChangeListener = resourceStateChangeListener;
    }

    public void notifySubscriptionDialogTerminated( SubscribeDialog subscribeDialog )
    {
        if( subscribeDialog == this.activeSubscribeDialog ){
            // state of all resources is unknown at this point, update accordingly.
            Map<String, SipResourceState> updatedSipUsersStates = null;
            updatedSipUsersStates = resourcesDialogInformation.update(true, emptyCollection);
            dumpResourceStates(updatedSipUsersStates);
            scheduleNewSubscribeDialogCreation();
        }
        else{
            logger.info("Subscriber::notifySubscriptionDialogTerminated received from SubscribeDialog that wasn't active: " + subscribeDialog +
                        " (currently active SubscribeDialog = " + this.activeSubscribeDialog + ")" );
        }
    }

    private void scheduleNewSubscribeDialogCreation()
    {
        Timer subscribeDialogCreator = new Timer();
        TimerTask task = new TimerTask() {
            public void run() {
                logger.info("subscribeDialogCreator timer fired");
                activeSubscribeDialog = new SubscribeDialog(Subscriber.this);
                activeSubscribeDialog.start();
            }
        };
        logger.info("arming subscribeDialogCreator");
        subscribeDialogCreator.schedule(task, 10000);
    }

    public void processIOException( IOExceptionEvent arg0 ){}

    public void processTimeout( TimeoutEvent arg0 ){}

}
