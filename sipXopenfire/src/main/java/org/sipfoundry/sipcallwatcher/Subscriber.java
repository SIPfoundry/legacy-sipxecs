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
    private int count;
    private Dialog subscriberDialog;
    private Dialog forkedDialog;
    private ClientTransaction subscribeTransaction;
    private WatcherConfig watcherConfig;
    private Timer refreshTimer; // tracks the time when a re-subscribe is required to refresh the
    // subscription
    private Timer subscribeRetryTimer; // used to provoke retries when dialog-forming SUBSCRIBEs
    // fail to establish a dialog
    private Timer subscriptionExpiryTimer; // tracks the expiration of the subsctiption
    private ResourcesDialogInformation resourcesDialogInformation = new ResourcesDialogInformation();
    private boolean bSubscriptionActive;
    private int rlmiVersion;

    private ResourceStateChangeListener resourceStateChangeListener;
    private SipStackBean stackBean;
    private SipProvider sipProvider;

    // ////////////////
    // inner classes //
    // ///////////// //

    /**
     * Encapsulates information about the active dialogs of a specific resource.
     * 
     */
    class DialogInformation {
        /*
         * maps active dialog Ids to their state (confirmed, trying, early, proceeding or
         * terminated) according to RFC 4235
         */
        private Map<String, String> activeDialogStates = new HashMap<String, String>();
        /* state of a resource considering all its dialogs */
        private SipResourceState compoundState = SipResourceState.UNDETERMINED;
        /* internal Id used to track entries that have been updated */
        private int updateId = -1;
        private String resourceName;

        public DialogInformation(String resourceName) {
            logger.debug("creating dialog Information for " + resourceName);
            this.resourceName = resourceName;

        }

        public boolean updateDialogStates(boolean isFullState,
                Collection<DialogInfoMessagePart.DialogInfo> updatedDialogs, int updateId) {
            boolean hasStateChanged = false;
            this.updateId = updateId;
            if (updatedDialogs.size() == 0) {
                if (isFullState) {
                    // Full state of the resource is an empty list of dialogs...
                    // There are no dialogs associated with this resource so it
                    // is clearly idle.
                    activeDialogStates.clear();
                    if (!this.compoundState.equals(SipResourceState.IDLE)) {
                        hasStateChanged = true;
                        this.compoundState = SipResourceState.IDLE;
                    }
                }
            } else {
                if (isFullState) {
                    /*
                     * the list of dialogs is complete. We can replace all the dialogs we had with
                     * the ones supplied if any.
                     */
                    activeDialogStates.clear();
                }
                // update the dialogs we have with the ones supplied.
                for (DialogInfoMessagePart.DialogInfo dialogInfo : updatedDialogs) {
                    // we only track 'active' dialogs
                    if (!dialogInfo.getState().equals("terminated")) {
                        activeDialogStates.put(dialogInfo.getId(), dialogInfo.getState());
                    } else {
                        // dialog is terminated - discontinue its tracking
                        activeDialogStates.remove(dialogInfo.getId());
                    }
                }
                hasStateChanged = computeCompoundState();
            }

            logger.debug("resource " + this.resourceName + " hasStateChanged = "
                    + hasStateChanged);
            return hasStateChanged;
        }

        /**
         * Computes the state of a resource considering all its dialogs. This method sets the
         * value of instance variable 'compoundState' to either BUSY or IDLE accordingly.
         * 
         * @return - true if compoundState got changed
         */
        private boolean computeCompoundState() {
            boolean hasStateChanged = false;
            SipResourceState newCompoundState;
            if (activeDialogStates.size() > 0) {
                // at least one active dialog - that makes the resource busy.
                newCompoundState = SipResourceState.BUSY;
            } else {
                // no active dialogs - that makes the resource idle.
                newCompoundState = SipResourceState.IDLE;
            }
            // Set hasStateChanged flag to true and update the compound state
            // if a state change was detected.
            if (!this.compoundState.equals(newCompoundState)) {

                hasStateChanged = true;
                this.compoundState = newCompoundState;
                logger.debug("resource " + this.resourceName + " stateChanged ");
            }
            return hasStateChanged;
        }

        public SipResourceState getCompoundState() {
            return compoundState;
        }

        public int getUpdateId() {
            return updateId;
        }
    }

    /**
     * Encapsulates the dialog information for all the resources being monitored.
     * 
     */
    private class ResourcesDialogInformation {
        /* update counter - will be used to track the resources that have been updated */
        private int updateId = 1;
        /*
         * maps a resource name to DialogInformation object storing that resource's active dialogs
         * and overall state.
         */
        private Map<String, DialogInformation> resourcesDialogInfoMap = new HashMap<String, DialogInformation>();

        /**
         * Updates the state of all the resources based on the supplied dialog updates.
         * 
         * @param isFullState - was RLMI a full report or not?
         * @param dialogsUpdate - list of dialog info updates
         * @return - map for resources names that experienced a state change as the result of the
         *         update along with their new state.
         */
        public Map<String, SipResourceState> update(boolean isFullState,
                Collection<RlmiMultipartMessage.ResourceDialogsDescriptor> dialogsUpdate) {
            logger.debug("update : isFullState " + isFullState + " ndescriptors "
                    + dialogsUpdate.size());
            Map<String, SipResourceState> updatedResources = new HashMap<String, SipResourceState>();

            // update the resources one-by-one
            for (RlmiMultipartMessage.ResourceDialogsDescriptor dialogsDesc : dialogsUpdate) {
                SipResourceState newCompoundState;
                if ((newCompoundState = update(dialogsDesc)) != null) {
                    updatedResources.put(dialogsDesc.getResourceName(), newCompoundState);

                } else {
                    logger.debug("newCompoundState is null");
                }
            }

            // check whether this is a full state report or not.
            if (isFullState) {
                // now, discover all the resources that did not get updated. These represent
                // the resources that are no longer part of the full report which suggests that
                // they are no longer part of the set of resources that we need to worry about.
                // Find them and take them out and report their state to be changed to
                // 'undetermined'
                // if it is not already that.

                final Iterator<String> mapIter = resourcesDialogInfoMap.keySet().iterator();
                // do not use enhanced for-loop since we remove elements from the map while
                // ierating through it
                while (mapIter.hasNext()) {
                    String resourceName = mapIter.next();
                    DialogInformation dialogInfo = resourcesDialogInfoMap.get(resourceName);
                    if (dialogInfo.getUpdateId() < updateId) {
                        // this record did not get updated by the full report, remove it.
                        if (!dialogInfo.getCompoundState().equals(SipResourceState.UNDETERMINED)) {
                            updatedResources.put(resourceName, SipResourceState.UNDETERMINED);

                        }
                        mapIter.remove();
                    }
                }
            }
            updateId++;

            logger.debug("updatedResources = " + updatedResources);
            return updatedResources;
        }

        /**
         * Updates the state of a specific resource based on the supplied information
         * 
         * @param dialogsDesc - describes the dialog(s) for a specific resource. Note: a dialog
         *        descriptor with an empty set indicates that no dialogs are associated with the
         *        resource.
         * @return - if update caused a state change then a ResourceState representing the
         *         resource's new state is returned, otherwise null.
         */
        public SipResourceState update(RlmiMultipartMessage.ResourceDialogsDescriptor dialogsDesc) {
            try {
                SipResourceState newState = null;

                // locate the record for that resource in our resourcesDialogInfoMap
                DialogInformation dialogInfo = resourcesDialogInfoMap.get(dialogsDesc
                        .getResourceName());
                if (dialogInfo == null) {
                    dialogInfo = new DialogInformation(dialogsDesc.getResourceName());
                    resourcesDialogInfoMap.put(dialogsDesc.getResourceName(), dialogInfo);
                }

                if (dialogInfo.updateDialogStates(dialogsDesc.isFullState(), dialogsDesc
                        .getDialogsList(), updateId)) {
                    newState = dialogInfo.getCompoundState();
                }
                return newState;
            } catch (RuntimeException ex) {
                logger.error("Unexpected exception ", ex);
                throw ex;
            }
        }
    }

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
        this.transport = "udp";
        this.bSubscriptionActive = false;
        this.stackBean = stackBean;

        headerFactory = this.stackBean.getHeaderFactory();
        addressFactory = this.stackBean.getAddressFactory();
        messageFactory = this.stackBean.getMessageFactory();

    }

    public void setProvider(SipProvider provider) {
        this.sipProvider = provider;
    }

    public void start() {
        sendDialogFormingSubscribe();
    }

    public synchronized void processRequest(RequestEvent requestReceivedEvent) {
        Request request = requestReceivedEvent.getRequest();
        ServerTransaction serverTransactionId = requestReceivedEvent.getServerTransaction();
        String viaBranch = ((ViaHeader) (request.getHeaders(ViaHeader.NAME).next()))
                .getParameter("branch");

        logger.info("\n\nRequest " + request.getMethod() + " received at "
                + stackBean.getStackName() + " with server transaction id " + serverTransactionId
                + " branch ID = " + viaBranch);

        if (request.getMethod().equals(Request.NOTIFY)) {
            processNotify(requestReceivedEvent, serverTransactionId);
        }

    }

    public synchronized void processNotify(RequestEvent requestEvent,
            ServerTransaction serverTransactionId) {
        SipProvider provider = (SipProvider) requestEvent.getSource();
        Request notify = requestEvent.getRequest();
        try {
            logger.info("subscriber:  got a notify count  " + this.count++);
            if (serverTransactionId == null) {
                logger.info("subscriber:  null TID.");
                serverTransactionId = provider.getNewServerTransaction(notify);
            }
            Dialog dialog = serverTransactionId.getDialog();
            logger.info("Dialog = " + dialog);

            if (dialog != null) {
                logger.info("Dialog State = " + dialog.getState());
            }

            if (dialog != subscriberDialog) {
                if (forkedDialog == null) {
                    forkedDialog = dialog;
                } else {
                    if (forkedDialog != dialog) {
                        ((SIPTransactionStack) stackBean.getSipStack()).printDialogTable();
                    }
                    assert (forkedDialog == dialog);
                }
            }

            Response response = messageFactory.createResponse(200, notify);
            // SHOULD add a Contact
            ContactHeader contact = (ContactHeader) contactHeader.clone();
            ((SipURI) contact.getAddress().getURI()).setParameter("id", "sub");
            response.addHeader(contact);
            logger.info("Transaction State = " + serverTransactionId.getState());
            serverTransactionId.sendResponse(response);
            if (dialog != null) {
                logger.info("Dialog State = " + dialog.getState());
            }
            SubscriptionStateHeader subscriptionState = (SubscriptionStateHeader) notify
                    .getHeader(SubscriptionStateHeader.NAME);
            // Subscription is terminated?
            String state = subscriptionState.getState();
            Map<String, SipResourceState> updatedSipUsersStates = null;
            if (state.equalsIgnoreCase(SubscriptionStateHeader.TERMINATED)) {
                logger
                        .warn("Subscription in 'terminated' state - dropping dialog and resubscribing");
                dialog.delete();
            } else {
                logger.info("Subscriber: state now " + state);
                // check if this is the notification we are expecting...
                ContentTypeHeader contentType = (ContentTypeHeader) notify
                        .getHeader(ContentTypeHeader.NAME);
                if (contentType.getContentType().equals("multipart")
                        && contentType.getContentSubType().equals("related")
                        && contentType.getParameter("type").equals("application/rlmi+xml")) {
                    RlmiMultipartMessage rlmiMultipartMessage = new RlmiMultipartMessage(
                            new String(notify.getRawContent()), contentType
                                    .getParameter("boundary"));
                    // seed RLMI messages version number if not done already
                    if (rlmiVersion == -1) {
                        rlmiVersion = rlmiMultipartMessage.getVersion();
                        logger.debug("Synchronizing version to " + rlmiVersion);
                    } else {
                        // check if we have received the next version we were expecting
                        if (++rlmiVersion != rlmiMultipartMessage.getVersion()) {
                            // there is a hole in the versions which means that we
                            // missed an update. If this is not a full state report
                            // then delete the dialog to force a re-subscription to
                            // resync the state
                            if (!rlmiMultipartMessage.isFullState()) {
                                logger.error("Detected missing RLMI report -> resubscribing");
                                dialog.delete();
                                return;
                            }
                        }
                    }
                    updatedSipUsersStates = resourcesDialogInformation.update(
                            rlmiMultipartMessage.isFullState(), rlmiMultipartMessage
                                    .getUpdatedEntitiesStates());
                    for (String user : updatedSipUsersStates.keySet()) {
                        SipResourceState resourceState = updatedSipUsersStates.get(user);
                        // notify state change listener of change
                        if (Subscriber.this.resourceStateChangeListener != null) {
                            ResourceStateEvent resourceStateEvent = new ResourceStateEvent(
                                    Subscriber.this, user, resourceState);
                            Subscriber.this.resourceStateChangeListener
                                    .handleResourceStateChange(resourceStateEvent);
                        } else {
                            logger.debug("No listener registered " + user + " state "
                                    + resourceState);
                        }
                    }

                }
            }
            dumpResourceStates(updatedSipUsersStates);
        } catch (Exception ex) {
            logger.error("Unexpected exception", ex);
        }
    }

    private void dumpResourceStates(Map<String, SipResourceState> updatedSipUsersStates) {
        logger.debug("<StateDump>");
        for (Map.Entry<String, SipResourceState> kvPair : updatedSipUsersStates.entrySet()) {
            logger.debug("  Sip User:'" + kvPair.getKey() + "' -> new state="
                    + kvPair.getValue().name());
        }
        logger.debug("</StateDump>");
    }

    public synchronized void processResponse(ResponseEvent responseReceivedEvent) {
        try {
            Response response = (Response) responseReceivedEvent.getResponse();
            ClientTransaction tid = responseReceivedEvent.getClientTransaction();

            // test the response and act accordingly
            logger.info("Response received with client transaction id " + tid + ": "
                    + response.getStatusCode());
            if (tid == null) {
                logger.info("Stray response -- dropping ");
                return;
            }

            logger.info("transaction state is " + tid.getState());
            logger.info("Dialog = " + tid.getDialog());
            if (tid.getDialog() != null)
                logger.info("Dialog State is " + tid.getDialog().getState());

            if (tid == subscribeTransaction) {
                logger.debug("dialog and tid match: " + subscriberDialog.getDialogId());
                if (response.getStatusCode() == Response.UNAUTHORIZED) {
                    stackBean.handleChallenge(response, tid);
                } else if (response.getStatusCode() == Response.ACCEPTED
                        || response.getStatusCode() == Response.OK) {
                    // update dialog
                    subscriberDialog = tid.getDialog();

                    // subscription is accepted - kick off timer task that will
                    // refresh it before it expires and another that will
                    // warn us if it ever does.
                    ExpiresHeader expires = response.getExpires();
                    this.bSubscriptionActive = true;
                    if (subscribeRetryTimer != null)
                        subscribeRetryTimer.cancel(); // dialog formed - no need to retry now...
                    scheduleSubscriptionRefreshTimer(expires.getExpires() / 2);
                    scheduleSubscriptionExpiryTimer(expires.getExpires());
                }
            } else {
                logger.warn("response received on unknown transaction: " + tid.toString() + "\n"
                        + "                 expected on transaction: "
                        + subscribeTransaction.toString());
            }
        } catch (Exception ex) {
            logger.error("Caught exception: " + ex);
            ex.printStackTrace();
        }
    }

    private void scheduleSubscriptionRefreshTimer(int delayInSecs) {
        if (refreshTimer != null) {
            refreshTimer.cancel();
        }
        refreshTimer = new Timer();
        TimerTask task = new TimerTask() {
            public void run() {
                refreshSubscription();
            }
        };
        logger.debug("arming refreshTimer for " + delayInSecs);
        refreshTimer.schedule(task, delayInSecs * 1000);
    }

    private void scheduleSubscriptionExpiryTimer(int delayInSecs) {
        if (subscriptionExpiryTimer != null) {
            subscriptionExpiryTimer.cancel();
        }
        subscriptionExpiryTimer = new Timer();
        TimerTask task = new TimerTask() {
            public void run() {
                processSubscriptionExpired();
            }
        };
        logger.debug("arming subscriptionExpiryTimer for " + delayInSecs);
        subscriptionExpiryTimer.schedule(task, delayInSecs * 1000);
    }

    private void scheduleSubscribeRetryTimer(int delayInSecs) {
        if (subscribeRetryTimer != null) {
            subscribeRetryTimer.cancel();
        }
        subscribeRetryTimer = new Timer();
        TimerTask task = new TimerTask() {
            public void run() {
                reAttemptSubscription();
            }
        };
        logger.debug("arming subscribeRetryTimer " + delayInSecs);
        subscribeRetryTimer.schedule(task, delayInSecs * 1000);
    }

    private synchronized void refreshSubscription() {
        logger.debug("refresh timer fired");
        // send a new subscription to refresh it and kick off a shorter
        // refresh timer in case it refresh fails
        try {
            logger.info("refreshing subscription");
            sendInDialogSubscribe();

            // kick off another refresh time in case this refresh attempt fails
            scheduleSubscriptionRefreshTimer(20);
        } catch (Exception ex) {
            logger.error("Caught: ", ex);
        }
    }

    private synchronized void reAttemptSubscription() {
        if (this.bSubscriptionActive == false) {
            logger.warn("retrying subscribe");
            sendDialogFormingSubscribe();
        }
    }

    private synchronized void processSubscriptionExpired() {
        // subscription expired - no point in refreshing it now. Cancel refresh timer
        if (refreshTimer != null)
            refreshTimer.cancel();
        this.bSubscriptionActive = false;
        logger.warn("subscription expired");
        sendDialogFormingSubscribe();
    }

    public synchronized void sendDialogFormingSubscribe() {
        try {
            // this will initiate a new subscription that will
            // come it with its own RLMI version number - reset
            // our RLMI version tracker count to force a resync
            // once the subscription gets re-established
            rlmiVersion = -1;

            // create >From Header
            String fromName = watcherConfig.getUserName();
            String fromSipAddress = watcherConfig.getProxyDomain();
            String fromDisplayName = "Call Watcher";

            SipURI fromAddress = addressFactory.createSipURI(fromName, fromSipAddress);

            Address fromNameAddress = addressFactory.createAddress(fromAddress);
            fromNameAddress.setDisplayName(fromDisplayName);
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
            int port = CallWatcher.getConfig().getWatcherPort();
            ViaHeader viaHeader = headerFactory.createViaHeader(
                    watcherConfig.getWatcherAddress(), watcherConfig.getProxyPort(), transport,
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

            addDialogEventHeaders(request);

            // Create the client transaction.
            this.subscribeTransaction = sipProvider.getNewClientTransaction(request);
            logger.info("Subscribe Dialog = " + subscribeTransaction.getDialog());

            this.subscriberDialog = subscribeTransaction.getDialog();
            // send the request out.
            TransactionContext.attach(subscribeTransaction, Operator.SEND_SUBSCRIBE);
            subscribeTransaction.sendRequest();
            scheduleSubscribeRetryTimer(32);
        } catch (SipException e) {
            logger.error("Error sending Subscribe -- tearing down dialog");
            if (this.subscribeTransaction != null) {
                this.subscribeTransaction.getDialog().delete();
            }
        } catch (ParseException ex) {
            logger.fatal("Unexpected exception", ex);
            throw new CallWatcherException(ex);
        } catch (InvalidArgumentException ex) {
            logger.fatal("Unexpected exception", ex);
            throw new CallWatcherException(ex);
        }
    }

    public void sendInDialogSubscribe() {
        try {
            Request request = subscriberDialog.createRequest(Request.SUBSCRIBE);
            addDialogEventHeaders(request);
            this.subscribeTransaction = sipProvider.getNewClientTransaction(request);
            this.subscriberDialog.sendRequest(subscribeTransaction);
            addDialogEventHeaders(request);
        } catch (SipException ex) {
            this.subscriberDialog.delete();
        }
    }

    private static void addDialogEventHeaders(Request request) {
        try {
            // Add a expires header
            ExpiresHeader expiresHeader = headerFactory.createExpiresHeader(3600);
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

    public void processIOException(IOExceptionEvent exceptionEvent) {
        logger.info("io exception event recieved");
        this.subscriberDialog.delete();
        this.subscribeRetryTimer.cancel();
    }

    public void processTransactionTerminated(TransactionTerminatedEvent transactionTerminatedEvent) {

    }

    public synchronized void processDialogTerminated(DialogTerminatedEvent dialogTerminatedEvent) {
        if (dialogTerminatedEvent.getDialog() == this.subscriberDialog) {
            logger.info("dialog terminated event received: "
                    + dialogTerminatedEvent.getDialog().getDialogId());
            if (refreshTimer != null)
                refreshTimer.cancel();
            if (subscribeRetryTimer != null)
                subscribeRetryTimer.cancel();
            if (subscriptionExpiryTimer != null)
                subscriptionExpiryTimer.cancel();

            // we just lost our subscription dialog - all bets are off
            // WRT the state of the resources we were monitoring.
            // Force an update that will put all the monitored resources
            // in an undetermined state;
            Map<String, SipResourceState> updatedSipUsersStates = null;
            updatedSipUsersStates = resourcesDialogInformation.update(true, emptyCollection);
            dumpResourceStates(updatedSipUsersStates);

            // initiate a new subscription to replace the dialog we just lost.
            sendDialogFormingSubscribe();
        }
    }

    public void processTimeout(javax.sip.TimeoutEvent timeoutEvent) {

        /*
         * We should do something here - maybe tear down the subscription
         */
        logger.info("Transaction Time out");
        this.subscriberDialog.delete();

    }

    /**
     * @param resourceStateChangeListener the resourceStateChangeListener to set
     */
    public void setResourceStateChangeListener(
            ResourceStateChangeListener resourceStateChangeListener) {
        this.resourceStateChangeListener = resourceStateChangeListener;
    }

}
