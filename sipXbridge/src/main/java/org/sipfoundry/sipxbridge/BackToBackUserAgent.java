/*
 *  Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 *  Contributors retain copyright to elements licensed under a Contributor Agreement.
 *  Licensed to the User under the LGPL license.
 *
 */
package org.sipfoundry.sipxbridge;

import gov.nist.javax.sip.DialogExt;
import gov.nist.javax.sip.SipStackExt;
import gov.nist.javax.sip.TransactionExt;
import gov.nist.javax.sip.header.HeaderFactoryExt;
import gov.nist.javax.sip.header.extensions.ReferredByHeader;
import gov.nist.javax.sip.header.extensions.ReplacesHeader;
import gov.nist.javax.sip.message.SIPMessage;

import java.io.IOException;
import java.net.URLDecoder;
import java.text.ParseException;
import java.util.HashSet;
import java.util.Iterator;
import java.util.LinkedList;
import java.util.List;
import java.util.Random;
import java.util.Set;

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
import javax.sip.TransactionState;
import javax.sip.address.Address;
import javax.sip.address.SipURI;
import javax.sip.header.CSeqHeader;
import javax.sip.header.CallIdHeader;
import javax.sip.header.CallInfoHeader;
import javax.sip.header.ContactHeader;
import javax.sip.header.ContentTypeHeader;
import javax.sip.header.FromHeader;
import javax.sip.header.Header;
import javax.sip.header.InReplyToHeader;
import javax.sip.header.MaxForwardsHeader;
import javax.sip.header.OrganizationHeader;
import javax.sip.header.ReasonHeader;
import javax.sip.header.ReferToHeader;
import javax.sip.header.ReplyToHeader;
import javax.sip.header.SubjectHeader;
import javax.sip.header.ToHeader;
import javax.sip.header.ViaHeader;
import javax.sip.header.WarningHeader;
import javax.sip.message.Request;
import javax.sip.message.Response;

import org.apache.log4j.Logger;
import org.sipfoundry.sipxbridge.symmitron.BridgeInterface;
import org.sipfoundry.sipxbridge.symmitron.BridgeState;
import org.sipfoundry.sipxbridge.symmitron.KeepaliveMethod;
import org.sipfoundry.sipxbridge.symmitron.SymImpl;
import org.sipfoundry.sipxbridge.symmitron.SymmitronClient;

/**
 * A class that represents an ongoing call. Each call Id points at one of these
 * structures. It can be a many to one mapping. When we receive an incoming
 * request we retrieve the corresponding backtobackuseragent and route the
 * request to it. It keeps the necessary state to handle subsequent requests
 * that are related to this call.
 * 
 * @author M. Ranganathan
 * 
 */
public class BackToBackUserAgent {

	/*
	 * Constants that we stick into the VIA header to detect spirals.
	 */
	static final String ORIGINATOR = "originator";

	private RtpBridge rtpBridge;

	/*
	 * This is just a table of dialogs that reference this B2bua. When the table
	 * is empty the b2bua is GCd.
	 */
	HashSet<Dialog> dialogTable = new HashSet<Dialog>();

	/*
	 * The REFER dialog currently in progress.
	 */
	Dialog referingDialog;

	private Dialog referingDialogPeer;

	/*
	 * For generation of call ids ( so we can filter logs easily)
	 */
	private int counter;

	/*
	 * Any call Id associated with this b2bua will be derived from this base
	 * call id.
	 */
	private String creatingCallId;

	private static Logger logger = Logger.getLogger(BackToBackUserAgent.class);

	private SymmitronClient symmitronClient;

	private String symmitronServerHandle;

	/*
	 * A given B2BUA has one pair of dialogs one for the WAN and one for the LAN
	 * and additionally possibly a MOH dialog which is signaled when the WAN
	 * side is put on hold.
	 */

	private Dialog musicOnHoldDialog;

	private boolean mohDisabled;

	private ClientTransaction musicOnHoldInviteTransaction;

	// ///////////////////////////////////////////////////////////////////////
	// Constructor.
	// ///////////////////////////////////////////////////////////////////////
	@SuppressWarnings("unused")
	private BackToBackUserAgent() {

	}

	BackToBackUserAgent(SipProvider provider, Request request, Dialog dialog,
			ItspAccountInfo itspAccountInfo) throws IOException {

		/*
		 * Received this request from the LAN The symmitron to use is the
		 * symmitron that runs where the request originated from. If received
		 * this request from the LAN side then we look at the topmost via header
		 * and try to contact the server at that address.
		 */
		if (provider == Gateway.getLanProvider()) {
			ViaHeader viaHeader = (ViaHeader) request.getHeader(ViaHeader.NAME);
			/*
			 * If we have a received header, then use that header to look for a
			 * symmmitron there. Otherwise use the Via header. The symmitron
			 * must be up and running by the time the request is seen at the
			 * server.
			 */
			String address = (viaHeader.getReceived() != null ? viaHeader
					.getReceived() : viaHeader.getHost());
			this.symmitronClient = Gateway.getSymmitronClient(address);
		} else {
			this.symmitronClient = Gateway.getSymmitronClient(Gateway
					.getLocalAddress());
		}

		BridgeInterface bridge = symmitronClient.createBridge();
		this.symmitronServerHandle = symmitronClient.getServerHandle();
		rtpBridge = new RtpBridge(request, bridge);
		dialogTable.add(dialog);
		if (itspAccountInfo == null || !itspAccountInfo.stripPrivateHeaders()) {
			/*
			 * If privacy is not desired we use the incoming callid and generate
			 * new call ids off that one. This makes life a bit easier when
			 * extracting traces.
			 */
			this.creatingCallId = ((CallIdHeader) request
					.getHeader(CallIdHeader.NAME)).getCallId();
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
	 * RTP session that is connected to the WAN Side. Note that we use a
	 * different method here because we need to record additionally whether or
	 * not to use global addressing in the RTP session descriptor associated
	 * with the receiver.
	 * 
	 * @return the rtp session created.
	 */

	RtpSession createRtpSession(Dialog dialog) {
		RtpSession rtpSession = DialogContext.getRtpSession(dialog);
		SipProvider provider = ((DialogExt) dialog).getSipProvider();
		DialogContext dialogApplicationData = DialogContext.get(dialog);

		if (rtpSession == null) {
			if (Gateway.getLanProvider() == provider) {
				if (dialogApplicationData.getRtpSession() == null) {
					SymImpl symImpl = symmitronClient.createEvenSym();
					rtpSession = new RtpSession(symImpl);
					dialogApplicationData.setRtpSession(rtpSession);
					this.rtpBridge.addSym(rtpSession);
				}

			} else {
				SymImpl symImpl = symmitronClient.createEvenSym();
				rtpSession = new RtpSession(symImpl);
				rtpSession.getReceiver().setGlobalAddress(
						symmitronClient.getPublicAddress());
				rtpSession.getReceiver().setUseGlobalAddressing(
						dialogApplicationData.getItspInfo() == null
								|| dialogApplicationData.getItspInfo()
										.isGlobalAddressingUsed());
				DialogContext.get(dialog).setRtpSession(rtpSession);
				this.rtpBridge.addSym(rtpSession);
			}
		}
		return dialogApplicationData.getRtpSession();
	}

	/**
	 * This method handles an Invite with a replaces header in it. It is invoked
	 * for consultative transfers. It does a Dialog splicing and media splicing
	 * on the Refer dialog. Here is the reference call flow for this case:
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
	 * @param serverTransaction
	 *            - Inbound server transaction we are processing.
	 * @param toDomain
	 *            -- domain to send the request to.
	 * @param requestEvent
	 *            -- the request event we are processing.
	 * @param replacedDialog
	 *            -- the dialog we are replacing.
	 * @throws SipException
	 */
	void handleSpriralInviteWithReplaces(RequestEvent requestEvent,
			Dialog replacedDialog, ServerTransaction serverTransaction,
			String toDomain) throws SipException {
		/* The inbound INVITE */
		SipProvider provider = (SipProvider) requestEvent.getSource();
		try {
			Dialog replacedDialogPeerDialog = ((DialogContext) replacedDialog
					.getApplicationData()).getPeerDialog();
			if (logger.isDebugEnabled()) {
				logger.debug("replacedDialogPeerDialog = "
						+ ((DialogContext) replacedDialog.getApplicationData())
								.getPeerDialog());
				logger.debug("referingDialogPeerDialog = "
						+ this.referingDialogPeer);

			}

			if (replacedDialogPeerDialog.getState() == DialogState.TERMINATED) {
				Response response = SipUtilities.createResponse(
						serverTransaction, Response.BUSY_HERE);
				serverTransaction.sendResponse(response);
				return;
			}

			if (this.referingDialogPeer.getState() == DialogState.TERMINATED) {
				Response response = SipUtilities.createResponse(
						serverTransaction, Response.BUSY_HERE);
				serverTransaction.sendResponse(response);
				return;
			}

			DialogContext.pairDialogs(((DialogContext) replacedDialog
					.getApplicationData()).getPeerDialog(),
					this.referingDialogPeer);
			/*
			 * Tear down the Music On Hold Dialog if any.
			 */
			this.sendByeToMohServer();

			/* The replaced dialog is about ready to die so he has no peer */
			((DialogContext) replacedDialog.getApplicationData())
					.setPeerDialog(null);

			if (logger.isDebugEnabled()) {
				logger.debug("referingDialog = " + referingDialog);
				logger.debug("replacedDialog = " + replacedDialog);
			}

			/*
			 * We need to form a new bridge. Remove the refering dialog from our
			 * rtp bridge. Remove the replacedDialog from its rtpBridge and form
			 * a new bridge.
			 */
			rtpBridge.pause();

			Set<RtpSession> myrtpSessions = this.rtpBridge.getSyms();
			DialogContext replacedDialogApplicationData = (DialogContext) replacedDialog
					.getApplicationData();
			RtpBridge hisBridge = replacedDialogApplicationData
					.getBackToBackUserAgent().rtpBridge;
			hisBridge.pause();

			Set<RtpSession> hisRtpSessions = hisBridge.getSyms();
			BridgeInterface bridge = symmitronClient.createBridge();

			RtpBridge newBridge = new RtpBridge(bridge);

			for (Iterator<RtpSession> it = myrtpSessions.iterator(); it
					.hasNext();) {
				RtpSession sym = it.next();
				if (sym != DialogContext.getRtpSession(replacedDialog)
						&& sym != DialogContext.getRtpSession(referingDialog)) {
					newBridge.addSym(sym);
					it.remove();
				}
			}

			for (Iterator<RtpSession> it = hisRtpSessions.iterator(); it
					.hasNext();) {
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
				logger.debug("replacedDialog State "
						+ replacedDialog.getState());
			}

			if (replacedDialog.getState() != DialogState.TERMINATED
					&& replacedDialog.getState() != DialogState.EARLY) {
				Request byeRequest = replacedDialog.createRequest(Request.BYE);
				ClientTransaction byeCtx = ((DialogExt) replacedDialog)
						.getSipProvider().getNewClientTransaction(byeRequest);
				TransactionContext.attach(byeCtx,
						Operation.HANDLE_SPIRAL_INVITE_WITH_REPLACES);
				replacedDialog.sendRequest(byeCtx);

			}

			Response response = SipUtilities.createResponse(serverTransaction,
					Response.OK);

			ContactHeader contactHeader = SipUtilities.createContactHeader(
					Gateway.SIPXBRIDGE_USER, provider);
			response.setHeader(contactHeader);

			/*
			 * If re-INVITE is supported send the INVITE down the other leg.
			 * Note that at this point we have already queried the Sdp from the
			 * other leg. We will ACK that with the response received.
			 */

			if (Gateway.getAccountManager().getBridgeConfiguration()
					.isReInviteSupported()) {
				Dialog referingDialogPeer = this.referingDialogPeer;

				DialogContext referingDialogPeerApplicationData = DialogContext
						.get(referingDialogPeer);
				Response lastResponse = referingDialogPeerApplicationData
						.getLastResponse();
				DialogContext replacedDialogPeerDialogApplicationData = DialogContext
						.get(replacedDialogPeerDialog);
				Request reInvite = replacedDialogPeerDialog
						.createRequest(Request.INVITE);

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

				replacedDialogPeerDialogApplicationData.getRtpSession()
						.getReceiver()
						.setSessionDescription(sessionDescription);

				SipUtilities.incrementSessionVersion(sessionDescription);

				reInvite.setContent(sessionDescription.toString(),
						ProtocolObjects.headerFactory.createContentTypeHeader(
								"application", "sdp"));
				SipProvider wanProvider = ((DialogExt) replacedDialogPeerDialog)
						.getSipProvider();
				ClientTransaction ctx = wanProvider
						.getNewClientTransaction(reInvite);
				TransactionContext.attach(ctx,
						Operation.HANDLE_SPIRAL_INVITE_WITH_REPLACES);
				replacedDialogPeerDialog.sendRequest(ctx);
			}
			serverTransaction.sendResponse(response);

		} catch (Exception ex) {
			logger.error("Unexpected internal error occured", ex);

			CallControlUtilities.sendBadRequestError(serverTransaction, ex);
		}

	}

	/**
	 * Remove a dialog from the table ( the dialog terminates ). This is a
	 * garbage collection routine. When all the dialogs referencing this
	 * structure have terminated, then we send BYE to the MOH dialog ( if it is
	 * still alive ).
	 * 
	 * @param dialog
	 *            -- the terminated dialog.
	 */
	synchronized void removeDialog(Dialog dialog) {

		this.dialogTable.remove(dialog);

		int count = 0;

		for (Dialog d : dialogTable) {
			DialogContext dat = DialogContext.get(d);
			if (!dat.isOriginatedBySipxbridge) {
				count++;
			}
		}

		if (count == 0) {
			for (Dialog d : dialogTable) {
				d.delete();
			}
			this.rtpBridge.stop();
			dialogTable.clear();
		}

		if (logger.isDebugEnabled()) {
			logger.debug("Remove Dialog " + dialog + " Dialog table size = "
					+ this.dialogTable.size());
		}

		if (dialogTable.size() == 0) {
			this.sendByeToMohServer();
			Gateway.getBackToBackUserAgentFactory().removeBackToBackUserAgent(
					this);
		}

	}

	/**
	 * Add a dialog entry to the b2bua. This implies that the given dialog holds
	 * a pointer to this structure.
	 * 
	 * @param provider
	 * @param dialog
	 */
	synchronized void addDialog(Dialog dialog) {
		this.dialogTable.add(dialog);

	}

	/**
	 * Create an INVITE request from an in-bound REFER. Note that the only
	 * reason why this is here and not in SipUtilites is because we need the
	 * callId counter - should it be moved?
	 * 
	 * 
	 * @param requestEvent
	 *            -- the in-bound REFER request Event.
	 * 
	 * @return the INVITE request crafted from the IB Refer
	 * 
	 */
	@SuppressWarnings("unchecked")
	Request createInviteFromReferRequest(RequestEvent requestEvent)
			throws SipException, ParseException, IOException {
		try {
			Dialog dialog = requestEvent.getDialog();
			ServerTransaction referServerTransaction = requestEvent
					.getServerTransaction();
			Request referRequest = referServerTransaction.getRequest();
			DialogContext dialogContext = DialogContext.get(dialog);
			FromHeader fromHeader = (FromHeader) dialogContext.request
					.getHeader(FromHeader.NAME).clone();
			fromHeader.removeParameter("tag");

			ToHeader toHeader = (ToHeader) referRequest
					.getHeader(ToHeader.NAME).clone();
			toHeader.removeParameter("tag");
			/*
			 * Get the Refer-To header and convert it into an INVITE to send to
			 * the REFER target.
			 */

			ReferToHeader referToHeader = (ReferToHeader) referRequest
					.getHeader(ReferToHeader.NAME);
			SipURI uri = (SipURI) referToHeader.getAddress().getURI().clone();
			CSeqHeader cseq = ProtocolObjects.headerFactory.createCSeqHeader(
					1L, Request.INVITE);
			ViaHeader viaHeader = SipUtilities.createViaHeader(Gateway
					.getLanProvider(), Gateway.getSipxProxyTransport());
			List viaList = new LinkedList();
			viaList.add(viaHeader);
			MaxForwardsHeader maxForwards = ProtocolObjects.headerFactory
					.createMaxForwardsHeader(20);
			CallIdHeader callId = ProtocolObjects.headerFactory
					.createCallIdHeader(this.creatingCallId + "."
							+ this.counter++);

			Request newRequest = ProtocolObjects.messageFactory.createRequest(
					uri, Request.INVITE, callId, cseq, fromHeader, toHeader,
					viaList, maxForwards);

			/*
			 * Does the refer to header contain a Replaces? ( attended transfer
			 * )
			 */
			String replacesParam = uri.getHeader(ReplacesHeader.NAME);
			logger.debug("replacesParam = " + replacesParam);

			ReplacesHeader replacesHeader = null;

			if (replacesParam != null) {

				String decodedReplaces = URLDecoder.decode(replacesParam,
						"UTF-8");
				replacesHeader = (ReplacesHeader) ProtocolObjects.headerFactory
						.createHeader(ReplacesHeader.NAME, decodedReplaces);
				newRequest.addHeader(replacesHeader);
			}

			uri.removeParameter(ReplacesHeader.NAME);

			for (Iterator it = uri.getHeaderNames(); it.hasNext();) {
				String headerName = (String) it.next();
				String headerValue = uri.getHeader(headerName);
				Header header = null;
				if (headerValue != null) {
					String decodedHeaderValue = URLDecoder.decode(headerValue,
							"UTF-8");
					header = (Header) ProtocolObjects.headerFactory
							.createHeader(headerName, decodedHeaderValue);
				}
				if (header != null) {
					newRequest.addHeader(header);
				}
			}

			/*
			 * Remove any header parameters - we have already dealt with them
			 * above.
			 */
			((gov.nist.javax.sip.address.SipURIExt) uri).removeHeaders();

			if (referRequest.getHeader(ReferredByHeader.NAME) != null) {
				newRequest.setHeader(referRequest
						.getHeader(ReferredByHeader.NAME));
			}

			Gateway.getBackToBackUserAgentFactory().setBackToBackUserAgent(
					callId.getCallId(), this);

			SipUtilities.addLanAllowHeaders(newRequest);

			String fromUser = ((SipURI) fromHeader.getAddress().getURI())
					.getUser();
			ContactHeader contactHeader = SipUtilities.createContactHeader(
					fromUser, Gateway.getLanProvider());
			newRequest.setHeader(contactHeader);
			/*
			 * Create a new out of dialog request.
			 */
			toHeader.getAddress().setURI(uri);

			fromHeader.setTag(Integer
					.toString(Math.abs(new Random().nextInt())));
			ContentTypeHeader cth = ProtocolObjects.headerFactory
					.createContentTypeHeader("application", "sdp");
			newRequest.setHeader(fromHeader);

			RtpSession lanRtpSession = this.createRtpSession(dialog);
			SessionDescription sd = lanRtpSession.getReceiver()
					.getSessionDescription();
			SipUtilities.setDuplexity(sd, "sendrecv");
			newRequest.setContent(sd, cth);

			return newRequest;
		} catch (InvalidArgumentException ex) {
			throw new SipXbridgeException(ex);
		}

	}

	/**
	 * This method is called when the REFER is received at the B2BUA. Since
	 * ITSPs do not handle REFER, we convert it to a re-INVITE to solicit an
	 * offer. To determine the codec that was negotiated in the original Call
	 * Setup, we send an INVITE (no-sdp) to the dialog to solicit an offer. This
	 * operation has already returned a result when this method is called. This
	 * method is called when the response for that solicitation is received. We
	 * need to direct an INVITE to the contact mentioned in the Refer. Notice
	 * that when this method is called, we have already created an INVITE
	 * previously that we will now replay with the SDP answer to the PBX side.
	 * 
	 * 
	 * @param inviteRequest
	 *            -- the INVITE request.
	 * @param referRequestEvent
	 *            -- the REFER dialog
	 * @param sessionDescription
	 *            -- the session description to use when forwarding the INVITE
	 *            to the sipx proxy server.
	 * @param dialogPendingSdpAnswer
	 *            -- the dialog that is pending SDP answer in ACK after the OK
	 *            comes in with an SDP.
	 * 
	 */

	void referInviteToSipxProxy(Request inviteRequest,
			ClientTransaction mohClientTransaction,
			Dialog dialogPendingSdpAnswer, RequestEvent referRequestEvent,
			SessionDescription sessionDescription) {
		logger.debug("referInviteToSipxProxy: ");

		try {

			Dialog referRequestEventDialog = referRequestEvent.getDialog();
			Request referRequest = referRequestEvent.getRequest();
			ServerTransaction stx = referRequestEvent.getServerTransaction();

			/*
			 * Transfer agent canceled the REFER before we had a chance to
			 * process it.
			 */
			if (referRequestEventDialog == null
					|| referRequestEventDialog.getState() == DialogState.TERMINATED) {
				/*
				 * Out of dialog refer.
				 */
				Response response;
				if (stx != null) {
					response = SipUtilities.createResponse(stx,
							Response.NOT_ACCEPTABLE);
				} else {
					response = ProtocolObjects.messageFactory.createResponse(
							Response.NOT_ACCEPTABLE, referRequest);
				}
				WarningHeader warning = ProtocolObjects.headerFactory
						.createWarningHeader(Gateway.SIPXBRIDGE_USER,
								WarningCode.OUT_OF_DIALOG_REFER,
								"Out of dialog REFER");
				response.setHeader(SipUtilities.createContactHeader(null,
						((SipProvider) referRequestEvent.getSource())));
				response.setHeader(warning);
				if (stx != null) {
					stx.sendResponse(response);
				} else {
					((SipProvider) referRequestEvent.getSource())
							.sendResponse(response);
				}

				return;
			}

			/*
			 * Create a new client transaction. First attach any queried session
			 * description.
			 */
			if (sessionDescription != null) {
				SipUtilities.setDuplexity(sessionDescription, "sendrecv");
				SipUtilities.setSessionDescription(inviteRequest,
						sessionDescription);
			}
			ClientTransaction ct = Gateway.getLanProvider()
					.getNewClientTransaction(inviteRequest);

			DialogContext newDialogContext = DialogContext.attach(this, ct
					.getDialog(), ct, ct.getRequest());
			DialogContext referDialogContext = (DialogContext) referRequestEventDialog
					.getApplicationData();

			newDialogContext.rtpSession = referDialogContext.rtpSession;
			newDialogContext.setPeerDialog(referDialogContext.getPeerDialog());

			if (logger.isDebugEnabled()) {
				logger.debug("referInviteToSipxProxy peerDialog = "
						+ newDialogContext.getPeerDialog());

			}

			this.referingDialogPeer = referDialogContext.getPeerDialog();

			DialogContext dat = (DialogContext) this.referingDialogPeer
					.getApplicationData();
			dat.setPeerDialog(ct.getDialog());

			/*
			 * Mark that we (sipxbridge) originated the dialog.
			 */
			newDialogContext.isOriginatedBySipxbridge = true;

			/*
			 * Record the referDialog so that when responses for the Client
			 * transaction come in we can NOTIFY the referrer.
			 */
			this.referingDialog = referRequestEventDialog;

			ct.getDialog().setApplicationData(newDialogContext);
			/*
			 * Mark that when we get the OK we want to re-INVITE the other side.
			 */
			newDialogContext
					.setPendingAction(PendingDialogAction.PENDING_RE_INVITE_WITH_SDP_OFFER);
			newDialogContext.setPeerDialog(referDialogContext.getPeerDialog());

			TransactionContext tad = TransactionContext.attach(ct,
					Operation.REFER_INVITE_TO_SIPX_PROXY);
			tad.setBackToBackUa(((DialogContext) referRequestEventDialog
					.getApplicationData()).getBackToBackUserAgent());
			tad.setReferingDialog(referRequestEventDialog);

			tad.setReferRequest(referRequest);

			tad.setDialogPendingSdpAnswer(dialogPendingSdpAnswer);

			tad.setMohClientTransaction(mohClientTransaction);

			/*
			 * Stamp the via header with our stamp so that we know we Referred
			 * this request. we will use this for spiral detection.
			 */
			ViaHeader via = (ViaHeader) inviteRequest.getHeader(ViaHeader.NAME);
			/*
			 * This is our signal that we originated the redirection. We use
			 * this in the INVITE processing below. SIPX will not strip any via
			 * header parameters.If the INVITE spirals back to us, we need to
			 * know that it is a spiral( see processing above that checks this
			 * flag).
			 */
			via.setParameter(ORIGINATOR, Gateway.SIPXBRIDGE_USER);

			/*
			 * Send the request. Note that this is not an in-dialog request.
			 */
			ct.sendRequest();

			/*
			 * We have the INVITE now so send an ACCEPTED to the REFER agent.
			 */
			Response response = ProtocolObjects.messageFactory.createResponse(
					Response.ACCEPTED, referRequest);
			response.setHeader(SipUtilities.createContactHeader(null,
					((SipProvider) referRequestEvent.getSource())));
			stx.sendResponse(response);

		} catch (ParseException ex) {
			logger.error("Unexpected parse exception", ex);
			throw new SipXbridgeException("Unexpected parse exception", ex);

		} catch (Exception ex) {
			logger
					.error("Error while processing the request - hanging up ",
							ex);
			try {
				this.tearDown();
			} catch (Exception e) {
				logger.error("Unexpected exception tearing down session", e);
			}
		}

	}

	/**
	 * Forward the REFER request to the ITSP. This only works if the ITSP
	 * actually supports REFER. It is not used but we keep it around for future
	 * use.
	 * 
	 * 
	 * @param requestEvent
	 *            - INBOUND REFER ( from sipx )
	 */
	void forwardReferToItsp(RequestEvent requestEvent) {

		try {
			Dialog dialog = requestEvent.getDialog();
			Dialog peerDialog = DialogContext.getPeerDialog(dialog);
			Request referRequest = requestEvent.getRequest();
			ItspAccountInfo itspInfo = DialogContext.get(peerDialog)
					.getItspInfo();
			SipProvider wanProvider = ((DialogExt) peerDialog).getSipProvider();

			Request outboundRefer = peerDialog.createRequest(Request.REFER);

			ReferToHeader referToHeader = (ReferToHeader) referRequest
					.getHeader(ReferToHeader.NAME);
			SipURI uri = (SipURI) referToHeader.getAddress().getURI();
			String referToUserName = uri.getUser();

			SipURI forwardedReferToUri = SipUtilities
					.createInboundRequestUri(itspInfo);

			forwardedReferToUri.setParameter("target", referToUserName);

			Address referToAddress = ProtocolObjects.addressFactory
					.createAddress(forwardedReferToUri);

			SipURI forwardedReferredByUri = SipUtilities
					.createInboundReferredByUri(itspInfo);

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
			TransactionContext tad = TransactionContext.attach(
					outboundReferClientTx, Operation.FORWARD_REFER);
			tad.setServerTransaction(requestEvent.getServerTransaction());
			peerDialog.sendRequest(outboundReferClientTx);

		} catch (SipException ex) {
			logger
					.error("Error while processing the request - hanging up ",
							ex);
			try {
				this.tearDown(ProtocolObjects.headerFactory.createReasonHeader(
						Gateway.SIPXBRIDGE_USER, ReasonCode.PROTOCOL_ERROR,
						"Protocol error detected."));
			} catch (Exception e) {
				logger.error("Unexpected exception tearing down session", e);
			}
		} catch (ParseException e) {
			logger
					.error(
							"INTERNAL Error while processing the request - hanging up ",
							e);
			throw new SipXbridgeException(
					"INTERNAL Error while processing the request ", e);
		}
	}

	/**
	 * Send an INVITE to SIPX proxy server.
	 * 
	 * @param requestEvent
	 *            -- The incoming RequestEvent ( from the ITSP side ) for which
	 *            we are generating the request outbound to the sipx proxy
	 *            server.
	 * 
	 * @param serverTransaction
	 *            -- The SIP Server transaction that we created to service this
	 *            request.
	 */

	void sendInviteToSipxProxy(RequestEvent requestEvent,
			ServerTransaction serverTransaction) {
		Request request = requestEvent.getRequest();

		logger.debug("sendInviteToSipXProxy "
				+ ((SIPMessage) request).getFirstLine());

		try {
			/*
			 * This is a request I got from the external provider. Route this
			 * into the network. The SipURI is the sipx proxy URI. He takes care
			 * of the rest.
			 */

			SipURI incomingRequestURI = (SipURI) request.getRequestURI();
			Dialog inboundDialog = serverTransaction.getDialog();
			ViaHeader inboundVia = ((ViaHeader) request
					.getHeader(ViaHeader.NAME));

			String host = inboundVia.getReceived() != null ? inboundVia
					.getReceived() : inboundVia.getHost();
			int port = inboundVia.getRPort() != -1 ? inboundVia.getRPort()
					: inboundVia.getPort();

			ItspAccountInfo itspAccountInfo = Gateway.getAccountManager()
					.getItspAccount(host, port);

			SipURI uri = null;
			if (!Gateway.isInboundCallsRoutedToAutoAttendant()) {
				uri = ProtocolObjects.addressFactory.createSipURI(
						incomingRequestURI.getUser(), Gateway
								.getSipxProxyDomain());
			} else {
				String destination = Gateway.getAutoAttendantName();
				if (destination.indexOf("@") == -1) {
					uri = ProtocolObjects.addressFactory.createSipURI(
							destination, Gateway.getSipxProxyDomain());
					if (Gateway.getBridgeConfiguration().getSipxProxyPort() != -1) {
						uri.setPort(Gateway.getBridgeConfiguration()
								.getSipxProxyPort());
					}
				} else {
					logger.warn("routing to domain other than proxy domain!");
					uri = (SipURI) ProtocolObjects.addressFactory
							.createURI("sip:" + destination);
				}
			}

			CallIdHeader callIdHeader = ProtocolObjects.headerFactory
					.createCallIdHeader(this.creatingCallId + "." + counter++);

			Gateway.getBackToBackUserAgentFactory().setBackToBackUserAgent(
					callIdHeader.getCallId(), this);

			CSeqHeader cseqHeader = ProtocolObjects.headerFactory
					.createCSeqHeader(1L, Request.INVITE);

			FromHeader fromHeader = (FromHeader) request.getHeader(
					FromHeader.NAME).clone();

			fromHeader.setParameter("tag", Long.toString(Math.abs(new Random()
					.nextLong())));

			/*
			 * Change the domain of the inbound request to that of the sipx
			 * proxy. Change the user part if routed to specific extension.
			 */
			String autoAttendantName = Gateway.getAutoAttendantName();
			ToHeader toHeader = null;
			if (autoAttendantName != null
					&& autoAttendantName.indexOf("@") != -1) {
				SipURI toUri = (SipURI) ProtocolObjects.addressFactory
						.createURI("sip:" + autoAttendantName);
				Address toAddress = ProtocolObjects.addressFactory
						.createAddress(toUri);
				toHeader = ProtocolObjects.headerFactory.createToHeader(
						toAddress, null);
			} else {
				toHeader = (ToHeader) request.getHeader(ToHeader.NAME).clone();
				((SipURI) toHeader.getAddress().getURI()).setHost(Gateway
						.getSipxProxyDomain());
				((SipURI) toHeader.getAddress().getURI()).removePort();
				if (Gateway.isInboundCallsRoutedToAutoAttendant()) {
					((SipURI) toHeader.getAddress().getURI()).setUser(Gateway
							.getAutoAttendantName());
				}
				toHeader.removeParameter("tag");
			}

			ViaHeader viaHeader = SipUtilities.createViaHeader(Gateway
					.getLanProvider(), Gateway.getSipxProxyTransport());
			viaHeader.setParameter(ORIGINATOR, Gateway.SIPXBRIDGE_USER);

			List<ViaHeader> viaList = new LinkedList<ViaHeader>();

			viaList.add(viaHeader);

			MaxForwardsHeader maxForwards = (MaxForwardsHeader) request
					.getHeader(MaxForwardsHeader.NAME);

			maxForwards.decrementMaxForwards();
			Request newRequest = ProtocolObjects.messageFactory.createRequest(
					uri, Request.INVITE, callIdHeader, cseqHeader, fromHeader,
					toHeader, viaList, maxForwards);
			ContactHeader contactHeader = SipUtilities.createContactHeader(
					incomingRequestURI.getUser(), Gateway.getLanProvider());
			newRequest.setHeader(contactHeader);
			/*
			 * The incoming session description.
			 */
			SessionDescription sessionDescription = SipUtilities
					.getSessionDescription(request);

			RtpSession incomingSession = this.createRtpSession(inboundDialog);

			incomingSession.getReceiver().setUseGlobalAddressing(
					itspAccountInfo == null
							|| itspAccountInfo.isGlobalAddressingUsed());

			RtpTransmitterEndpoint rtpEndpoint = new RtpTransmitterEndpoint(
					incomingSession, symmitronClient);
			incomingSession.setTransmitter(rtpEndpoint);
			KeepaliveMethod keepaliveMethod = itspAccountInfo != null ? itspAccountInfo
					.getRtpKeepaliveMethod()
					: KeepaliveMethod.NONE;

			rtpEndpoint.setKeepAliveMethod(keepaliveMethod);
			rtpEndpoint.setSessionDescription(sessionDescription, true);

			ContentTypeHeader cth = ProtocolObjects.headerFactory
					.createContentTypeHeader("application", "sdp");
			/*
			 * Create a new client transaction.
			 */
			ClientTransaction ct = Gateway.getLanProvider()
					.getNewClientTransaction(newRequest);

			Dialog outboundDialog = ct.getDialog();

			DialogContext.attach(this, outboundDialog, ct, ct.getRequest());
			/*
			 * Set the ITSP account info for the inbound INVITE to sipx proxy.
			 */
			DialogContext.get(outboundDialog).setItspInfo(itspAccountInfo);
			DialogContext.pairDialogs(inboundDialog, outboundDialog);

			/*
			 * Apply the Session Description from the INBOUND invite to the
			 * Receiver of the RTP session pointing towards the PBX side. This
			 * resets the ports in the session description.
			 */
			SessionDescription sd = SipUtilities.getSessionDescription(request);

			RtpSession outboundSession = this.createRtpSession(outboundDialog);

			outboundSession.getReceiver().setSessionDescription(sd);

			newRequest.setContent(outboundSession.getReceiver()
					.getSessionDescription().toString(), cth);

			SipUtilities.addLanAllowHeaders(newRequest);

			TransactionContext tad = new TransactionContext(ct,
					Operation.SEND_INVITE_TO_SIPX_PROXY);

			tad.setServerTransaction(serverTransaction);
			tad.setBackToBackUa(this);
			tad.setItspAccountInfo(itspAccountInfo);

			this.addDialog(ct.getDialog());
			this.referingDialog = ct.getDialog();

			this.referingDialogPeer = serverTransaction.getDialog();

			ct.sendRequest();

		} catch (InvalidArgumentException ex) {
			logger.error("Unexpected exception encountered");
			throw new SipXbridgeException("Unexpected exception encountered",
					ex);
		} catch (ParseException ex) {
			logger.error("Unexpected parse exception", ex);
			throw new SipXbridgeException("Unexpected parse exception", ex);
		} catch (SdpParseException ex) {
			try {
				Response response = ProtocolObjects.messageFactory
						.createResponse(Response.BAD_REQUEST, request);
				serverTransaction.sendResponse(response);
			} catch (Exception e) {
				logger.error("Unexpected exception", e);
			}
		} catch (Exception ex) {
			logger.error("Error while processing the request", ex);
			try {
				Response response = ProtocolObjects.messageFactory
						.createResponse(Response.SERVICE_UNAVAILABLE, request);
				serverTransaction.sendResponse(response);
			} catch (Exception e) {
				logger.error("Unexpected exception ", e);
			}
		}

	}

	/**
	 * Create a Client Tx pointing towards the park server.
	 * 
	 * @param - the session description to apply to the INVITE.
	 * 
	 * @return the dialog generated as a result of sending the invite to the MOH
	 *         server.
	 * 
	 */
	ClientTransaction createClientTxToMohServer(
			SessionDescription sessionDescription) {

		ClientTransaction retval = null;

		try {

			SipURI uri = Gateway.getMusicOnHoldUri();

			CallIdHeader callIdHeader = ProtocolObjects.headerFactory
					.createCallIdHeader(this.creatingCallId + "."
							+ this.counter++);

			Gateway.getBackToBackUserAgentFactory().setBackToBackUserAgent(
					callIdHeader.getCallId(), this);

			CSeqHeader cseqHeader = ProtocolObjects.headerFactory
					.createCSeqHeader(1L, Request.INVITE);

			Address gatewayAddress = Gateway.getGatewayFromAddress();

			FromHeader fromHeader = ProtocolObjects.headerFactory
					.createFromHeader(gatewayAddress, Long.toString(Math
							.abs(new Random().nextLong())));

			Address mohServerAddress = Gateway.getMusicOnHoldAddress();

			ToHeader toHeader = ProtocolObjects.headerFactory.createToHeader(
					mohServerAddress, null);

			toHeader.removeParameter("tag");

			ViaHeader viaHeader = SipUtilities.createViaHeader(Gateway
					.getLanProvider(), Gateway.getSipxProxyTransport());

			List<ViaHeader> viaList = new LinkedList<ViaHeader>();

			viaList.add(viaHeader);

			MaxForwardsHeader maxForwards = ProtocolObjects.headerFactory
					.createMaxForwardsHeader(20);

			maxForwards.decrementMaxForwards();
			Request newRequest = ProtocolObjects.messageFactory.createRequest(
					uri, Request.INVITE, callIdHeader, cseqHeader, fromHeader,
					toHeader, viaList, maxForwards);
			ContactHeader contactHeader = SipUtilities.createContactHeader(
					Gateway.SIPXBRIDGE_USER, Gateway.getLanProvider());
			newRequest.setHeader(contactHeader);

			/*
			 * Create a new client transaction.
			 */
			ClientTransaction ct = Gateway.getLanProvider()
					.getNewClientTransaction(newRequest);

			/*
			 * Set the duplexity of the INVITE to recvonly.
			 */
			SipUtilities.setDuplexity(sessionDescription, "recvonly");

			ContentTypeHeader cth = ProtocolObjects.headerFactory
					.createContentTypeHeader("application", "sdp");

			newRequest.setContent(sessionDescription.toString(), cth);

			TransactionContext tad = TransactionContext.attach(ct,
					Operation.SEND_INVITE_TO_MOH_SERVER);

			tad.setBackToBackUa(this);
			this.addDialog(ct.getDialog());
			DialogContext.attach(this, ct.getDialog(), ct, ct.getRequest());

			this.musicOnHoldDialog = ct.getDialog();
			this.musicOnHoldInviteTransaction = ct;

			retval = ct;

		} catch (InvalidArgumentException ex) {
			logger.error("Unexpected exception encountered");
			throw new SipXbridgeException("Unexpected exception encountered",
					ex);
		} catch (Exception ex) {
			logger.error("Unexpected parse exception", ex);
			if (retval != null) {
				this.dialogTable.remove(retval);
			}
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
					Request byeRequest = musicOnHoldDialog
							.createRequest(Request.BYE);

					ClientTransaction ctx = Gateway.getLanProvider()
							.getNewClientTransaction(byeRequest);
					TransactionContext.attach(ctx,
							Operation.SEND_BYE_TO_MOH_SERVER);
					musicOnHoldDialog.sendRequest(ctx);
				} else {
					DialogContext.get(this.musicOnHoldDialog)
							.setTerminateOnConfirm();

				}
			}
		} catch (SipException ex) {
			logger.error("Error sending BYE to the MOH dialog", ex);
		}

	}

	/**
	 * Send a request to an ITSP. This is the out of dialog request that sets up
	 * the call.
	 * 
	 * @param requestEvent
	 *            -- in bound request event ( sent by the sipx proxy server)
	 * @param serverTransaction
	 *            -- the server transaction.
	 * @param toDomain
	 *            -- domain to send it to.
	 * 
	 * @throws SipException
	 */

	void sendInviteToItsp(RequestEvent requestEvent,
			ServerTransaction serverTransaction, String toDomain)
			throws SipException {

		logger.debug("sendInviteToItsp: " + this);
		Request incomingRequest = serverTransaction.getRequest();
		Dialog incomingDialog = serverTransaction.getDialog();
		ItspAccountInfo itspAccountInfo = Gateway.getAccountManager()
				.getAccount(incomingRequest);
		if (itspAccountInfo == null) {
			SipUtilities.createResponse(serverTransaction, Response.NOT_FOUND);
			return;
		}

		SipProvider itspProvider = Gateway
				.getWanProvider(itspAccountInfo == null ? Gateway.DEFAULT_ITSP_TRANSPORT
						: itspAccountInfo.getOutboundTransport());

		if (Gateway.getCallLimit() != -1
				&& Gateway.getCallCount() >= Gateway.getCallLimit()) {
			try {
				serverTransaction.sendResponse(SipUtilities.createResponse(
						serverTransaction, Response.BUSY_HERE));
			} catch (Exception e) {
				String s = "Unepxected exception ";
				logger.fatal(s, e);
				throw new SipXbridgeException(s, e);
			}
		}

		boolean spiral = SipUtilities.isOriginatorSipXbridge(incomingRequest);

		ReplacesHeader replacesHeader = (ReplacesHeader) incomingRequest
				.getHeader(ReplacesHeader.NAME);

		if (logger.isDebugEnabled()) {
			logger.debug("sendInviteToItsp: spiral=" + spiral);
		}
		try {
			if (replacesHeader != null) {

				/* Fetch the Dialog object corresponding to the ReplacesHeader */
				Dialog replacedDialog = ((SipStackExt) ProtocolObjects.sipStack)
						.getReplacesDialog(replacesHeader);

				if (replacedDialog == null) {

					Response response = SipUtilities.createResponse(
							serverTransaction, Response.NOT_FOUND);
					response.setReasonPhrase("Replaced Dialog not found");
					serverTransaction.sendResponse(response);
					return;
				}

				if (spiral) {
					handleSpriralInviteWithReplaces(requestEvent,
							replacedDialog, serverTransaction, toDomain);
				} else {
					handleInviteWithReplaces(requestEvent, replacedDialog,
							serverTransaction);
				}

				return;
			}

			SipURI incomingRequestUri = (SipURI) incomingRequest
					.getRequestURI();

			this.mohDisabled = incomingRequestUri
					.getParameter("sipxbridge-moh") != null
					&& incomingRequestUri.getParameter("sipxbridge-moh")
							.equals("false");

			FromHeader fromHeader = (FromHeader) incomingRequest.getHeader(
					FromHeader.NAME).clone();

			Request outgoingRequest = SipUtilities.createInviteRequest(
					(SipURI) incomingRequestUri.clone(), itspProvider,
					itspAccountInfo, fromHeader, this.creatingCallId + "."
							+ this.counter++);

			/*
			 * Attach headers selectively to the outbound request. If privacy is
			 * requested, we suppress forwarding certain headers that can reveal
			 * information about the caller.
			 */
			for (String headerName : new String[] { ReplyToHeader.NAME,
					CallInfoHeader.NAME, SubjectHeader.NAME,
					OrganizationHeader.NAME, InReplyToHeader.NAME }) {
				Header header = incomingRequest.getHeader(headerName);
				if (header != null && !itspAccountInfo.stripPrivateHeaders()) {
					outgoingRequest.addHeader(header);
				}
			}
			SipUtilities.addWanAllowHeaders(outgoingRequest);
			ClientTransaction ct = itspProvider
					.getNewClientTransaction(outgoingRequest);
			Dialog outboundDialog = ct.getDialog();

			DialogContext.attach(this, outboundDialog, ct, outgoingRequest);
			DialogContext.get(outboundDialog).setItspInfo(itspAccountInfo);

			SessionDescription outboundSessionDescription = null;

			/*
			 * Set up to use global addressing on the outbound session if
			 * needed.
			 */
			boolean globalAddressing = itspAccountInfo == null
					|| itspAccountInfo.isGlobalAddressingUsed();
			if (spiral) {
				/*
				 * If this is a spiral, we re-use the RTP session from the
				 * refering dialog. This case occurs when we do a blind
				 * transfer.
				 */
				if (globalAddressing) {
					DialogContext.getRtpSession(this.referingDialog)
							.getReceiver().setGlobalAddress(
									this.symmitronClient.getPublicAddress());
				}
				DialogContext.getRtpSession(this.referingDialog).getReceiver()
						.setUseGlobalAddressing(globalAddressing);
				outboundSessionDescription = DialogContext.getRtpSession(
						this.referingDialog).getReceiver()
						.getSessionDescription();
			} else {
				RtpSession wanRtpSession = this
						.createRtpSession(outboundDialog);
				wanRtpSession.getReceiver().setUseGlobalAddressing(
						globalAddressing);
				outboundSessionDescription = SipUtilities
						.getSessionDescription(incomingRequest);
				wanRtpSession.getReceiver().setSessionDescription(
						outboundSessionDescription);
			}

			if (outboundSessionDescription == null) {
				Response response = SipUtilities.createResponse(
						serverTransaction, Response.NOT_ACCEPTABLE_HERE);
				serverTransaction.sendResponse(response);
				return;
			}

			/*
			 * Indicate that we will be transmitting first.
			 */

			ContentTypeHeader cth = ProtocolObjects.headerFactory
					.createContentTypeHeader("application", "sdp");

			outgoingRequest.setContent(outboundSessionDescription.toString(),
					cth);

			String outboundTransport = itspAccountInfo == null ? Gateway.DEFAULT_ITSP_TRANSPORT
					: itspAccountInfo.getOutboundTransport();
			ListeningPoint lp = itspProvider
					.getListeningPoint(outboundTransport);
			String sentBy = lp.getSentBy();
			if (itspAccountInfo == null
					|| itspAccountInfo.isGlobalAddressingUsed()) {
				lp.setSentBy(Gateway.getGlobalAddress() + ":" + lp.getPort());
			}
			lp.setSentBy(sentBy);

			/*
			 * If we spiraled back, then pair the refered dialog with the
			 * outgoing dialog. Otherwise pair the inbound and outboud dialogs.
			 */
			if (!spiral) {
				DialogContext.pairDialogs(incomingDialog, outboundDialog);
			} else {
				DialogContext.pairDialogs(this.referingDialogPeer,
						outboundDialog);
			}

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

			if (!spiral) {
				RtpSession incomingSession = this
						.createRtpSession(incomingDialog);
				KeepaliveMethod keepAliveMethod = itspAccountInfo
						.getRtpKeepaliveMethod();
				RtpTransmitterEndpoint rtpEndpoint = new RtpTransmitterEndpoint(
						incomingSession, symmitronClient);
				rtpEndpoint.setKeepAliveMethod(keepAliveMethod);
				incomingSession.setTransmitter(rtpEndpoint);
				rtpEndpoint.setSessionDescription(sessionDescription, true);
			} else if (spiral && replacesHeader == null) {

				/*
				 * This is a spiral. We are going to reuse the port in the
				 * incoming INVITE (we already own this port). Note here that we
				 * set the early media flag.
				 */

				if (this.rtpBridge.getState() == BridgeState.RUNNING) {
					this.rtpBridge.pause(); // Pause to block inbound packets.
				}

				RtpSession rtpSession = DialogContext
						.getRtpSession(this.referingDialog);
				if (rtpSession == null) {
					Response errorResponse = SipUtilities.createResponse(
							serverTransaction, Response.SESSION_NOT_ACCEPTABLE);
					errorResponse
							.setReasonPhrase("Could not RtpSession for refering dialog");
					serverTransaction.sendResponse(errorResponse);
					if (this.rtpBridge.getState() == BridgeState.PAUSED) {
						this.rtpBridge.resume();
					}

					return;
				}
				tad.setReferingDialog(referingDialog);
				tad
						.setReferRequest(DialogContext.get(referingDialog).referRequest);
				RtpTransmitterEndpoint rtpEndpoint = new RtpTransmitterEndpoint(
						rtpSession, symmitronClient);
				rtpSession.setTransmitter(rtpEndpoint);

				rtpEndpoint.setSessionDescription(sessionDescription, true);
				int keepaliveInterval = Gateway.getMediaKeepaliveMilisec();
				KeepaliveMethod keepaliveMethod = tad.getItspAccountInfo()
						.getRtpKeepaliveMethod();
				rtpEndpoint.setIpAddressAndPort(keepaliveInterval,
						keepaliveMethod);

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

			ct.setApplicationData(tad);
			this.addDialog(ct.getDialog());

			/*
			 * Record the call for this dialog.
			 */
			this.addDialog(ct.getDialog());

			logger.debug("Dialog table size = " + this.dialogTable.size());
			ct.sendRequest();

		} catch (SdpParseException ex) {
			try {
				serverTransaction.sendResponse(SipUtilities.createResponse(
						serverTransaction, Response.BAD_REQUEST));
			} catch (Exception e) {
				String s = "Unepxected exception ";
				logger.fatal(s, e);
				throw new SipXbridgeException(s, e);
			}
		} catch (ParseException ex) {
			String s = "Unepxected exception ";
			logger.fatal(s, ex);
			throw new SipXbridgeException(s, ex);
		} catch (IOException ex) {
			logger.error("Caught IO exception ", ex);
			for (Dialog dialog : this.dialogTable) {
				dialog.delete();
			}
		} catch (Exception ex) {
			logger.error("Error occurred during processing of request ", ex);
			try {
				Response response = SipUtilities.createResponse(
						serverTransaction, Response.SERVER_INTERNAL_ERROR);
				response.setReasonPhrase("Unexpected Exception occured at "
						+ ex.getStackTrace()[1].getFileName() + ":"
						+ ex.getStackTrace()[1].getLineNumber());
				serverTransaction.sendResponse(response);
			} catch (Exception e) {
				logger.error("Unexpected exception ", e);
			}
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
	void handleInviteWithReplaces(RequestEvent requestEvent,
			Dialog replacedDialog, ServerTransaction serverTransaction)
			throws Exception {
		Request request = requestEvent.getRequest();
		SessionDescription newOffer = SipUtilities
				.getSessionDescription(request);
		logger.debug("handleInviteWithReplaces: replacedDialog = "
				+ replacedDialog);
		String address = ((ViaHeader) request.getHeader(ViaHeader.NAME))
				.getHost();
		DialogContext inviteDat = DialogContext.get(serverTransaction
				.getDialog());
		Dialog dialog = serverTransaction.getDialog();

		try {
			RtpSession rtpSession = this.createRtpSession(replacedDialog);
			String ipAddress = SipUtilities
					.getSessionDescriptionMediaIpAddress(newOffer);
			int port = SipUtilities.getSessionDescriptionMediaPort(newOffer);
			if (rtpSession.getTransmitter() != null) {
				/*
				 * Already has a transmitter then simply redirect the
				 * transmitter to the new location.
				 */
				rtpSession.getTransmitter()
						.setIpAddressAndPort(ipAddress, port);
			} else {
				RtpTransmitterEndpoint transmitter = new RtpTransmitterEndpoint(
						rtpSession, Gateway.getSymmitronClient(address));
				transmitter.setIpAddressAndPort(ipAddress, port);
				rtpSession.setTransmitter(transmitter);
			}

			if (this.musicOnHoldDialog != null) {
				this.sendByeToMohServer();
				rtpSession.getTransmitter().setOnHold(false);
			}

			logger.debug("replacedDialog.getState() : "
					+ replacedDialog.getState());

			if (replacedDialog.getState() == DialogState.CONFIRMED) {
				DialogContext replacedDialogApplicationData = DialogContext
						.get(replacedDialog);

				Dialog peerDialog = replacedDialogApplicationData
						.getPeerDialog();
				DialogContext peerDat = DialogContext.get(peerDialog);

				Request reInvite = peerDialog.createRequest(Request.INVITE);
				SipUtilities.addWanAllowHeaders(reInvite);

				RtpSession wanRtpSession = peerDat.getRtpSession();
				wanRtpSession.getReceiver().setSessionDescription(newOffer);
				SipUtilities.incrementSessionVersion(newOffer);
				ContentTypeHeader contentTypeHeader = ProtocolObjects.headerFactory
						.createContentTypeHeader("application", "sdp");
				reInvite.setContent(newOffer.toString(), contentTypeHeader);

				SipProvider provider = ((DialogExt) peerDialog)
						.getSipProvider();
				ClientTransaction ctx = provider
						.getNewClientTransaction(reInvite);
				TransactionContext tad = TransactionContext.attach(ctx,
						Operation.HANDLE_INVITE_WITH_REPLACES);
				tad.setServerTransaction(serverTransaction);
				tad.setReplacedDialog(replacedDialog);

				// send the in-dialog re-invite to the other side.
				DialogContext.get(peerDialog).sendReInvite(ctx);
			} else {
				/*
				 * The following condition happens during call pickup. The other
				 * side has not been sent an SDP answer yet. Extract the OFFER
				 * from the in-bound INVITE and send it as an answer. We have to
				 * follow these restrictions: For each "m=" line in the offer,
				 * there MUST be a corresponding "m=" line in the answer. The
				 * answer MUST contain exactly the same number of "m=" lines as
				 * the offer. This allows for streams to be matched up based on
				 * their order. This implies that if the offer contained zero
				 * "m=" lines, the answer MUST contain zero "m=" lines.
				 * 
				 * The "t=" line in the answer MUST equal that of the offer. The
				 * time of the session cannot be negotiated. An offered stream
				 * MAY be rejected in the answer, for any reason. If a stream is
				 * rejected, the offerer and answerer MUST NOT generate media
				 * (or RTCP packets) for that stream. To reject an offered
				 * stream, the port number in the corresponding stream in the
				 * answer MUST be set to zero. Any media formats listed are
				 * ignored. At least one MUST be present, as specified by SDP.
				 */

				SessionDescription sdes = rtpSession.getReceiver()
						.getSessionDescription();

				/*
				 * Inbound INVITE (for call pickup).
				 */
				HashSet<Integer> codecs = SipUtilities.getCommonCodec(sdes,
						newOffer);

				logger.debug("Codecs = " + codecs);

				if (replacedDialog.getState() != DialogState.CONFIRMED) {
					if (codecs.size() == 0) {
						Response errorResponse = SipUtilities
								.createResponse(serverTransaction,
										Response.NOT_ACCEPTABLE_HERE);
						serverTransaction.sendResponse(errorResponse);
						return;

					}

					SipUtilities.cleanSessionDescription(sdes, codecs);

				}

				/*
				 * Track the RTP session we were using.
				 */
				inviteDat.rtpSession = rtpSession;

				/*
				 * Generate an OK response to be sent after BYE OK comes in from
				 * transfer agent.
				 */
				Response okResponse = SipUtilities.createResponse(
						serverTransaction, Response.OK);

				ContentTypeHeader cth = ProtocolObjects.headerFactory
						.createContentTypeHeader("application", "sdp");
				SipProvider txProvider = ((TransactionExt) serverTransaction)
						.getSipProvider();
				ContactHeader contactHeader = SipUtilities.createContactHeader(
						Gateway.SIPXBRIDGE_USER, txProvider);
				okResponse.setHeader(contactHeader);
				okResponse.setContent(rtpSession.getReceiver()
						.getSessionDescription().toString(), cth);
				DialogContext replacedDat = DialogContext.get(replacedDialog);
				if (replacedDat.transaction != null
						&& replacedDat.transaction instanceof ClientTransaction) {

					ClientTransaction ctx = (ClientTransaction) replacedDat.transaction;
					TransactionContext.attach(ctx,
							Operation.CANCEL_REPLACED_INVITE);
					Request cancelRequest = ctx.createCancel();
					if (replacedDat.transaction.getState() != TransactionState.TERMINATED) {
						ClientTransaction cancelTx = txProvider
								.getNewClientTransaction(cancelRequest);

						cancelTx.sendRequest();
					}
				}

				Request byeRequest = replacedDialog.createRequest(Request.BYE);
				SipProvider provider = ((DialogExt) replacedDialog)
						.getSipProvider();
				ClientTransaction byeCtx = provider
						.getNewClientTransaction(byeRequest);

				/*
				 * Create a little transaction context to identify the operator
				 * in our state machine. when we see a response to the BYE.
				 */
				TransactionContext.attach(byeCtx,
						Operation.SEND_BYE_TO_REPLACED_DIALOG);

				/*
				 * bid adeu to the replaced dialog.
				 */
				replacedDialog.sendRequest(byeCtx);

				serverTransaction.sendResponse(okResponse);

				DialogContext replacedDialogApplicationData = DialogContext
						.get(dialog);

				Dialog peerDialog = replacedDialogApplicationData
						.getPeerDialog();
				DialogContext peerDat = DialogContext.get(peerDialog);
				RtpSession wanRtpSession = peerDat.getRtpSession();
				SipProvider wanProvider = ((DialogExt) peerDialog)
						.getSipProvider();
				ContactHeader contact = SipUtilities.createContactHeader(
						wanProvider, peerDat.getItspInfo());
				SessionDescription answerSdes = SipUtilities
						.cleanSessionDescription(newOffer, codecs);
				wanRtpSession.getReceiver().setSessionDescription(answerSdes);

				ServerTransaction peerSt = ((ServerTransaction) peerDat.transaction);
				Response peerOk = SipUtilities.createResponse(peerSt,
						Response.OK);
				peerOk.setHeader(contact);
				peerOk.setContent(answerSdes, cth);
				peerSt.sendResponse(peerOk);
				this.getRtpBridge().start();

			}
		} catch (Exception ex) {
			logger.error("Unexpected exception -- tearing down call ", ex);
			try {
				this.tearDown(ProtocolObjects.headerFactory.createReasonHeader(
						Gateway.SIPXBRIDGE_USER,
						ReasonCode.SIPXBRIDGE_INTERNAL_ERROR, ex.getMessage()));
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
		ServerTransaction st = requestEvent.getServerTransaction();

		DialogContext dialogContext = (DialogContext) dialog
				.getApplicationData();
		Dialog peer = dialogContext.getPeerDialog();

		if (dialogContext.isForwardByeToPeer() && peer != null
				&& peer.getState() != DialogState.TERMINATED
				&& peer.getState() != null) {
			SipProvider provider = ((gov.nist.javax.sip.DialogExt) peer)
					.getSipProvider();

			Request bye = peer.createRequest(Request.BYE);

			ClientTransaction ct = provider.getNewClientTransaction(bye);

			TransactionContext transactionContext = TransactionContext.attach(
					st, Operation.PROCESS_BYE);

			transactionContext.setClientTransaction(ct);
			transactionContext.setItspAccountInfo(DialogContext.get(peer)
					.getItspInfo());
			ct.setApplicationData(transactionContext);

			peer.sendRequest(ct);
		} else {
			/*
			 * Peer dialog is not yet established or is terminated.
			 */
			logger.debug("BackToBackUserAgent: peerDialog = " + peer);
			if (peer != null) {
				logger.debug("BackToBackUserAgent: peerDialog state = "
						+ peer.getState());
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
		ReasonHeader rh = ProtocolObjects.headerFactory.createReasonHeader(
				agent, code, reason);
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
		 * Prevent a concurrent modification exception. Copy all the dialogs
		 * into a temp set.
		 */
		HashSet<Dialog> temp = new HashSet<Dialog>();
		temp.addAll(this.dialogTable);

		for (Dialog dialog : temp) {
			if (dialog.getState() != DialogState.TERMINATED) {
				DialogContext dialogCtx = DialogContext.get(dialog);
				SipProvider lanProvider = ((DialogExt) dialog).getSipProvider();
				if (dialogCtx.getTransaction() != null
						&& dialogCtx.getTransaction().getState() != TransactionState.TERMINATED
						&& dialogCtx.getTransaction() instanceof ClientTransaction) {
					ClientTransaction ctx = (ClientTransaction) dialogCtx
							.getTransaction();
					if (ctx.getState() == TransactionState.PROCEEDING) {
						ClientTransaction cancelTx = lanProvider
								.getNewClientTransaction(ctx.createCancel());
						TransactionContext.attach(cancelTx,
								Operation.CANCEL_INVITE);
						cancelTx.sendRequest();
					}
				}

				/*
				 * Cannot send BYE to a Dialog in EARLY state.
				 */
				if (dialog.getState() != null
						&& dialog.getState() != DialogState.EARLY) {
					Request byeRequest = dialog.createRequest(Request.BYE);
					if (reason != null) {
						byeRequest.addHeader(reason);
					}
					SipProvider provider = ((DialogExt) dialog)
							.getSipProvider();
					ClientTransaction ct = provider
							.getNewClientTransaction(byeRequest);
					dialog.sendRequest(ct);
				} else {
					/* kill the dialog if in early state or not established */
					dialog.delete();
				}

			}
		}

		/* Clean up the MOH dialog */
		if (this.musicOnHoldDialog != null) {
			if (this.musicOnHoldDialog.getState() != DialogState.TERMINATED
					&& this.musicOnHoldDialog.getState() != DialogState.EARLY) {
				this.sendByeToMohServer();
			} else if (this.musicOnHoldDialog.getState() == null
					|| this.musicOnHoldDialog.getState() == DialogState.EARLY) {
				this.musicOnHoldDialog.delete();
				if (this.musicOnHoldInviteTransaction != null
						&& this.musicOnHoldInviteTransaction.getState() != TransactionState.TERMINATED) {
					this.musicOnHoldInviteTransaction.terminate();
				}
			}

		}

	}

	/**
	 * @return the musicOnHoldDialog
	 */
	public Dialog getMusicOnHoldDialog() {
		return musicOnHoldDialog;
	}

	public boolean isMohDisabled() {
		return this.mohDisabled;
	}

}
