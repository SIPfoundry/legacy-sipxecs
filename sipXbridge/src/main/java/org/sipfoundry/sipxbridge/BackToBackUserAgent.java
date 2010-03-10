/*
 *  Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 *  Contributors retain copyright to elements licensed under a Contributor Agreement.
 *  Licensed to the User under the LGPL license.
 *
 */
package org.sipfoundry.sipxbridge;

import gov.nist.javax.sip.ClientTransactionExt;
import gov.nist.javax.sip.DialogExt;
import gov.nist.javax.sip.SipStackExt;
import gov.nist.javax.sip.TransactionExt;
import gov.nist.javax.sip.header.HeaderFactoryExt;
import gov.nist.javax.sip.header.extensions.ReferencesHeader;
import gov.nist.javax.sip.header.extensions.ReferredByHeader;
import gov.nist.javax.sip.header.extensions.ReplacesHeader;
import gov.nist.javax.sip.message.SIPMessage;
import gov.nist.javax.sip.stack.SIPTransaction;

import java.io.IOException;
import java.net.URLDecoder;
import java.text.ParseException;
import java.util.Collection;
import java.util.HashSet;
import java.util.Iterator;
import java.util.LinkedList;
import java.util.List;
import java.util.Random;
import java.util.Set;
import java.util.TimerTask;

import javax.sdp.SdpParseException;
import javax.sdp.SessionDescription;
import javax.sip.ClientTransaction;
import javax.sip.Dialog;
import javax.sip.DialogState;
import javax.sip.InvalidArgumentException;
import javax.sip.ListeningPoint;
import javax.sip.RequestEvent;
import javax.sip.ServerTransaction;
import javax.sip.SipException;
import javax.sip.SipProvider;
import javax.sip.TransactionAlreadyExistsException;
import javax.sip.TransactionState;
import javax.sip.TransactionUnavailableException;
import javax.sip.address.Address;
import javax.sip.address.Hop;
import javax.sip.address.SipURI;
import javax.sip.header.AlertInfoHeader;
import javax.sip.header.AuthorizationHeader;
import javax.sip.header.CSeqHeader;
import javax.sip.header.CallIdHeader;
import javax.sip.header.CallInfoHeader;
import javax.sip.header.ContactHeader;
import javax.sip.header.ContentTypeHeader;
import javax.sip.header.DateHeader;
import javax.sip.header.ExtensionHeader;
import javax.sip.header.FromHeader;
import javax.sip.header.Header;
import javax.sip.header.InReplyToHeader;
import javax.sip.header.MaxForwardsHeader;
import javax.sip.header.OrganizationHeader;
import javax.sip.header.ProxyAuthorizationHeader;
import javax.sip.header.ReasonHeader;
import javax.sip.header.RecordRouteHeader;
import javax.sip.header.ReferToHeader;
import javax.sip.header.ReplyToHeader;
import javax.sip.header.RouteHeader;
import javax.sip.header.SubjectHeader;
import javax.sip.header.ToHeader;
import javax.sip.header.ViaHeader;
import javax.sip.header.WarningHeader;
import javax.sip.message.Request;
import javax.sip.message.Response;

import org.apache.log4j.Logger;
import org.sipfoundry.commons.siprouter.ProxyHop;
import org.sipfoundry.sipxrelay.BridgeInterface;
import org.sipfoundry.sipxrelay.BridgeState;
import org.sipfoundry.sipxrelay.KeepaliveMethod;
import org.sipfoundry.sipxrelay.SymImpl;
import org.sipfoundry.sipxrelay.SymmitronClient;
import org.sipfoundry.sipxrelay.SymmitronException;


/**
 * A class that represents an ongoing call. An ongoing call maps to exactly one instance of this
 * structure. Creation and destruction of instances of this class is managed by the
 * BackToBackUserAgentFactory. When we receive an incoming call we retrieve the corresponding
 * instance of this structure and route the request to it. This structure keeps the necessary
 * state to handle subsequent requests that are related to this call. This structure manages a
 * collection of dialogs. The dialogs may be paired with each other to form back to back user
 * agents. At any given time there is a single pair of dialogs that is the actual active B2BUA. As
 * transfers take place, the pairing may change. Eventually when the call is torn down, all the
 * dialogs managed by this structure are destroyed.
 *
 * @author M. Ranganathan
 *
 */
public class BackToBackUserAgent implements Comparable {
    private static Logger logger = Logger.getLogger(BackToBackUserAgent.class);

    /*
     * Constants that we stick into the VIA header to detect spirals.
     */
    static final String ORIGINATOR = "sipxecs-id";

    /*
     * Since we have only one media type we have only one bridge. If we handle multiple media
     * types we will need to store a hashtable here indexed by media type.
     */
    private RtpBridge rtpBridge;

    /*
     * This is just a table of dialogs that reference this B2bua. When the table is empty the
     * b2bua is GCd.
     */
    HashSet<Dialog> dialogTable = new HashSet<Dialog>();

    /*
     * The REFER dialog currently in progress.
     */
    private Dialog referingDialog;

    /*
     * The peer of the Refer dialog.
     */
    private Dialog referingDialogPeer;

    /*
    * A counter that is incremented each time we see a Authentication header.
    */
    private int baseCounter;

    /*
     * For generation of call ids ( so we can filter logs easily)
     * the creating callid is appended with baseCounter + "." counter.
     * The counter is incremented each time we want to generate a new call id.
     */
    private int counter ;

    /*
     * Any call Id associated with this b2bua will be derived from this base call id. This makes
     * it easy to filter logs (you only need specify this prefix).
     */
    private String creatingCallId;

    /*
     * The call IDs that are associated with this b2bua. This is the pool that is consulted when
     * we associate inbound dialog forming requests with an instance of this class.
     */
    private HashSet<String> myCallIds = new HashSet<String>();

    /*
     * Handle to the relay.
     */
    private SymmitronClient symmitronClient;

    /*
     * The server handle returned to us when we sign into the symmitron.
     */
    private String symmitronServerHandle;

    /*
     * A given B2BUA has one pair of dialogs one for the WAN and one for the LAN and additionally
     * possibly a MOH dialog which is signaled when the WAN side is put on hold.
     */

    private Dialog musicOnHoldDialog;

    /*
     * Set to true if MOH is disabled ( no INIVTE will be sent to MOH ).
     */

    private boolean mohDisabled;

    private Hop proxyAddress;

    /*
     * This structure is pending garbage collection. There is a scanning thread that runs in the
     * BackToBackUserAgentFactory that will garbage collect this structure when it is marked
     * "pendingTermination".
     */
    private boolean pendingTermination;

    /*
     * List of Proxy servers that we have already tried. This is for DNS SRV handling. We do an
     * initial lookup and try the next one on timeout. Once we have exhausted the list we can time
     * out the operation.
     */
    private HashSet<Hop> blackListedProxyServers = new HashSet<Hop>();

    /*
     * Dialogs that are not tracked for garbage collection.
     */
    private HashSet<Dialog> cleanupList = new HashSet<Dialog>();
    
    

  
  
   
    // ////////////////////////////////////////////////////////////////////////
    // Inner classes.
    // ////////////////////////////////////////////////////////////////////////

    public class DelayedByeSender extends TimerTask {
        Dialog dialog;
        ServerTransaction serverTransaction;

        public DelayedByeSender(Dialog dialog, ServerTransaction serverTransaction) {
            this.dialog = dialog;
            this.serverTransaction = serverTransaction;
        }

        public void run() {
            try {
                if (dialog.getState() == DialogState.CONFIRMED && serverTransaction != null) {
                    TransactionContext.attach(serverTransaction, Operation.PROCESS_BYE);
                    DialogContext.get(dialog).forwardBye(serverTransaction);
                }
            } catch (Exception ex) {
                logger.error("Exception forwarding BYE.",ex);
            }
        }
    }

    // ///////////////////////////////////////////////////////////////////////
    // Constructor.
    // ///////////////////////////////////////////////////////////////////////
    @SuppressWarnings("unused")
    private BackToBackUserAgent() {

    }

    BackToBackUserAgent(SipProvider provider, Request request, Dialog dialog,
            ItspAccountInfo itspAccountInfo) throws IOException {
        int load = 0;

        /*
         * Received this request from the LAN The symmitron to use is the symmitron that runs
         * where the request originated from. If received this request from the LAN side then we
         * look at the topmost via header and try to contact the server at that address.
         */
        if (provider == Gateway.getLanProvider()) {
            ViaHeader viaHeader = (ViaHeader) request.getHeader(ViaHeader.NAME);
            /*
             * If we have a received header, then use that header to look for a symmmitron there.
             * Otherwise use the Via header. The symmitron must be up and running by the time the
             * request is seen at the server.
             */
            String address = (viaHeader.getReceived() != null ? viaHeader.getReceived()
                    : viaHeader.getHost());
            this.symmitronClient = Gateway.getSymmitronClient(address);
            this.proxyAddress = new ProxyHop(address, viaHeader.getPort(), viaHeader
                    .getTransport());

        } else {
            this.findNextSipXProxy();
            if (this.proxyAddress == null) {
                throw new IOException(
                        "Could not locate a sipx proxy server -- cannot create B2BUA");
            }
            logger.debug("routing call to " + this.proxyAddress.getHost() + ":"
                    + this.proxyAddress.getPort() + " relay load = " + load);
        }

        BridgeInterface bridge = symmitronClient.createBridge();
        this.symmitronServerHandle = symmitronClient.getServerHandle();
        rtpBridge = new RtpBridge(request, bridge);
        if (itspAccountInfo == null || !itspAccountInfo.stripPrivateHeaders()) {
            /*
             * If privacy is not desired we use the incoming callid and generate new call ids off
             * that one. This makes life a bit easier when extracting traces.
             */
            this.creatingCallId = ((CallIdHeader) request.getHeader(CallIdHeader.NAME))
                    .getCallId();
        } else {
            /*
             * If privacy IS desired then generate a new random call id header.
             */
            this.creatingCallId = provider.getNewCallId().getCallId();
        }
        

    }

    // ////////////////////////////////////////////////////////////////////////
    // Package local methods.
    // ////////////////////////////////////////////////////////////////////////

    /**
     * Create an RTP session for a dialog.
     *
     * RTP session that is connected to the WAN Side. Note that we use a different method here
     * because we need to record additionally whether or not to use global addressing in the RTP
     * session descriptor associated with the receiver.
     *
     * @return the rtp session created.
     */

    RtpSession createRtpSession(Dialog dialog) {
        RtpSession rtpSession = DialogContext.getRtpSession(dialog);
        SipProvider provider = ((DialogExt) dialog).getSipProvider();
        DialogContext dialogContext = DialogContext.get(dialog);

        if (rtpSession == null) {
            String clid = DialogContext.get(dialog).getCallLegId();
            for (Dialog dlg : this.dialogTable) {
                if (DialogContext.get(dlg).getCallLegId().equals(clid)) {
                    rtpSession = DialogContext.get(dlg).getRtpSession();
                    if (rtpSession != null) {
                        dialogContext.setRtpSession(rtpSession);
                        return rtpSession;
                    }
                }
            }

            if (Gateway.getLanProvider() == provider) {
                if (dialogContext.getRtpSession() == null) {
                        SymImpl symImpl = symmitronClient.createEvenSym();
                        rtpSession = new RtpSession(symImpl);
                        this.rtpBridge.addSym(rtpSession);
                        dialogContext.setRtpSession(rtpSession);
                   
                }
            } else {
                SymImpl symImpl = symmitronClient.createEvenSym();
                rtpSession = new RtpSession(symImpl);
                this.rtpBridge.addSym(rtpSession);
                rtpSession.getReceiver().setGlobalAddress(symmitronClient.getPublicAddress());
                rtpSession.getReceiver().setUseGlobalAddressing(
                            dialogContext.getItspInfo() == null
                                    || dialogContext.getItspInfo().isGlobalAddressingUsed());
                dialogContext.setRtpSession(rtpSession);
            }
        }
        return dialogContext.getRtpSession();
    }

    /**
     * This method handles an Invite with a replaces header in it. It is invoked for consultative
     * transfers. It does a Dialog splicing and media splicing on the Refer dialog. Here is the
     * reference call flow for this case:
     *
     * <pre>
     *
     *  Transferor           Transferee             Transfer
     *              |                    |                  Target
     *              |                    |                    |
     *    dialog1   | INVITE/200 OK/ACK F1 F2                 |
     *             	|&lt;-------------------|                    |
     *    dialog1   | INVITE (hold)/200 OK/ACK                |
     *              |-------------------&gt;|                    |
     *    dialog2   | INVITE/200 OK/ACK F3 F4                 |
     *              |----------------------------------------&gt;|
     *    dialog2   | INVITE (hold)/200 OK/ACK                |
     *              |----------------------------------------&gt;|
     *    dialog3   | REFER (Target-Dialog:2,                 |
     *              |  Refer-To:sips:Transferee?Replaces=1) F5|
     *              |----------------------------------------&gt;|
     *    dialog3   | 202 Accepted       |                    |
     *              |&lt;----------------------------------------|
     *    dialog3   | NOTIFY (100 Trying)|                    |
     *              |&lt;----------------------------------------|
     *    dialog3   |                    |            200 OK  |
     *              |----------------------------------------&gt;|
     *    dialog4   |         INVITE (Replaces:dialog1)/200 OK/ACK F6
     *              |                    |&lt;-------------------|
     *    dialog1   | BYE/200 OK         |                    |
     *              |&lt;-------------------|                    |
     *    dialog3   | NOTIFY (200 OK)    |                    |
     *              |&lt;----------------------------------------|
     *    dialog3   |                    |            200 OK  |
     *              |----------------------------------------&gt;|
     *    dialog2   | BYE/200 OK         |                    |
     *              |----------------------------------------&gt;|
     *              |              (transferee and target converse)
     *    dialog4   |                    |  BYE/200 OK        |
     *              |                    |-------------------&gt;|
     * </pre>
     *
     *
     * @param serverTransaction - Inbound server transaction we are processing.
     * @param toDomain -- domain to send the request to.
     * @param requestEvent -- the request event we are processing.
     * @param replacedDialog -- the dialog we are replacing.
     * @throws SipException
     */
    void handleSpriralInviteWithReplaces(RequestEvent requestEvent, Dialog replacedDialog,
            ServerTransaction serverTransaction, String toDomain) throws SipException {
        /* The inbound INVITE */
        SipProvider provider = (SipProvider) requestEvent.getSource();
        try {
            Dialog replacedDialogPeerDialog = ((DialogContext) replacedDialog
                    .getApplicationData()).getPeerDialog();
            if (logger.isDebugEnabled()) {
                logger.debug("replacedDialogPeerDialog = "
                        + ((DialogContext) replacedDialog.getApplicationData()).getPeerDialog());
                logger.debug("referingDialogPeerDialog = " + this.referingDialogPeer);
            }

            if (replacedDialogPeerDialog.getState() == DialogState.TERMINATED) {
                Response response = SipUtilities.createResponse(serverTransaction,
                        Response.BUSY_HERE);
                serverTransaction.sendResponse(response);
                return;
            }

            if (this.referingDialogPeer.getState() == DialogState.TERMINATED) {
                Response response = SipUtilities.createResponse(serverTransaction,
                        Response.BUSY_HERE);
                serverTransaction.sendResponse(response);
                return;
            }

            DialogContext.pairDialogs(((DialogContext) replacedDialog.getApplicationData())
                    .getPeerDialog(), this.referingDialogPeer);
            /*
             * Tear down the Music On Hold Dialog if any.
             */
            this.sendByeToMohServer();

            /* The replaced dialog is about ready to die so he has no peer */
            ((DialogContext) replacedDialog.getApplicationData()).setPeerDialog(null);

            if (logger.isDebugEnabled()) {
                logger.debug("referingDialog = " + referingDialog);
                logger.debug("replacedDialog = " + replacedDialog);
            }

            /*
             * We need to form a new bridge. Remove the refering dialog from our rtp bridge.
             * Remove the replacedDialog from its rtpBridge and form a new bridge.
             */
            rtpBridge.pause();

            Set<RtpSession> myrtpSessions = this.rtpBridge.getSyms();
            DialogContext replacedDialogApplicationData = (DialogContext) replacedDialog
                    .getApplicationData();
            RtpBridge hisBridge = replacedDialogApplicationData.getBackToBackUserAgent().rtpBridge;
            hisBridge.pause();

            Set<RtpSession> hisRtpSessions = hisBridge.getSyms();
            BridgeInterface bridge = symmitronClient.createBridge();

            RtpBridge newBridge = new RtpBridge(bridge);

            for (Iterator<RtpSession> it = myrtpSessions.iterator(); it.hasNext();) {
                RtpSession sym = it.next();
                if (sym != DialogContext.getRtpSession(replacedDialog)
                        && sym != DialogContext.getRtpSession(referingDialog)) {
                    newBridge.addSym(sym);
                    it.remove();
                }
            }

            for (Iterator<RtpSession> it = hisRtpSessions.iterator(); it.hasNext();) {
                RtpSession sym = it.next();
                if (sym != DialogContext.getRtpSession(replacedDialog)
                        && sym != DialogContext.getRtpSession(referingDialog)) {
                    newBridge.addSym(sym);
                    it.remove();
                }
            }

            /*
             * Let the RTP Bridge initialize its selector table.
             */

            this.rtpBridge.resume();

            hisBridge.resume();

            newBridge.start();

            if (logger.isDebugEnabled()) {
                logger.debug("replacedDialog State " + replacedDialog.getState());
            }

            if (replacedDialog.getState() != DialogState.TERMINATED
                    && replacedDialog.getState() != DialogState.EARLY) {
                Request byeRequest = replacedDialog.createRequest(Request.BYE);
                ClientTransaction byeCtx = ((DialogExt) replacedDialog).getSipProvider()
                        .getNewClientTransaction(byeRequest);
                TransactionContext.attach(byeCtx, Operation.HANDLE_SPIRAL_INVITE_WITH_REPLACES);
                replacedDialog.sendRequest(byeCtx);

            }

            Response response = SipUtilities.createResponse(serverTransaction, Response.OK);

            ContactHeader contactHeader = SipUtilities.createContactHeader(
                    Gateway.SIPXBRIDGE_USER, provider, SipUtilities.getViaTransport(response));
            response.setHeader(contactHeader);

            /*
             * If re-INVITE is supported send the INVITE down the other leg. Note that at this
             * point we have already queried the Sdp from the other leg. We will ACK that with the
             * response received.
             */

            Dialog referingDialogPeer = this.referingDialogPeer;

            DialogContext referingDialogPeerApplicationData = DialogContext
            .get(referingDialogPeer);
            Response lastResponse = referingDialogPeerApplicationData.getLastResponse();
            DialogContext replacedDialogPeerDialogApplicationData = DialogContext
            .get(replacedDialogPeerDialog);
            Request reInvite = replacedDialogPeerDialog.createRequest(Request.INVITE);

            ItspAccountInfo accountInfo = replacedDialogPeerDialogApplicationData
            .getItspInfo();
            /*
             * Patch up outbound re-INVITE.
             */
            if (accountInfo == null || accountInfo.isGlobalAddressingUsed()) {
                SipUtilities.setGlobalAddresses(reInvite);
            }

            SessionDescription sessionDescription = SipUtilities
            .getSessionDescription(lastResponse);

            replacedDialogPeerDialogApplicationData.getRtpSession().getReceiver()
            .setSessionDescription(sessionDescription);

            SipUtilities.incrementSessionVersion(sessionDescription);

            reInvite.setContent(sessionDescription.toString(), ProtocolObjects.headerFactory
                    .createContentTypeHeader("application", "sdp"));
            SipProvider wanProvider = ((DialogExt) replacedDialogPeerDialog).getSipProvider();
            ClientTransaction ctx = wanProvider.getNewClientTransaction(reInvite);
            TransactionContext.attach(ctx, Operation.HANDLE_SPIRAL_INVITE_WITH_REPLACES);
                replacedDialogPeerDialog.sendRequest(ctx);
           
            serverTransaction.sendResponse(response);

        } catch (Exception ex) {
            logger.error("Unexpected internal error occured", ex);

            CallControlUtilities.sendBadRequestError(serverTransaction, ex);
        }

    }

    /**
     * Remove a dialog from the table ( the dialog terminates ). This is a garbage collection
     * routine. When all the dialogs referencing this structure have terminated, then we send BYE
     * to the MOH dialog ( if it is still alive ).
     *
     * @param dialog -- the terminated dialog.
     */
    synchronized void removeDialog(Dialog dialog) {

        if (!this.dialogTable.contains(dialog)) {
            return;
        }

        logger.debug("removeDialog " + dialog + " dialogTableSize = " + this.dialogTable.size());

        this.dialogTable.remove(dialog);

        if (logger.isDebugEnabled()) {
            logger.debug("Remove Dialog " + dialog + " Dialog table size = "
                    + this.dialogTable.size());
        }
        if (this.dialogTable.size() == 1 ) {
            // This could be a stuck call. We can never have a situation
            // Wait for 4 seconds. If we still have no dialogs, we are done.
            Gateway.getTimer().schedule(new TimerTask() {
                @Override
                public void run() {
                    if (dialogTable.size() == 1) {
                        try {
                            tearDown(Gateway.SIPXBRIDGE_USER, ReasonCode.CALL_TERMINATED,
                                    "Call Termination Detected");
                        } catch (Exception ex) {
                            logger.error("Error tearing down call", ex);
                        }
                    }
                   
                }
            }, 4000);

        }

        if (dialogTable.size() == 0) {
            try {
                logger.debug("Terminating all calls on B2BUA " + this);
                this.tearDown(Gateway.SIPXBRIDGE_USER, ReasonCode.CALL_TERMINATED,
                        "Call Termination Detected");
                this.sendByeToMohServer();
                this.pendingTermination = true;
            } catch (Exception ex) {
                logger.error("Problem tearing down backToBackUserAgent.");
            }
        }
    }

    public void cleanUp() {
        if (this.rtpBridge != null) {
            rtpBridge.stop();
        }
    }

    /**
     * Add a dialog entry to the b2bua. This implies that the given dialog holds a pointer to this
     * structure.
     *
     * @param provider
     * @param dialog
     */
    synchronized void addDialog(DialogContext dialogContext) {

        Dialog dialog = dialogContext.getDialog();
        if (dialogTable.contains(dialog)) {
            logger.debug("Dialog already in table");
            return;
        }
        this.dialogTable.add(dialog);
        dialogContext.recordInsertionPoint();
        String callId = dialog.getCallId().getCallId();
        this.myCallIds.add(callId);
        dialogContext.setBackToBackUserAgent(this);
        logger.debug("addDialog " + dialog + " dialogTableSize = " + this.dialogTable.size()
                + " Dialog was created at: "
                + DialogContext.get(dialog).getCreationPointStackTrace());
        if (this.cleanupList.contains(dialog)) {
            logger.error("Cleanup list also contains dialog");
        }
    }

    /**
     * Add a dialog to the cleanup list. This is the list we clean up after all external dialog
     * references are gone.
     * 
     * @param dialog
     */
    void addDialogToCleanup(Dialog dialog) {
        if (dialogTable.contains(dialog)) {
            logger.warn("Dialog was also found in dialog table - should only be in one "
                    + DialogContext.get(dialog).getCreationPointStackTrace());
            logger.warn("addDialogToCleanup Dialog added to cleanup list at "
                    + SipUtilities.getStackTrace());
            this.dialogTable.remove(dialog);
        }

        logger.debug("addDialogToCleanup " + dialog + " listSize " + this.cleanupList.size()
                + " Dialog was created at: "
                + DialogContext.get(dialog).getCreationPointStackTrace());
        this.cleanupList.add(dialog);
    }

    /**
     * Create an INVITE request from an in-bound REFER. Note that the only reason why this is here
     * and not in SipUtilites is because we need the callId counter - should it be moved?
     *
     *
     * @param requestEvent -- the in-bound REFER request Event.
     *
     * @return the INVITE request crafted from the IB Refer
     *
     */
    @SuppressWarnings("unchecked")
    Request createInviteFromReferRequest(RequestEvent requestEvent) throws SipException,
            ParseException, IOException {
        try {
            Dialog dialog = requestEvent.getDialog();
            ServerTransaction referServerTransaction = requestEvent.getServerTransaction();
            Request referRequest = referServerTransaction.getRequest();
            DialogContext dialogContext = DialogContext.get(dialog);
            FromHeader fromHeader = (FromHeader) dialogContext.getRequest().getHeader(
                    FromHeader.NAME).clone();
            fromHeader.removeParameter("tag");

            ToHeader toHeader = (ToHeader) referRequest.getHeader(ToHeader.NAME).clone();
            toHeader.removeParameter("tag");
            /*
             * Get the Refer-To header and convert it into an INVITE to send to the REFER target.
             */

            ReferToHeader referToHeader = (ReferToHeader) referRequest
                    .getHeader(ReferToHeader.NAME);
            /*
             * Do not set maddr parameter here. This is the refer target. The phone sets it. We do
             * not want to override the port. See XECS-2480. We do not want to set the transport
             * here either.
             */
            SipURI uri = (SipURI) referToHeader.getAddress().getURI().clone();

            CSeqHeader cseq = ProtocolObjects.headerFactory.createCSeqHeader(1L, Request.INVITE);
            ViaHeader viaHeader = null;
            if (uri.getTransportParam() != null) {
                viaHeader = SipUtilities.createViaHeader(Gateway.getLanProvider(), uri
                        .getTransportParam());
            } else {
                viaHeader = SipUtilities.createViaHeader(Gateway.getLanProvider(), "UDP");
            }
            List viaList = new LinkedList();
            viaList.add(viaHeader);
            MaxForwardsHeader maxForwards = ProtocolObjects.headerFactory
                    .createMaxForwardsHeader(20);

            CallIdHeader callId = ProtocolObjects.headerFactory
                    .createCallIdHeader(this.creatingCallId + "." + this.baseCounter + "."
                            + this.counter++);

            Request newRequest = ProtocolObjects.messageFactory.createRequest(uri,
                    Request.INVITE, callId, cseq, fromHeader, toHeader, viaList, maxForwards);

            /*
             * If we are routing this request to the Proxy server, better send it to the SAME
             * proxy server. See XX-5792. Dont do this if we are sending the request directly to a
             * phone!
             */
            RouteHeader proxyRoute = SipUtilities.createRouteHeader(this.proxyAddress);          
            logger.debug("setting ProxyRoute : " + proxyRoute);                      
            newRequest.setHeader(proxyRoute);

            /*
             * Does the refer to header contain a Replaces? ( attended transfer )
             */
            String replacesParam = uri.getHeader(ReplacesHeader.NAME);
            logger.debug("replacesParam = " + replacesParam);

            ReplacesHeader replacesHeader = null;

            if (replacesParam != null) {

                String decodedReplaces = URLDecoder.decode(replacesParam, "UTF-8");
                replacesHeader = (ReplacesHeader) ProtocolObjects.headerFactory.createHeader(
                        ReplacesHeader.NAME, decodedReplaces);
                newRequest.addHeader(replacesHeader);
            }

            uri.removeParameter(ReplacesHeader.NAME);

            for (Iterator it = uri.getHeaderNames(); it.hasNext();) {
                String headerName = (String) it.next();
                String headerValue = uri.getHeader(headerName);
                Header header = null;
                if (headerValue != null) {
                    String decodedHeaderValue = URLDecoder.decode(headerValue, "UTF-8");
                    header = (Header) ProtocolObjects.headerFactory.createHeader(headerName,
                            decodedHeaderValue);
                }
                if (header != null) {
                   newRequest.addHeader(header);
                }
            }
    
            ReferencesHeader referencesHeader = SipUtilities.createReferencesHeader(referRequest, ReferencesHeader.REFER);
            newRequest.setHeader(referencesHeader);
      
            /*
             * Remove any header parameters - we have already dealt with them
             * above.
             */
            ((gov.nist.javax.sip.address.SipURIExt) uri).removeHeaders();

            if (referRequest.getHeader(ReferredByHeader.NAME) != null) {
                newRequest.setHeader(referRequest.getHeader(ReferredByHeader.NAME));
            }

            SipUtilities.addLanAllowHeaders(newRequest);

            String fromUser = ((SipURI) fromHeader.getAddress().getURI()).getUser();
            ContactHeader contactHeader = SipUtilities.createContactHeader(fromUser,
                    Gateway.getLanProvider(), Gateway.getSipxProxyTransport());
            newRequest.setHeader(contactHeader);
            /*
             * Create a new out of dialog request.
             */
            toHeader.getAddress().setURI(uri);

            fromHeader.setTag(Integer.toString(Math.abs(new Random().nextInt())));
            ContentTypeHeader cth = ProtocolObjects.headerFactory.createContentTypeHeader(
                    "application", "sdp");
            newRequest.setHeader(fromHeader);

            /*
             * If we are routing this request to the Proxy server, better send it to the SAME
             * proxy server. See XX-5792. Dont do this if we are sending the request directly to a
             * phone!
             */
            if (uri.getHost().equals(Gateway.getSipxProxyDomain())) {
                RouteHeader routeHeader = SipUtilities.createRouteHeader(this.proxyAddress);
                newRequest.setHeader(routeHeader);
            }

            RtpSession lanRtpSession = this.createRtpSession(dialog);
            SessionDescription sd = lanRtpSession.getReceiver().getSessionDescription();
            SipUtilities.setDuplexity(sd, "sendrecv");
            newRequest.setContent(sd, cth);

            return newRequest;
        } catch (InvalidArgumentException ex) {
            throw new SipXbridgeException(ex);
        }

    }

    /**
     * This method is called when the REFER is received at the B2BUA. Since ITSPs do not handle
     * REFER, we convert it to a re-INVITE to solicit an offer. To determine the codec that was
     * negotiated in the original Call Setup, we send an INVITE (no-sdp) to the dialog to solicit
     * an offer. This operation has already returned a result when this method is called. This
     * method is called when the response for that solicitation is received. We need to direct an
     * INVITE to the contact mentioned in the Refer. Notice that when this method is called, we
     * have already created an INVITE previously that we will now replay with the SDP answer to
     * the PBX side.
     *
     *
     * @param inviteRequest -- the INVITE request.
     * @param referRequestEvent -- the REFER dialog
     * @param sessionDescription -- the session description to use when forwarding the INVITE to
     *        the sipx proxy server.
     * @param dialogPendingSdpAnswer -- the dialog that is pending SDP answer in ACK after the OK
     *        comes in with an SDP.
     *
     */

    void referInviteToSipxProxy(Request inviteRequest, ClientTransaction mohClientTransaction,
            Dialog dialogPendingSdpAnswer, RequestEvent referRequestEvent,
            SessionDescription sessionDescription) {
        logger.debug("referInviteToSipxProxy: ");

        try {

            Dialog referRequestEventDialog = referRequestEvent.getDialog();
            Request referRequest = referRequestEvent.getRequest();
            ServerTransaction stx = referRequestEvent.getServerTransaction();

            /*
             * Transfer agent canceled the REFER before we had a chance to process it.
             */
            if (referRequestEventDialog == null
                    || referRequestEventDialog.getState() == DialogState.TERMINATED) {
                /*
                 * Out of dialog refer.
                 */
                Response response;
                if (stx != null) {
                    response = SipUtilities.createResponse(stx, Response.NOT_ACCEPTABLE);
                } else {
                    response = ProtocolObjects.messageFactory.createResponse(
                            Response.NOT_ACCEPTABLE, referRequest);
                }
                WarningHeader warning = ProtocolObjects.headerFactory.createWarningHeader(
                        Gateway.SIPXBRIDGE_USER, WarningCode.OUT_OF_DIALOG_REFER,
                        "Out of dialog REFER");
                response.setHeader(SipUtilities.createContactHeader(null,
                        ((SipProvider) referRequestEvent.getSource()),
                        SipUtilities.getViaTransport(response)));
                response.setHeader(warning);
                if (stx != null) {
                    stx.sendResponse(response);
                } else {
                    ((SipProvider) referRequestEvent.getSource()).sendResponse(response);
                }

                return;
            }

            /*
             * Create a new client transaction. First attach any queried session description.
             */
            if (sessionDescription != null) {
                SipUtilities.setDuplexity(sessionDescription, "sendrecv");
                SipUtilities.setSessionDescription(inviteRequest, sessionDescription);
            }
            ClientTransaction ct = Gateway.getLanProvider()
                    .getNewClientTransaction(inviteRequest);

            DialogContext newDialogContext = DialogContext.attach(this, ct.getDialog(), ct, ct
                    .getRequest());
            this.addDialog(newDialogContext);
            DialogContext referDialogContext = (DialogContext) referRequestEventDialog
                    .getApplicationData();

            newDialogContext.rtpSession = referDialogContext.rtpSession;
            newDialogContext.setPeerDialog(referDialogContext.getPeerDialog());

            if (logger.isDebugEnabled()) {
                logger.debug("referInviteToSipxProxy peerDialog = "
                        + newDialogContext.getPeerDialog());

            }

            this.referingDialogPeer = referDialogContext.getPeerDialog();

            DialogContext dat = (DialogContext) this.referingDialogPeer.getApplicationData();
            dat.setPeerDialog(ct.getDialog());

            /*
             * Mark that we (sipxbridge) originated the dialog.
             */
            newDialogContext.isOriginatedBySipxbridge = true;

            /*
             * Record the referDialog so that when responses for the Client transaction come in we
             * can NOTIFY the referrer.
             */
            this.referingDialog = referRequestEventDialog;

            /*
             * Mark that when we get the OK we want to re-INVITE the other side. This drives the
             * Dialog state machine when we get the OK response. We note that we need to send the
             * other side an re-INVITE with SDP offer.
             */
            newDialogContext
                    .setPendingAction(PendingDialogAction.PENDING_RE_INVITE_WITH_SDP_OFFER);

            /*
             * Link this new dialog contex to the peer.
             */
            newDialogContext.setPeerDialog(referDialogContext.getPeerDialog());

            TransactionContext tad = TransactionContext.attach(ct,
                    Operation.REFER_INVITE_TO_SIPX_PROXY);
            tad.setBackToBackUa(((DialogContext) referRequestEventDialog.getApplicationData())
                    .getBackToBackUserAgent());
            tad.setReferingDialog(referRequestEventDialog);

            tad.setReferRequest(referRequest);

            tad.setDialogPendingSdpAnswer(dialogPendingSdpAnswer);

            tad.setMohClientTransaction(mohClientTransaction);

            /*
             * Stamp the via header with our stamp so that we know we Referred this request. we
             * will use this for spiral detection.
             */
            ViaHeader via = (ViaHeader) inviteRequest.getHeader(ViaHeader.NAME);
            /*
             * This is our signal that we originated the redirection. We use this in the INVITE
             * processing below. SIPX will not strip any via header parameters.If the INVITE
             * spirals back to us, we need to know that it is a spiral( see processing above that
             * checks this flag).
             */
            via.setParameter(ORIGINATOR, newDialogContext.getDialogContextId());

            /*
             * Send the request. Note that this is not an in-dialog request.
             */
            ct.sendRequest();

            /*
             * We have the INVITE now so send an ACCEPTED to the REFER agent.
             */
            Response response = ProtocolObjects.messageFactory.createResponse(Response.ACCEPTED,
                    referRequest);
            response.setHeader(SipUtilities.createContactHeader(null,
                    ((SipProvider) referRequestEvent.getSource()),
                    SipUtilities.getViaTransport(response)));
            stx.sendResponse(response);

        } catch (ParseException ex) {
            logger.error("Unexpected parse exception", ex);
            throw new SipXbridgeException("Unexpected parse exception", ex);

        } catch (Exception ex) {
            logger.error("Error while processing the request - hanging up ", ex);
            try {
                this.tearDown();
            } catch (Exception e) {
                logger.error("Unexpected exception tearing down session", e);
            }
        }

    }

    /**
     * Forward the REFER request to the ITSP. This only works if the ITSP actually supports REFER.
     * It is not used but we keep it around for future use.
     *
     *
     * @param requestEvent - INBOUND REFER ( from sipx )
     */
    void forwardReferToItsp(RequestEvent requestEvent) {

        try {
            Dialog dialog = requestEvent.getDialog();
            Dialog peerDialog = DialogContext.getPeerDialog(dialog);
            Request referRequest = requestEvent.getRequest();
            ItspAccountInfo itspInfo = DialogContext.get(peerDialog).getItspInfo();
            SipProvider wanProvider = ((DialogExt) peerDialog).getSipProvider();

            Request outboundRefer = peerDialog.createRequest(Request.REFER);

            ReferToHeader referToHeader = (ReferToHeader) referRequest
                    .getHeader(ReferToHeader.NAME);
            SipURI uri = (SipURI) referToHeader.getAddress().getURI();
            String referToUserName = uri.getUser();

            SipURI forwardedReferToUri = SipUtilities.createInboundRequestUri(itspInfo);

            forwardedReferToUri.setParameter("target", referToUserName);

            Address referToAddress = ProtocolObjects.addressFactory
                    .createAddress(forwardedReferToUri);

            SipURI forwardedReferredByUri = SipUtilities.createInboundReferredByUri(itspInfo);

            Address referByAddress = ProtocolObjects.addressFactory
                    .createAddress(forwardedReferredByUri);

            ReferToHeader outboundReferToHeader = ProtocolObjects.headerFactory
                    .createReferToHeader(referToAddress);

            ReferredByHeader outboundReferByHeader = ((HeaderFactoryExt) ProtocolObjects.headerFactory)
                    .createReferredByHeader(referByAddress);

            outboundRefer.setHeader(outboundReferByHeader);

            outboundRefer.setHeader(outboundReferToHeader);

            if (itspInfo == null || itspInfo.isGlobalAddressingUsed()) {
                SipUtilities.setGlobalAddresses(outboundRefer);
            }

            SipUtilities.addWanAllowHeaders(outboundRefer);

            ClientTransaction outboundReferClientTx = wanProvider
                    .getNewClientTransaction(outboundRefer);
            TransactionContext tad = TransactionContext.attach(outboundReferClientTx,
                    Operation.FORWARD_REFER);
            tad.setServerTransaction(requestEvent.getServerTransaction());
            peerDialog.sendRequest(outboundReferClientTx);

        } catch (SipException ex) {
            logger.error("Error while processing the request - hanging up ", ex);
            try {
                this.tearDown(ProtocolObjects.headerFactory.createReasonHeader(
                        Gateway.SIPXBRIDGE_USER, ReasonCode.PROTOCOL_ERROR,
                        "Protocol error detected."));
            } catch (Exception e) {
                logger.error("Unexpected exception tearing down session", e);
            }
        } catch (ParseException e) {
            logger.error("INTERNAL Error while processing the request - hanging up ", e);
            throw new SipXbridgeException("INTERNAL Error while processing the request ", e);
        }
    }

    /**
     * Send an INVITE to SIPX proxy server.
     *
     * @param requestEvent -- The incoming RequestEvent ( from the ITSP side ) for which we are
     *        generating the request outbound to the sipx proxy server.
     *
     * @param serverTransaction -- The SIP Server transaction that we created to service this
     *        request.
     */

    void sendInviteToSipxProxy(RequestEvent requestEvent, ServerTransaction serverTransaction) {
        Request request = requestEvent.getRequest();

        logger.debug("sendInviteToSipXProxy " + ((SIPMessage) request).getFirstLine());

        try {
            /*
             * This is a request I got from the external provider. Route this into the network.
             * The SipURI is the sipx proxy URI. He takes care of the rest.
             */

            SipURI incomingRequestURI = (SipURI) request.getRequestURI();
            Dialog inboundDialog = serverTransaction.getDialog();
            logger.debug("inboundDialog  " + inboundDialog);
            /*
             * Add the dialog context to our table of managed dialogs.
             */
            this.addDialog(DialogContext.get(inboundDialog));
            ViaHeader inboundVia = ((ViaHeader) request.getHeader(ViaHeader.NAME));

            String host = inboundVia.getReceived() != null ? inboundVia.getReceived()
                    : inboundVia.getHost();
            // do some ITSPs need the rport?  For TLS it is not correct...
            int port = inboundVia.getPort();

            ItspAccountInfo itspAccountInfo = Gateway.getAccountManager().getItspAccount(host,
                    port);

            SipURI uri = null;
            if (!Gateway.isInboundCallsRoutedToAutoAttendant()) {
                uri = ProtocolObjects.addressFactory.createSipURI(incomingRequestURI.getUser(),
                        Gateway.getSipxProxyDomain());
            } else {
                String destination = Gateway.getAutoAttendantName();
                if (destination.indexOf("@") == -1) {
                    uri = ProtocolObjects.addressFactory.createSipURI(destination, Gateway
                            .getSipxProxyDomain());
                    if (Gateway.getBridgeConfiguration().getSipxProxyPort() != -1) {
                        uri.setPort(Gateway.getBridgeConfiguration().getSipxProxyPort());
                    }
                } else {
                    logger.warn("routing to domain other than proxy domain!");
                    uri = (SipURI) ProtocolObjects.addressFactory.createURI("sip:" + destination);
                }
            }

            CSeqHeader cseqHeader = ProtocolObjects.headerFactory.createCSeqHeader(
                    ((CSeqHeader) request.getHeader(CSeqHeader.NAME)).getSeqNumber(),
                    Request.INVITE);

            FromHeader fromHeader = (FromHeader) request.getHeader(FromHeader.NAME).clone();

            /*
             * Change the domain of the inbound request to that of the sipx proxy. Change the user
             * part if routed to specific extension.
             */
            String autoAttendantName = Gateway.getAutoAttendantName();
            ToHeader toHeader = null;
            if (autoAttendantName != null && autoAttendantName.indexOf("@") != -1) {
                SipURI toUri = (SipURI) ProtocolObjects.addressFactory.createURI("sip:"
                        + autoAttendantName);
                Address toAddress = ProtocolObjects.addressFactory.createAddress(toUri);
                toHeader = ProtocolObjects.headerFactory.createToHeader(toAddress, null);
            } else {
                toHeader = (ToHeader) request.getHeader(ToHeader.NAME).clone();
                ((SipURI) toHeader.getAddress().getURI()).setHost(Gateway.getSipxProxyDomain());
                ((SipURI) toHeader.getAddress().getURI()).removePort();
                if (Gateway.isInboundCallsRoutedToAutoAttendant()) {
                    ((SipURI) toHeader.getAddress().getURI()).setUser(Gateway
                            .getAutoAttendantName());
                }
                toHeader.removeParameter("tag");
            }
            String transport = proxyAddress.getTransport().equalsIgnoreCase("TLS") ? "TLS"
                    : "UDP";

            ViaHeader viaHeader = SipUtilities.createViaHeader(Gateway.getLanProvider(),
                    transport);
          
            List<ViaHeader> viaList = new LinkedList<ViaHeader>();

            viaList.add(viaHeader);

            MaxForwardsHeader maxForwards = (MaxForwardsHeader) request
                    .getHeader(MaxForwardsHeader.NAME);

            maxForwards.decrementMaxForwards();
            ProxyAuthorizationHeader authorization = null;
            if (request.getHeader(ProxyAuthorizationHeader.NAME) != null) {
                authorization = (ProxyAuthorizationHeader) request
                        .getHeader(ProxyAuthorizationHeader.NAME);
            }

            CallIdHeader callIdHeader = ProtocolObjects.headerFactory
                    .createCallIdHeader(this.creatingCallId + "." + baseCounter);

            if (request.getHeader(ProxyAuthorizationHeader.NAME) == null) {
                this.baseCounter++;
            }

            Request newRequest = ProtocolObjects.messageFactory.createRequest(
                    uri, Request.INVITE, callIdHeader, cseqHeader, fromHeader,
                    toHeader, viaList, maxForwards);

            ReferencesHeader referencesHeader = SipUtilities.createReferencesHeader(request,
                        ReferencesHeader.CHAIN);
            newRequest.addHeader(referencesHeader);

            ContactHeader contactHeader = SipUtilities.createContactHeader(
                    incomingRequestURI.getUser(), Gateway.getLanProvider(),
                    SipUtilities.getViaTransport(newRequest));
            newRequest.setHeader(contactHeader);
            if (authorization != null) {
                newRequest.addHeader(authorization);
            }

            /*
             * The incoming session description.
             */
            SessionDescription sessionDescription = SipUtilities.getSessionDescription(request);

            RtpSession incomingSession = this.createRtpSession(inboundDialog);

            incomingSession.getReceiver().setUseGlobalAddressing(
                    itspAccountInfo == null || itspAccountInfo.isGlobalAddressingUsed());

            RtpTransmitterEndpoint rtpEndpoint = new RtpTransmitterEndpoint(incomingSession,
                    symmitronClient);
            incomingSession.setTransmitter(rtpEndpoint);
            KeepaliveMethod keepaliveMethod = itspAccountInfo != null ? itspAccountInfo
                    .getRtpKeepaliveMethod() : KeepaliveMethod.NONE;

            rtpEndpoint.setKeepAliveMethod(keepaliveMethod);
            rtpEndpoint.setSessionDescription(sessionDescription, true);

            ContentTypeHeader cth = ProtocolObjects.headerFactory.createContentTypeHeader(
                    "application", "sdp");
            /*
             * Create a new client transaction.
             */
            ClientTransaction ct = Gateway.getLanProvider().getNewClientTransaction(newRequest);

            Dialog outboundDialog = ct.getDialog();

            DialogContext newDialogContext = DialogContext.attach(this, outboundDialog, ct, ct
                    .getRequest());
            
            viaHeader.setParameter(ORIGINATOR, newDialogContext.getDialogContextId());

            /*
             * Set the ITSP account info for the inbound INVITE to sipx proxy.
             */
            newDialogContext.setItspInfo(itspAccountInfo);
            DialogContext.pairDialogs(inboundDialog, outboundDialog);
            /*
             * Put these in our dialog table.
             */
            this.addDialog(DialogContext.get(outboundDialog));
            /*
             * Apply the Session Description from the INBOUND invite to the Receiver of the RTP
             * session pointing towards the PBX side. This resets the ports in the session
             * description.
             */
            SessionDescription sd = SipUtilities.getSessionDescription(request);

            RtpSession outboundSession = this.createRtpSession(outboundDialog);

            logger.debug("outboundSession = " + outboundSession);
            logger.debug("outboundDialogContext = " + DialogContext.get(outboundDialog));

            outboundSession.getReceiver().setSessionDescription(sd);

            newRequest.setContent(outboundSession.getReceiver().getSessionDescription()
                    .toString(), cth);

            SipUtilities.addLanAllowHeaders(newRequest);

            if ( SipUtilities.getViaTransport(request).equalsIgnoreCase("TLS")) {
                logger.debug("incoming request came over TLS");
                List<String> certIdentities = ((SIPTransaction)serverTransaction).extractCertIdentities();
                if (certIdentities.isEmpty()) {
                    logger.warn("Could not find any identities in the TLS certificate");
                }
                else {
                    logger.debug("found identities: " + certIdentities);
                    // Policy enforcement: now use the set of SIP
                    // domain identities gathered from the certificate to
                    // make authorization decisions.
                    // Iterate over the subjects in the certificate and use the first one
                    // that matches a configured peer.
                    String peerIdentity;
                    Boolean foundPeerIdentity = false;
                    for (String domain : certIdentities) {
                        PeerIdentities peers = Gateway.getPeerIdentities();
                        if ( (peers != null) && (peerIdentity = peers.getUserId(domain)) != null) {
                            // Now attach a signed X-Sipx-Authidentity header with the identity
                            // configured for this connection
                            peerIdentity += "@" + Gateway.getSipxProxyDomain();
                            SipXauthIdentity.insertIdentity(peerIdentity, newRequest,
                                    SipXauthIdentity.AuthIdentityHeaderName);
                            foundPeerIdentity = true;
                            break;
                        }
                    }
                    if (!foundPeerIdentity) {
                        // This is not necessarily an error; it just means that no user identity will be
                        // assigned to this call, and access to resources requiring permissions will be refused
                        logger.warn("No matching TLS Peer found for " + certIdentities);
                    }
                }
            }

            TransactionContext tad = new TransactionContext(ct,
                    Operation.SEND_INVITE_TO_SIPX_PROXY);

            tad.setRequestEvent(requestEvent);
            tad.setServerTransaction(serverTransaction);
            tad.setBackToBackUa(this);
            tad.setItspAccountInfo(itspAccountInfo);

            this.referingDialog = ct.getDialog();

            this.referingDialogPeer = serverTransaction.getDialog();
            
            /*
             * Finally copy all the headers we do not understand.
             */
           // SipUtilities.copyHeaders(request, newRequest, false);

            ct.sendRequest();

        } catch (InvalidArgumentException ex) {
            logger.error("Unexpected exception encountered");
            throw new SipXbridgeException("Unexpected exception encountered", ex);
        } catch (ParseException ex) {
            logger.error("Unexpected parse exception", ex);
            throw new SipXbridgeException("Unexpected parse exception", ex);
        } catch (SdpParseException ex) {
            try {
                Response response = ProtocolObjects.messageFactory.createResponse(
                        Response.BAD_REQUEST, request);
                serverTransaction.sendResponse(response);
            } catch (Exception e) {
                logger.error("Unexpected exception", e);
            }
        } catch (Exception ex) {
            logger.error("Error while processing the request", ex);
            CallControlUtilities.sendServiceUnavailableError(serverTransaction, ex);

        }

    }

    /**
     * Create a Client Tx pointing towards the park server.
     *
     * @param sessionDescription : the session description to apply to the INVITE.
     * @param triggeringResponse: The response that triggered this action.
     * @param callid : the callId of the response that triggered this client transaction ( so we can 
     *    form a causal chain).
     * @param topmostViaBranch : the topmost via branch of the response that triggered this client tx.
     * 
     *
     * @return the dialog generated as a result of sending the invite to the MOH server.
     *
     */
    ClientTransaction createClientTxToMohServer(SessionDescription sessionDescription, Response triggeringResponse) {

        ClientTransaction retval = null;

              
        try {

            SipURI uri = Gateway.getMusicOnHoldUri();

            CallIdHeader callIdHeader = ProtocolObjects.headerFactory
                    .createCallIdHeader(this.creatingCallId + "." + this.baseCounter + "."
                            + this.counter++);

            CSeqHeader cseqHeader = ProtocolObjects.headerFactory.createCSeqHeader(1L,
                    Request.INVITE);

            Address gatewayAddress = Gateway.getGatewayFromAddress();

            FromHeader fromHeader = ProtocolObjects.headerFactory.createFromHeader(
                    gatewayAddress, Long.toString(Math.abs(new Random().nextLong())));

            Address mohServerAddress = Gateway.getMusicOnHoldAddress();

            ToHeader toHeader = ProtocolObjects.headerFactory.createToHeader(mohServerAddress,
                    null);

            toHeader.removeParameter("tag");

            ViaHeader viaHeader = SipUtilities.createViaHeader(Gateway.getLanProvider(), Gateway
                    .getSipxProxyTransport());

            List<ViaHeader> viaList = new LinkedList<ViaHeader>();

            viaList.add(viaHeader);

            MaxForwardsHeader maxForwards = ProtocolObjects.headerFactory
                    .createMaxForwardsHeader(20);

            maxForwards.decrementMaxForwards();
            Request newRequest = ProtocolObjects.messageFactory.createRequest(uri,
                    Request.INVITE, callIdHeader, cseqHeader, fromHeader, toHeader, viaList,
                    maxForwards);

          
            ReferencesHeader referencesHeader =
                        SipUtilities.createReferencesHeader(triggeringResponse,              
                        ReferencesHeader.SERVICE);
            newRequest.setHeader(referencesHeader);
            
            ContactHeader contactHeader = SipUtilities.createContactHeader(
                    Gateway.SIPXBRIDGE_USER, Gateway.getLanProvider(),
                    Gateway.getSipxProxyTransport());
            newRequest.setHeader(contactHeader);

            RouteHeader routeHeader = SipUtilities.createRouteHeader(this.proxyAddress);
            newRequest.setHeader(routeHeader);

            /*
             * Create a new client transaction.
             */
            ClientTransaction ct = Gateway.getLanProvider().getNewClientTransaction(newRequest);

            /*
             * Set the duplexity of the INVITE to recvonly.
             */
            SipUtilities.setDuplexity(sessionDescription, "recvonly");

            ContentTypeHeader cth = ProtocolObjects.headerFactory.createContentTypeHeader(
                    "application", "sdp");

            newRequest.setContent(sessionDescription.toString(), cth);

            TransactionContext tad = TransactionContext.attach(ct,
                    Operation.SEND_INVITE_TO_MOH_SERVER);

            tad.setBackToBackUa(this);
            DialogContext.attach(this, ct.getDialog(), ct, ct.getRequest());

            /*
             * Store that dialog away for later use. This is managed separately.
             */
            this.musicOnHoldDialog = ct.getDialog();
            retval = ct;

        } catch (InvalidArgumentException ex) {
            logger.error("Unexpected exception encountered");
            throw new SipXbridgeException("Unexpected exception encountered", ex);
        } catch (Exception ex) {
            logger.error("Unexpected parse exception", ex);
        }
        return retval;
    }

    /**
     * Send a BYE to the Music On Hold Server.
     *
     * @param musicOnHoldDialog
     * @throws SipException
     */
    void sendByeToMohServer() {
        try {
            if (this.musicOnHoldDialog != null
                    && this.musicOnHoldDialog.getState() != DialogState.TERMINATED) {
                if (this.musicOnHoldDialog.getState() == DialogState.CONFIRMED) {
                    logger.debug("sendByeToMohServer");
                    Request byeRequest = musicOnHoldDialog.createRequest(Request.BYE);

                    ClientTransaction ctx = Gateway.getLanProvider().getNewClientTransaction(
                            byeRequest);
                    TransactionContext.attach(ctx, Operation.SEND_BYE_TO_MOH_SERVER);
                    musicOnHoldDialog.sendRequest(ctx);
                } else {
                    DialogContext.get(this.musicOnHoldDialog).setTerminateOnConfirm();

                }
            }
        } catch (SipException ex) {
            logger.error("Error sending BYE to the MOH dialog", ex);
        }

    }

    /**
     * Send a request to an ITSP. This is the out of dialog request that sets up the call.
     *
     * @param requestEvent -- in bound request event ( sent by the sipx proxy server)
     * @param serverTransaction -- the server transaction.
     * @param toDomain -- domain to send it to.
     *
     * @throws SipException
     */

    void sendInviteToItsp(RequestEvent requestEvent, ServerTransaction serverTransaction,
            String toDomain) throws SipException {

        logger.debug("sendInviteToItsp: " + this);
        Request incomingRequest = serverTransaction.getRequest();
        Dialog incomingDialog = serverTransaction.getDialog();
        ItspAccountInfo itspAccountInfo = Gateway.getAccountManager().getAccount(incomingRequest);

        SipProvider itspProvider = Gateway
                .getWanProvider(itspAccountInfo == null ? Gateway.DEFAULT_ITSP_TRANSPORT
                        : itspAccountInfo.getOutboundTransport());

        boolean spiral = SipUtilities.isOriginatorSipXbridge(incomingRequest);

        ReplacesHeader replacesHeader = (ReplacesHeader) incomingRequest
                .getHeader(ReplacesHeader.NAME);

        if (logger.isDebugEnabled()) {
            logger.debug("sendInviteToItsp: spiral=" + spiral);
        }
        try {
            if (replacesHeader != null) {

                /* Fetch the Dialog object corresponding to the ReplacesHeader */
                Dialog replacedDialog = ((SipStackExt) ProtocolObjects.getSipStack())
                        .getReplacesDialog(replacesHeader);

                if (replacedDialog == null) {

                    Response response = SipUtilities.createResponse(serverTransaction,
                            Response.NOT_FOUND);
                    response.setReasonPhrase("Replaced Dialog not found");
                    serverTransaction.sendResponse(response);
                    return;
                }

                if (spiral) {
                    handleSpriralInviteWithReplaces(requestEvent, replacedDialog,
                            serverTransaction, toDomain);
                } else {
                    handleInviteWithReplaces(requestEvent, replacedDialog, serverTransaction);
                }

                return;
            }

            SipURI incomingRequestUri = (SipURI) incomingRequest.getRequestURI();

            this.mohDisabled = incomingRequestUri.getParameter("sipxbridge-moh") != null
                    && incomingRequestUri.getParameter("sipxbridge-moh").equals("false");

            FromHeader fromHeader = (FromHeader) incomingRequest.getHeader(FromHeader.NAME)
                    .clone();
            /*
             * If the proxy has sent us a maddr parameter or has inserted an LR parameter that
             * does not correspond to us, we use that as the list of addresses to send to next.
             */
            Collection<Hop> addresses = new HashSet<Hop>();
            SipURI outboundRequestUri = (SipURI) incomingRequestUri.clone();
            /*
             * Determine next hop information for the re-originated request. If the inbound
             * request URI has a maddr param, use it. Otherwise look at the topmost Route header.
             * If the route header matches our address, pop it and use the next one. After popping
             * the route header, we test to see if there exist other route headers. If they exist,
             * use them. Else if a maddr exists, use it, otherwise use the default route for the
             * ITSP.
             */
            if (incomingRequest.getHeader(RouteHeader.NAME) != null) {
                Iterator routes = incomingRequest.getHeaders(RouteHeader.NAME);
                if (routes != null && routes.hasNext()) {
                    RouteHeader route = (RouteHeader) routes.next();
                    if (((SipURI) route.getAddress().getURI()).getHost().equals(
                            Gateway.getBridgeConfiguration().getLocalAddress())
                            && routes.hasNext()) {
                        route = (RouteHeader) routes.next();
                    } else {
                        route = null;
                    }

                    if (route != null) {
                        Hop hop = SipUtilities.createHop(route);
                        addresses.add(hop);
                    } else if (incomingRequestUri.getMAddrParam() == null
                            || incomingRequestUri.getMAddrParam().equals(
                                    Gateway.getBridgeConfiguration().getLocalAddress())) {
                        /*
                         * No Maddr seen on incoming requestURI. or maddr matches our address. Use
                         * the OB proxy for the ITSP.
                         */
                        addresses = itspAccountInfo.getItspProxyAddresses();
                    } else {
                        String maddr = incomingRequestUri.getParameter("maddr");
                        int port = incomingRequestUri.getPort() <= 0 ? 5060 : incomingRequestUri
                                .getPort();
                        String transport = incomingRequestUri.getParameter("transport");
                        if (transport == null) {
                            transport = "udp";
                        }
                        if (!Gateway.getSupportedTransports().contains(transport.toLowerCase())) {
                            Response response = SipUtilities.createResponse(serverTransaction,
                                    Response.NOT_ACCEPTABLE);
                            response.setReasonPhrase("Requested Transport is not supported");
                            serverTransaction.sendResponse(response);
                            return;
                        }
                        HopImpl hop = new HopImpl(maddr, port, transport);
                        addresses.add(hop);
                    }

                } else {
                    addresses = itspAccountInfo.getItspProxyAddresses();
                }
            } else if (incomingRequestUri.getMAddrParam() != null) {
                /*
                 * No Route header attached to the iB request. If the inbound request has a maddr
                 * parameter that is not sipxbridge, then forward that.
                 */
                if (incomingRequestUri.getMAddrParam().equals(
                        Gateway.getBridgeConfiguration().getLocalAddress())) {
                    addresses = itspAccountInfo.getItspProxyAddresses();
                } else {
                    String maddr = incomingRequestUri.getParameter("maddr");
                    int port = incomingRequestUri.getPort() <= 0 ? 5060 : incomingRequestUri
                            .getPort();
                    String transport = incomingRequestUri.getParameter("transport");
                    if (transport == null) {
                        transport = "udp";
                    }
                    if (!Gateway.getSupportedTransports().contains(transport.toLowerCase())) {
                        Response response = SipUtilities.createResponse(serverTransaction,
                                Response.NOT_ACCEPTABLE);
                        response.setReasonPhrase("Requested Transport is not supported");
                        serverTransaction.sendResponse(response);
                        return;
                    }
                    HopImpl hop = new HopImpl(maddr, port, transport);
                    addresses.add(hop);

                }

            } else {
                addresses = itspAccountInfo.getItspProxyAddresses();
            }

            Request outgoingRequest = SipUtilities.createInviteRequest(
                    (SipURI) outboundRequestUri, itspProvider, itspAccountInfo, fromHeader,
                    this.creatingCallId + "." + baseCounter, addresses);
            /*
             * If there is an AUTH header there, it could be that the client is sending
             * us credentials. In that case, do not change the call ID. 
             * Incrementing the base counter ensures that other calls that get assigned
             * this B2Bua instance get a unique call Id. For example, this would be the
             * case with the call leg we establish with the MOH server. All calls have the
             * same prefix but the suffix is baseCounter.counter.
             */
            if (incomingRequest.getHeader(AuthorizationHeader.NAME) == null) {
                baseCounter++;
            }
            /*
             * If we have authorization information, we can attach it to the outbound request.
             */
            if ( incomingRequest.getHeader(AuthorizationHeader.NAME) != null ) {
                AuthorizationHeader authorization = (AuthorizationHeader)
                    incomingRequest.getHeader(AuthorizationHeader.NAME);
                outgoingRequest.setHeader(authorization);
            }

            ReferencesHeader referencesHeader = SipUtilities.createReferencesHeader(incomingRequest,
                    ReferencesHeader.CHAIN);
            outgoingRequest.setHeader(referencesHeader);

            /*
             * If we have authorization information, we can attach it to the outbound request.
             */
            if (incomingRequest.getHeader(AuthorizationHeader.NAME) != null) {
                AuthorizationHeader authorization = (AuthorizationHeader) incomingRequest
                        .getHeader(AuthorizationHeader.NAME);
                outgoingRequest.setHeader(authorization);
            }

            /*
             * Attach headers selectively to the outbound request. If privacy is requested, we
             * suppress forwarding certain headers that can reveal information about the caller.
             */
            for (String headerName : new String[] {
                ReplyToHeader.NAME, CallInfoHeader.NAME, SubjectHeader.NAME,
                OrganizationHeader.NAME, InReplyToHeader.NAME
            }) {
                Header header = incomingRequest.getHeader(headerName);
                if (header != null && !itspAccountInfo.stripPrivateHeaders()) {
                    outgoingRequest.addHeader(header);
                }
            }
            
           
            
            /*
             * Attach ALLOW headers that are only relevant for the LAN side.
             */
            SipUtilities.addWanAllowHeaders(outgoingRequest);
            /*
             * Create Client tx to send the request.
             */
            ClientTransaction ct = itspProvider.getNewClientTransaction(outgoingRequest);
            Dialog outboundDialog = ct.getDialog();

            DialogContext.attach(this, outboundDialog, ct, outgoingRequest);
            DialogContext.get(outboundDialog).setItspInfo(itspAccountInfo);

            SessionDescription outboundSessionDescription = null;

            /*
             * Set up to use global addressing on the outbound session if needed.
             */
            boolean globalAddressing = itspAccountInfo == null
                    || itspAccountInfo.isGlobalAddressingUsed();
            if (spiral) {
            	/*
                 * If this is a spiral, we re-use the RTP session from the refering dialog. This
                 * case occurs when we do a blind transfer.
                 */
                if (globalAddressing) {
                    DialogContext.getRtpSession(this.referingDialog).getReceiver()
                            .setGlobalAddress(this.symmitronClient.getPublicAddress());
                }
                DialogContext.getRtpSession(this.referingDialog).getReceiver()
                        .setUseGlobalAddressing(globalAddressing);
                outboundSessionDescription = DialogContext.getRtpSession(this.referingDialog)
                        .getReceiver().getSessionDescription();
                
           } else {
                RtpSession wanRtpSession = this.createRtpSession(outboundDialog);
                wanRtpSession.getReceiver().setUseGlobalAddressing(globalAddressing);
                outboundSessionDescription = SipUtilities.getSessionDescription(incomingRequest);
                wanRtpSession.getReceiver().setSessionDescription(outboundSessionDescription);
            }

            if (outboundSessionDescription == null) {
                Response response = SipUtilities.createResponse(serverTransaction,
                        Response.NOT_ACCEPTABLE_HERE);
                serverTransaction.sendResponse(response);
                return;
            }

            /*
             * Indicate that we will be transmitting first.
             */

            ContentTypeHeader cth = ProtocolObjects.headerFactory.createContentTypeHeader(
                    "application", "sdp");

            outgoingRequest.setContent(outboundSessionDescription.toString(), cth);

            String outboundTransport = itspAccountInfo == null ? Gateway.DEFAULT_ITSP_TRANSPORT
                    : itspAccountInfo.getOutboundTransport();
            ListeningPoint lp = itspProvider.getListeningPoint(outboundTransport);
            String sentBy = lp.getSentBy();
            if (itspAccountInfo == null || itspAccountInfo.isGlobalAddressingUsed()) {
                lp.setSentBy(Gateway.getGlobalAddress() + ":" + Gateway.getGlobalPort(lp.getTransport()));
            }
            

            /*
             * pair the inbound and outboud dialogs.
             */
           
            DialogContext.pairDialogs(incomingDialog, outboundDialog);
            this.addDialog(DialogContext.get(incomingDialog));

            SessionDescription sessionDescription = SipUtilities
                    .getSessionDescription(incomingRequest);

            /*
             * This prepares for an authentication challenge.
             */
            TransactionContext tad = new TransactionContext(serverTransaction,
                    spiral ? Operation.SPIRAL_BLIND_TRANSFER_INVITE_TO_ITSP
                            : Operation.SEND_INVITE_TO_ITSP);
            tad.setItspAccountInfo(itspAccountInfo);
            tad.setBackToBackUa(this);
            tad.setClientTransaction(ct);
            tad.setProxyAddresses(addresses);
            /*
             * Set up for fast failover. If we dont get a 100 in 2 second we will get a timeout
             * alert. If the other side supports DNS SRV we will try the other hop.
             */
            ((ClientTransactionExt) ct).alertIfStillInCallingStateBy(2);

            if (!spiral) {
                RtpSession incomingSession = this.createRtpSession(incomingDialog);
                KeepaliveMethod keepAliveMethod = itspAccountInfo.getRtpKeepaliveMethod();
                RtpTransmitterEndpoint rtpEndpoint = new RtpTransmitterEndpoint(incomingSession,
                        symmitronClient);
                rtpEndpoint.setKeepAliveMethod(keepAliveMethod);
                incomingSession.setTransmitter(rtpEndpoint);
                rtpEndpoint.setSessionDescription(sessionDescription, true);
            } else if (spiral && replacesHeader == null) {

                /*
                 * This is a spiral. We are going to reuse the port in the incoming INVITE (we
                 * already own this port). Note here that we set the early media flag.
                 */

                if (this.rtpBridge.getState() == BridgeState.RUNNING) {
                    this.rtpBridge.pause(); // Pause to block inbound packets.
                }

                RtpSession rtpSession = DialogContext.getRtpSession(this.referingDialog);
                if (rtpSession == null) {
                    Response errorResponse = SipUtilities.createResponse(serverTransaction,
                            Response.SESSION_NOT_ACCEPTABLE);
                    errorResponse.setReasonPhrase("Could not RtpSession for refering dialog");
                    serverTransaction.sendResponse(errorResponse);
                    if (this.rtpBridge.getState() == BridgeState.PAUSED) {
                        this.rtpBridge.resume();
                    }

                    return;
                }
                tad.setReferingDialog(referingDialog);
                tad.setReferRequest(DialogContext.get(referingDialog).getReferRequest());
                RtpTransmitterEndpoint rtpEndpoint = new RtpTransmitterEndpoint(rtpSession,
                        symmitronClient);
                rtpSession.setTransmitter(rtpEndpoint);
                int keepaliveInterval = Gateway.getMediaKeepaliveMilisec();
                KeepaliveMethod keepaliveMethod = tad.getItspAccountInfo()
                        .getRtpKeepaliveMethod();
                rtpEndpoint.setIpAddressAndPort(keepaliveInterval, keepaliveMethod);

                /*
                 * The RTP session now belongs to the ClientTransaction.
                 */
                this.rtpBridge.addSym(rtpSession);

                if (this.rtpBridge.getState() == BridgeState.PAUSED) {
                    this.rtpBridge.resume(); /* Resume operation. */
                } else if (rtpBridge.getState() == BridgeState.INITIAL) {
                    this.rtpBridge.start();
                }

            } else {
                logger.fatal("Internal error -- case not covered");
                throw new SipXbridgeException("Case not covered");
            }

            /*
             * Set our transaction context pointer.
             */
            ct.setApplicationData(tad);
            /*
             * Add this to the list of managed dialogs.
             */
            this.addDialog(DialogContext.get(ct.getDialog()));
            
            /*
             * Copy any unrecognized that we have not accounted for
             * Stil questionable about whether unknown headers should be copied so I am commenting the
             * following out for the moment.
             */
           // SipUtilities.copyHeaders(incomingRequest,outgoingRequest, true);
           
            
            /*
             * Send the request.
             */
            ct.sendRequest();
            /*
             * Kick off our session timer.
             */
            DialogContext.get(ct.getDialog()).startSessionTimer(
                    itspAccountInfo.getSessionTimerInterval());

        } catch (SdpParseException ex) {
            logger.error("Unexpected exception ", ex);
            CallControlUtilities.sendBadRequestError(serverTransaction, ex);
        } catch (ParseException ex) {
            logger.error("Unexpected exception ", ex);
            CallControlUtilities.sendInternalError(serverTransaction, ex);
        } catch (IOException ex) {
            logger.error("Caught IO exception ", ex);
            CallControlUtilities.sendInternalError(serverTransaction, ex);
        } catch (SymmitronException ex) {
            logger.error("Caught exception ", ex);
            CallControlUtilities.sendServiceUnavailableError(serverTransaction, ex);
        } catch (SipException ex) {
            logger.error("Error occurred during processing of request ", ex);
            CallControlUtilities.sendServiceUnavailableError(serverTransaction, ex);
        } catch (Exception ex) {
            logger.error("Error occurred during processing of request ", ex);
            CallControlUtilities.sendInternalError(serverTransaction, ex);
        }

    }

    /**
     * Retransmits the client transaction to the next hop.
     *
     * @param request
     * @param hops
     */
    void resendInviteToItsp(ClientTransaction clientTransaction) {
        TransactionContext transactionContext = TransactionContext.get(clientTransaction);
        ServerTransaction serverTransaction = transactionContext.getServerTransaction();
        try {
            /*
             * Restart transaction and send to new hop.
             */

            Collection<Hop> hops = transactionContext.getProxyAddresses();
            Iterator<Hop> hopIter = hops.iterator();
            Hop nextHop = hopIter.next();
            hopIter.remove();
            Request request = clientTransaction.getRequest();
            Request newRequest = (Request) request.clone();
            if ( transactionContext.getItspAccountInfo() == null ||
                 transactionContext.getItspAccountInfo().isAddLrRoute() ) {
                RouteHeader routeHeader = SipUtilities.createRouteHeader(nextHop);
                newRequest.setHeader(routeHeader);
            } else {
                SipURI requestUri = (SipURI) newRequest.getRequestURI();
                requestUri.setMAddrParam(nextHop.getHost());
                requestUri.setPort(nextHop.getPort());
            }
            ((ViaHeader) newRequest.getHeader(ViaHeader.NAME)).removeParameter("branch");
            ((FromHeader) newRequest.getHeader(FromHeader.NAME)).removeParameter("tag");
            String newTag = new Integer(Math.abs(new Random().nextInt())).toString();
            ((FromHeader) newRequest.getHeader(FromHeader.NAME)).setTag(newTag);

            DialogContext dialogContext = DialogContext.get(clientTransaction.getDialog());
            SipProvider provider = ((TransactionExt) clientTransaction).getSipProvider();
            newRequest.setContent(request.getContent(), (ContentTypeHeader) request
                    .getHeader(ContentTypeHeader.NAME));

            ClientTransaction newTransaction = provider.getNewClientTransaction(newRequest);

            TransactionContext newContext = TransactionContext.attach(newTransaction,
                    transactionContext.getOperation());

            transactionContext.copyTo(newContext);

            dialogContext.setDialog(newTransaction.getDialog());
            dialogContext.setDialogCreatingTransaction(newTransaction);
            this.addDialog(DialogContext.get(newTransaction.getDialog()));
            newTransaction.sendRequest();
        } catch (ParseException ex) {
            logger.error("Unexpected exception ", ex);
            CallControlUtilities.sendInternalError(serverTransaction, ex);
        } catch (SipException ex) {
            logger.error("Error occurred during processing of request ", ex);
            CallControlUtilities.sendServiceUnavailableError(serverTransaction, ex);
        }

    }

    /**
     * Handle an incoming INVITE with a replaces header.
     *
     * @param requestEvent
     * @param replacedDialog
     * @param serverTransaction
     * @throws Exception
     */
    void handleInviteWithReplaces(RequestEvent requestEvent, Dialog replacedDialog,
            ServerTransaction serverTransaction) throws Exception {
        Request request = requestEvent.getRequest();
        SessionDescription newOffer = SipUtilities.getSessionDescription(request);
        logger.debug("handleInviteWithReplaces: replacedDialog = " + replacedDialog);
        String address = ((ViaHeader) request.getHeader(ViaHeader.NAME)).getHost();
        DialogContext inviteDat = DialogContext.get(serverTransaction.getDialog());

        try {
            RtpSession rtpSession = this.createRtpSession(replacedDialog);
            String ipAddress = SipUtilities.getSessionDescriptionMediaIpAddress(newOffer);
            int port = SipUtilities.getSessionDescriptionMediaPort(newOffer);
            if (rtpSession.getTransmitter() != null) {
                /*
                 * Already has a transmitter then simply redirect the transmitter to the new
                 * location.
                 */
                rtpSession.getTransmitter().setIpAddressAndPort(ipAddress, port);
            } else {
                RtpTransmitterEndpoint transmitter = new RtpTransmitterEndpoint(rtpSession,
                        Gateway.getSymmitronClient(address));
                transmitter.setIpAddressAndPort(ipAddress, port);
                rtpSession.setTransmitter(transmitter);
            }

            if (this.musicOnHoldDialog != null) {
                this.sendByeToMohServer();
            }
            rtpSession.getTransmitter().setOnHold(false);
            
            logger.debug("replacedDialog.getState() : " + replacedDialog.getState());

            DialogContext replacedDialogApplicationData = DialogContext.get(replacedDialog);

            Dialog peerDialog = replacedDialogApplicationData.getPeerDialog();
            DialogContext peerDat = DialogContext.get(peerDialog);

            if (peerDat.getDialogCreatingTransaction() instanceof ClientTransaction) {

                Request reInvite = peerDialog.createRequest(Request.INVITE);
                SipUtilities.addWanAllowHeaders(reInvite);
                if ( peerDat.getSipProvider() != Gateway.getLanProvider()) {
                    if (peerDat.getItspInfo() == null || peerDat.getItspInfo().isGlobalAddressingUsed() ) {
                        SipUtilities.setGlobalAddresses(reInvite);
                    }
                }

                RtpSession wanRtpSession = peerDat.getRtpSession();
                wanRtpSession.getTransmitter().setOnHold(false);
                wanRtpSession.getReceiver().setSessionDescription(newOffer);
                SipUtilities.incrementSessionVersion(newOffer);
                ContentTypeHeader contentTypeHeader = ProtocolObjects.headerFactory
                        .createContentTypeHeader("application", "sdp");
                reInvite.setContent(newOffer.toString(), contentTypeHeader);

                SipProvider provider = ((DialogExt) peerDialog).getSipProvider();
                ClientTransaction ctx = provider.getNewClientTransaction(reInvite);
                TransactionContext tad = TransactionContext.attach(ctx,
                        Operation.HANDLE_INVITE_WITH_REPLACES);
                tad.setServerTransaction(serverTransaction);
                tad.setReplacedDialog(replacedDialog);
                // send the in-dialog re-invite to the other side.
                DialogContext.get(peerDialog).sendReInvite(ctx);
                // Tear down the dialog with the transfer controller.
                replacedDialogApplicationData.sendBye(false);
            } else {
                /*
                 * The following condition happens during call pickup. The other side has not been
                 * sent an SDP answer yet. Extract the OFFER from the in-bound INVITE and send it
                 * as an answer. We have to follow these restrictions: For each "m=" line in the
                 * offer, there MUST be a corresponding "m=" line in the answer. The answer MUST
                 * contain exactly the same number of "m=" lines as the offer. This allows for
                 * streams to be matched up based on their order. This implies that if the offer
                 * contained zero "m=" lines, the answer MUST contain zero "m=" lines.
                 *
                 * The "t=" line in the answer MUST equal that of the offer. The time of the
                 * session cannot be negotiated. An offered stream MAY be rejected in the answer,
                 * for any reason. If a stream is rejected, the offerer and answerer MUST NOT
                 * generate media (or RTCP packets) for that stream. To reject an offered stream,
                 * the port number in the corresponding stream in the answer MUST be set to zero.
                 * Any media formats listed are ignored. At least one MUST be present, as
                 * specified by SDP.
                 */

                SessionDescription sdes = rtpSession.getReceiver().getSessionDescription();

                /*
                 * Inbound INVITE (for call pickup).
                 */
                HashSet<Integer> codecs = SipUtilities.getCommonCodec(sdes, newOffer);

                logger.debug("Codecs = " + codecs);

                if (replacedDialog.getState() != DialogState.CONFIRMED) {
                    if (codecs.size() == 0) {
                        Response errorResponse = SipUtilities.createResponse(serverTransaction,
                                Response.NOT_ACCEPTABLE_HERE);
                        serverTransaction.sendResponse(errorResponse);
                        return;

                    }

                }

                /*
                 * Track the RTP session we were using.
                 */
                inviteDat.rtpSession = rtpSession;

                /*
                 * Generate an OK response to be sent after BYE OK comes in from transfer agent.
                 */
                Response okResponse = SipUtilities.createResponse(serverTransaction, Response.OK);

                ContentTypeHeader cth = ProtocolObjects.headerFactory.createContentTypeHeader(
                        "application", "sdp");
                SipProvider txProvider = ((TransactionExt) serverTransaction).getSipProvider();
                ContactHeader contactHeader = SipUtilities.createContactHeader(
                        Gateway.SIPXBRIDGE_USER, txProvider,
                        SipUtilities.getViaTransport(okResponse));
                okResponse.setHeader(contactHeader);
                /*
                 * This call pickup can ONLY originate from pbx. We cannot handle call pickup
                 * originating from ITSP.
                 */
                SessionDescription localSessionDescription = rtpSession.getReceiver()
                        .getLocalSessionDescription();
                okResponse.setContent(localSessionDescription.toString(), cth);

                DialogContext replacedDat = DialogContext.get(replacedDialog);
                if (replacedDat.dialogCreatingTransaction != null
                        && replacedDat.dialogCreatingTransaction instanceof ClientTransaction) {

                    ClientTransaction ctx = (ClientTransaction) replacedDat.dialogCreatingTransaction;
                    TransactionContext.attach(ctx, Operation.CANCEL_REPLACED_INVITE);
                    Request cancelRequest = ctx.createCancel();
                    if (replacedDat.dialogCreatingTransaction.getState() != TransactionState.TERMINATED) {
                        ClientTransaction cancelTx = txProvider
                                .getNewClientTransaction(cancelRequest);

                        cancelTx.sendRequest();
                    }
                }

                if (replacedDialog.getState() == DialogState.CONFIRMED) {
                    Request byeRequest = replacedDialog.createRequest(Request.BYE);
                    SipProvider provider = ((DialogExt) replacedDialog).getSipProvider();
                    ClientTransaction byeCtx = provider.getNewClientTransaction(byeRequest);

                    /*
                     * Create a little transaction context to identify the operator in our state
                     * machine. when we see a response to the BYE.
                     */
                    TransactionContext.attach(byeCtx, Operation.SEND_BYE_TO_REPLACED_DIALOG);
                    replacedDialog.sendRequest(byeCtx);
                } else if (replacedDialog.getState() != DialogState.TERMINATED) {
                    this.addDialogToCleanup(replacedDialog);
                    DialogContext.get(replacedDialog).setTerminateOnConfirm();
                }
                
                DialogContext.getPeerDialogContext(serverTransaction.getDialog()).setPendingAction(PendingDialogAction.PENDING_SOLICIT_SDP_OFFER_ON_ACK);        
                serverTransaction.sendResponse(okResponse);
                 
                RtpSession wanRtpSession = peerDat.getRtpSession();
                SipProvider wanProvider = ((DialogExt) peerDialog).getSipProvider();
               
                SessionDescription answerSdes = SipUtilities.cleanSessionDescription(SipUtilities
                        .cloneSessionDescription(sdes), codecs);
                wanRtpSession.getReceiver().setSessionDescription(answerSdes);

                ServerTransaction peerSt = ((ServerTransaction) peerDat.dialogCreatingTransaction);
                ContactHeader contact = SipUtilities.createContactHeader(wanProvider, peerDat
                        .getItspInfo(),peerSt);
                if ( peerSt != null && peerSt.getState() != TransactionState.TERMINATED ) {
                	Response peerOk = SipUtilities.createResponse(peerSt,
							Response.OK);   	
					peerOk.setHeader(contact);
					peerOk.setContent(answerSdes, cth);
					peerSt.sendResponse(peerOk);
		         } else {
                	logger.debug("peerSt = " + peerSt);
                	if ( peerSt != null ) {
                		logger.debug("peerSt.getState() : " + peerSt.getState());
                	}
                }
                
                this.getRtpBridge().start();

            }
        } catch (Exception ex) {
            logger.error("Unexpected exception -- tearing down call ", ex);
            try {
                this.tearDown(ProtocolObjects.headerFactory.createReasonHeader(
                        Gateway.SIPXBRIDGE_USER, ReasonCode.SIPXBRIDGE_INTERNAL_ERROR, ex
                                .getMessage()));
            } catch (Exception e) {
                logger.error("Unexpected exception ", e);
            }
        }

    }

    /**
     * Handle a bye on one of the dialogs of this b2bua.
     *
     * @param dialog
     * @throws SipException
     */
    void processBye(RequestEvent requestEvent) throws SipException {
        Dialog dialog = requestEvent.getDialog();
        logger.debug("processBye : BYE received on dialog " + dialog);
        ServerTransaction st = requestEvent.getServerTransaction();
        CallControlUtilities.sendTryingResponse(st);

        DialogContext dialogContext = (DialogContext) dialog.getApplicationData();
        Dialog peer = dialogContext.getPeerDialog();
        
        if (!SipUtilities.isRequestNotForwarded(st.getRequest())
                && dialogContext.isForwardByeToPeer() && peer != null
                && peer.getState() != DialogState.TERMINATED && peer.getState() != null) {
            if (peer.getState() == DialogState.EARLY) {
                DialogContext peerDialogContext = DialogContext.get(peer);
                if (peerDialogContext.getDialogCreatingTransaction() != null &&
                		peerDialogContext.getDialogCreatingTransaction() instanceof ClientTransaction ) {
                	 ClientTransaction ctx = (ClientTransaction) peerDialogContext.getDialogCreatingTransaction();
                	 Request cancelRequest = ctx.createCancel();
                	 SipUtilities.addWanAllowHeaders(cancelRequest);
                	 SipProvider peerProvider = peerDialogContext.getSipProvider();
                	 ClientTransaction cancelCtx = peerProvider.getNewClientTransaction(cancelRequest);
                	 TransactionContext transactionContext = new TransactionContext(cancelCtx,Operation.CANCEL_INVITE);
                	 cancelCtx.sendRequest();
                }
                Gateway.getTimer().schedule(new DelayedByeSender(peer, st), 1000);
                Response response = SipUtilities.createResponse(st, Response.OK);
                try {
					st.sendResponse(response);
				} catch (InvalidArgumentException e) {
					throw new SipXbridgeException("Unexpected exception",e);
				}
            } else {
                if (requestEvent.getServerTransaction() != null) {
                    TransactionContext.attach(requestEvent.getServerTransaction(),
                            Operation.PROCESS_BYE);
                    DialogContext.getPeerDialogContext(dialog).forwardBye(
                            requestEvent.getServerTransaction());
                }
            }

        } else {
            /*
             * Peer dialog is not yet established or is terminated.
             */
            logger.debug("BackToBackUserAgent: peerDialog = " + peer);
            if (peer != null) {
                logger.debug("BackToBackUserAgent: peerDialog state = " + peer.getState());
            }
            try {
                Response ok = SipUtilities.createResponse(st, Response.OK);
                st.sendResponse(ok);

            } catch (InvalidArgumentException ex) {
                logger.error("Unexpected exception", ex);
            }
        }

    }

    /**
     * Return true if theres stuff in the dialog table and false otherwise.
     *
     * @return
     */
    boolean isEmpty() {
        return this.dialogTable.isEmpty();
    }

    /**
     * @return the rtpBridge
     */
    RtpBridge getRtpBridge() {
        return rtpBridge;
    }

    /**
     * Get XML RPC client interface.
     *
     * @return
     */

    SymmitronClient getSymmitronClient() {
        return symmitronClient;
    }

    /**
     * Terminate the two sides of the bridge with no Reason header.
     *
     */
    void tearDown() {
        try {
            this.tearDown(null);
        } catch (Exception ex) {
            logger.error("Unexpected exception tearing down call", ex);
        }
    }

    /**
     * @return the symmitronServerHandle
     */
    String getSymmitronServerHandle() {
        return symmitronServerHandle;
    }

    /**
     * tear down dialogs.
     */
    void tearDown(String agent, int code, String reason) throws Exception {
        ReasonHeader rh = ProtocolObjects.headerFactory.createReasonHeader(agent, code, reason);
        this.tearDown(rh);
    }

    /**
     * Tear down all the dialogs with specified reason header.
     *
     * @param reason
     * @throws Exception
     */

    void tearDown(ReasonHeader reason) throws Exception {
        /*
         * Prevent a concurrent modification exception. Copy all the dialogs into a temp set.
         */
        HashSet<Dialog> temp = new HashSet<Dialog>();
        temp.addAll(this.dialogTable);

        for (Dialog dialog : temp) {
            logger.debug("tearing down " + dialog + " dialogState = " + dialog.getState()
                    + " dialog.isServer " + dialog.isServer());
            if (dialog.getState() != DialogState.TERMINATED) {
                DialogContext dialogCtx = DialogContext.get(dialog);
                SipProvider lanProvider = ((DialogExt) dialog).getSipProvider();
                if (dialogCtx != null
                        && dialogCtx.getDialogCreatingTransaction() != null
                        && dialogCtx.getDialogCreatingTransaction().getState() != TransactionState.TERMINATED
                        && dialogCtx.getDialogCreatingTransaction() instanceof ClientTransaction) {
                    ClientTransaction ctx = (ClientTransaction) dialogCtx
                            .getDialogCreatingTransaction();
                    try {
                        if (ctx.getState() == TransactionState.PROCEEDING) {
                            ClientTransaction cancelTx = lanProvider.getNewClientTransaction(ctx
                                    .createCancel());
                            TransactionContext.attach(cancelTx, Operation.CANCEL_INVITE);
                            cancelTx.sendRequest();
                        }
                    } catch (TransactionAlreadyExistsException ex) {
                        logger.debug("CANCEL Already issued on transaction");
                    } catch (TransactionUnavailableException ex) {
                        logger.debug("Too late to CANCEL the transaction");
                    }
                }

                /*
                 * Cannot send BYE to a Dialog in EARLY state.
                 */
                if (dialog.getState() != null
                        && (!dialog.isServer() || dialog.getState() != DialogState.EARLY)) {
                    Request byeRequest = dialog.createRequest(Request.BYE);
                    if (reason != null) {
                        byeRequest.addHeader(reason);
                    }
                    SipProvider provider = ((DialogExt) dialog).getSipProvider();
                    ClientTransaction ct = provider.getNewClientTransaction(byeRequest);
                    TransactionContext.attach(ct, Operation.SEND_BYE_FOR_TEARDOWN);
                    dialog.sendRequest(ct);
                } else {
                    DialogContext.get(dialog).setTerminateOnConfirm();
                }

            }
        }

        /* Clean up the MOH dialog */

        if (this.musicOnHoldDialog != null) {
            if (this.musicOnHoldDialog.getState() != null
                    && this.musicOnHoldDialog.getState() != DialogState.TERMINATED) {
                this.sendByeToMohServer();
            } else {
                DialogContext.get(this.musicOnHoldDialog).setTerminateOnConfirm();
            }
        }

        /*
         * Garbage collect those dialogs that were supposed to be terminated during normal
         * processing.
         */
        for (Dialog dialog : this.cleanupList) {
            if (dialog.getState() != null && dialog.getState() != DialogState.TERMINATED
                    && dialog.getState() != DialogState.EARLY) {
                Request byeRequest = dialog.createRequest(Request.BYE);
                if (reason != null) {
                    byeRequest.addHeader(reason);
                }
                logger.debug("Tear down call " + dialog);
                ClientTransaction ctx = ((DialogExt) dialog).getSipProvider()
                        .getNewClientTransaction(byeRequest);
                TransactionContext.attach(ctx, Operation.SEND_BYE_FOR_TEARDOWN);
                dialog.sendRequest(ctx);
            } else if (dialog.getState() != DialogState.TERMINATED) {
                dialog.delete();
            }
        }

    }

    /**
     * @return the musicOnHoldDialog
     */
    Dialog getMusicOnHoldDialog() {
        return musicOnHoldDialog;
    }

    /**
     * Return true if MOH is disabled.
     */
    boolean isMohDisabled() {
        return this.mohDisabled;
    }

    /**
     * Return true if this structure manages a given call Id
     */
    boolean managesCallId(String callId) {
        return myCallIds.contains(callId);
    }

    void setPendingTermination(boolean pendingTermination) {
        this.pendingTermination = pendingTermination;
    }

    /**
     * Return true if this is pending termination.
     *
     * @return
     */
    boolean isPendingTermination() {
        return this.pendingTermination;
    }

    boolean findNextSipXProxy() throws IOException, SymmitronException, SipXbridgeException {

        if (this.proxyAddress != null) {
            this.blackListedProxyServers.add(this.proxyAddress);
            this.proxyAddress = null;

        }

        for (Hop hop : Gateway.initializeSipxProxyAddresses()) {

            try {
                SymmitronClient symmitronClient = Gateway.getSymmitronClient(hop.getHost());
                /* Find if sipxrelay is alive */
                if (!symmitronClient.pingAndTest(hop.getHost(), hop.getPort())) {
                    this.blackListedProxyServers.add(hop);
                    continue;
                }

                this.proxyAddress = hop;
                this.symmitronClient = symmitronClient;
                break;

            } catch (SymmitronException ex) {
                logger.error("Could not contact Relay at " + hop.getHost());
                this.blackListedProxyServers.add(hop);
            }
        }

        return (this.proxyAddress != null);
    }

    @Override
    public int compareTo(Object obj) {
        if (obj instanceof BackToBackUserAgent) {
            BackToBackUserAgent b2bua = (BackToBackUserAgent) obj;
            if (this.creatingCallId.equals(b2bua.creatingCallId))
                return 0;
            else if (this.creatingCallId.hashCode() < b2bua.creatingCallId.hashCode())
                return -1;
            else
                return 1;
        } else {
            throw new UnsupportedOperationException("Cannot compare");
        }
    }


    void tearDownNow() {
        logger.debug("tearDownNow " + this);
        if ( this.rtpBridge != null ) this.rtpBridge.stop();
        Gateway.getBackToBackUserAgentFactory().removeBackToBackUserAgent(this);
        this.tearDown();
    }
    
    

   

}
