/*
 *  Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 *  Contributors retain copyright to elements licensed under a Contributor Agreement.
 *  Licensed to the User under the LGPL license.
 *
 */

package org.sipfoundry.sipxbridge;

import gov.nist.javax.sip.DialogExt;
import gov.nist.javax.sip.header.HeaderFactoryExt;
import gov.nist.javax.sip.header.extensions.MinSE;
import gov.nist.javax.sip.header.extensions.SessionExpiresHeader;
import gov.nist.javax.sip.message.SIPResponse;

import java.util.ListIterator;
import java.util.TimerTask;
import java.util.concurrent.atomic.AtomicBoolean;

import javax.sdp.SessionDescription;
import javax.sip.ClientTransaction;
import javax.sip.Dialog;
import javax.sip.DialogState;
import javax.sip.ServerTransaction;
import javax.sip.SipException;
import javax.sip.SipProvider;
import javax.sip.Transaction;
import javax.sip.header.AcceptHeader;
import javax.sip.header.AllowHeader;
import javax.sip.header.ContactHeader;
import javax.sip.header.SubjectHeader;
import javax.sip.header.SupportedHeader;
import javax.sip.header.ViaHeader;
import javax.sip.message.Request;
import javax.sip.message.Response;

import org.apache.log4j.Logger;

/**
 * Store information that is specific to a Dialog. This is a temporary holding
 * place for dialog specific data that is specific to the lifetime of a SIP
 * Dialog. There is one of these structures per dialog.
 * 
 * @author M. Ranganathan
 * 
 */
class DialogContext {

	private static Logger logger = Logger.getLogger(DialogContext.class);

	private PendingDialogAction pendingAction = PendingDialogAction.NONE;

	/*
	 * Dialog associated with this application data.
	 */
	private Dialog dialog;
	/*
	 * The Peer Dialog of this Dialog.
	 */
	private Dialog peerDialog;

	/*
	 * The request that originated the Dialog
	 */
	Request request;

	/*
	 * The transaction that created this DialogApplicationData
	 */

	Transaction transaction;

	/*
	 * The current re-invite transaction
	 */
	ClientTransaction reInviteTransaction;

	/*
	 * The last ACK.
	 */
	Request lastAck;

	/*
	 * The last response seen by the dialog.
	 */
	private Response lastResponse;

	/*
	 * The B2BUA associated with the dialog. The BackToBackUserAgent structure
	 * tracks call state.
	 */
	private BackToBackUserAgent backToBackUserAgent;

	/*
	 * Account information for the associated dialog. There can be several ITSPs
	 * involved in a single call ( each call leg can have its own ITSP).
	 */
	private ItspAccountInfo itspInfo;

	/*
	 * A flag that indicates whether this Dialog was created by sipxbridge and
	 * is managed by sipXbridge.
	 */
	boolean isOriginatedBySipxbridge;

	// /////////////////////////////////////////////////////////////
	// Auxilliary data structures associated with dialog state machine.
	// ////////////////////////////////////////////////////////////
	/*
	 * Rtp session associated with this call leg.
	 */

	RtpSession rtpSession;

	/*
	 * Session timer associated with this call leg.
	 */
	private SessionTimerTask sessionTimer;

	// //////////////////////////////////////////////////////////////
	// The following are state variables associated with the Dialog State
	// machine.
	// ///////////////////////////////////////////////////////////////

	/*
	 * Used by the session timer - to compute whether or not to send a session
	 * timer re-INVITE.
	 */
	long timeLastAckSent;

	/*
	 * Session timer interval ( seconds ).
	 */
	int sessionExpires;

	/*
	 * Records whether or not an ACCEPTED has been sent for the REFER. This
	 * dictates what we need to do when we see a BYE for this dialog.
	 */
	private boolean forwardByeToPeer = true;

	/*
	 * The generated REFER request.
	 */
	Request referRequest;
	
	/*
	 * If this flag is set to true then the call dialog is torn down immediately after it is CONFIRMed.
	 */

	private boolean terminateOnConfirm;
	
	/*
	 * A private flag that is used to prevent re-entrant re-INVITEs ( some ITSPs do not react
	 * in accordance with the RFC when such a re-INVITE is seen ).
	 */

	private AtomicBoolean waitingToSendReInvite = new AtomicBoolean(false);

	// /////////////////////////////////////////////////////////////////
	// Inner classes.
	// ////////////////////////////////////////////////////////////////

	/**
	 * The session timer task -- sends Re-INVITE to the associated dialog at
	 * specific intervals.
	 */
	class SessionTimerTask extends TimerTask {

		String method;

		public SessionTimerTask(String method) {
			this.method = method;

		}

		@Override
		public void run() {
			if (dialog.getState() == DialogState.TERMINATED) {
				this.cancel();
			}

			try {
				Request request;
				long currentTimeMilis = System.currentTimeMillis();
				if (dialog.getState() == DialogState.CONFIRMED
						&& DialogContext.get(dialog).getPeerDialog() != null) {
					if (method.equalsIgnoreCase(Request.INVITE)) {

						if (currentTimeMilis < timeLastAckSent - sessionExpires
								* 1000) {
							return;
						}

						SipProvider provider = ((DialogExt) dialog)
								.getSipProvider();
						RtpSession rtpSession = getRtpSession();
						if (rtpSession == null
								|| rtpSession.getReceiver() == null) {
							return;
						}
						SessionDescription sd = rtpSession.getReceiver()
								.getSessionDescription();

						request = dialog.createRequest(Request.INVITE);
						request.removeHeader(AllowHeader.NAME);
						SipUtilities.addWanAllowHeaders(request);
						AcceptHeader accept = ProtocolObjects.headerFactory
								.createAcceptHeader("application", "sdp");
						request.setHeader(accept);
						request.removeHeader(SupportedHeader.NAME);

						request.setContent(sd.toString(),
								ProtocolObjects.headerFactory
										.createContentTypeHeader("application",
												"sdp"));
						ContactHeader cth = SipUtilities.createContactHeader(
								provider, getItspInfo());
						request.setHeader(cth);
						SessionExpiresHeader sexp = ((HeaderFactoryExt) ProtocolObjects.headerFactory)
								.createSessionExpiresHeader(Gateway
										.getSessionExpires());
						request.setHeader(sexp);
						MinSE minSe = new MinSE();
						minSe.setExpires(Gateway.getSessionExpires());
						request.setHeader(minSe);
						if (getItspInfo() != null
								&& !getItspInfo().stripPrivateHeaders()) {
							SubjectHeader sh = ProtocolObjects.headerFactory
									.createSubjectHeader("SipxBridge Session Timer");
							request.setHeader(sh);
						} else {
							SipUtilities.stripPrivateHeaders(request);

						}

						if (getItspInfo() == null
								|| getItspInfo().isGlobalAddressingUsed()) {
							SipUtilities.setGlobalAddresses(request);
						}

					} else {
						/*
						 * This is never used but keep it here for now.
						 */
						request = dialog.createRequest(Request.OPTIONS);
					}

					DialogExt dialogExt = (DialogExt) dialog;
					ClientTransaction ctx = dialogExt.getSipProvider()
							.getNewClientTransaction(request);
					TransactionContext.attach(ctx, Operation.SESSION_TIMER);

					new ReInviteSender(DialogContext.get(dialog), ctx).run();
					DialogContext.this.sessionTimer = new SessionTimerTask(
							this.method);

					int expiryTime = sessionExpires < Gateway.MIN_EXPIRES ? Gateway.MIN_EXPIRES
							: sessionExpires;

					Gateway.getTimer().schedule(sessionTimer,
							expiryTime - Gateway.TIMER_ADVANCE * 1000);

				}

			} catch (Exception ex) {
				logger
						.error(
								"Unexpected exception sending Session Timer INVITE",
								ex);
				this.cancel();

			}
		}
	}

	/**
	 * This task waits till a pending ack has been recorded and then sends out a
	 * re-INVITE. This is to prevent interleaving INVITEs ( which will result in
	 * a 493 from the ITSP ).
	 * 
	 */
	public class ReInviteSender implements Runnable {
		DialogContext dialogContext;
		ClientTransaction ctx;

		public ReInviteSender(DialogContext dialogContext, ClientTransaction ctx) {
			this.dialogContext = dialogContext;
			this.ctx = ctx;
			dialogContext.waitingToSendReInvite.set(true);
		}

		public void run() {
			try {
				int i = 0;

				while (dialogContext.dialog.getState() != DialogState.TERMINATED
						&& dialogContext.isWaitingForAck(ctx)) {
					if (i++ > 80) {
						/*
						 * Could not send re-INVITE we should kill the call.
						 */
						logger
								.error("Could not send re-INVITE -- killing call");
						ctx.terminate();
						backToBackUserAgent.tearDown("sipxbridge",
								ReasonCode.TIMED_OUT_WAITING_TO_SEND_REINVITE,
								"Timed out waiting to re-INVITE");
						return;
					} else {
						logger.debug("Waiting for ACK");
						Thread.sleep(100);
					}
				}

				/*
				 * Wait for the ACK to actually get to the other side. Wait for
				 * any ACK retransmissions to finish. Then send out the request.
				 */

				Thread.sleep(500);

				logger.debug("Sending re-INVITE : Transaction operation = "
						+ TransactionContext.get(ctx).getOperation());

				if (dialogContext.dialog.getState() != DialogState.TERMINATED) {
					dialogContext.reInviteTransaction = ctx;
					dialogContext.dialog.sendRequest(ctx);
				}
				logger.debug("re-INVITE successfully sent");
			} catch (Exception ex) {
				logger.error("Error sending INVITE", ex);
			} finally {
				dialogContext.waitingToSendReInvite.set(false);
			}
		}
	}

	/**
	 * Delays sending the INVITE to the park server. If a RE-INVITE is sent to
	 * the ITSP in that interval, the INVITE to the park server is not sent.
	 * Instead, we ACK the incoming INVITE so that the INVITE waiting for the
	 * ACK can proceed.
	 * 
	 */
	class MohTimer extends TimerTask {

		private ClientTransaction mohCtx;

		public MohTimer(ClientTransaction mohCtx) {
			this.mohCtx = mohCtx;
		}

		public void run() {
			try {

				/*
				 * Check the state of the peer dialog. If the phone has
				 * answered, waitingToSendReInvite will be true. If so, just
				 * return an ACK. This will allow the process to continue.
				 */
				if (waitingToSendReInvite.get()) {
					/*
					 * The ITSP is waiting for an ACK at this point -- so let
					 * him have the ACK but fool him by sending a filtered
					 * version of the SDP.
					 */
					if (getLastResponse() != null
							&& getLastResponse().getContent() != null) {
						// If the response has not been consumed, send it off.
						SessionDescription sessionDescription = SipUtilities
								.getSessionDescription(getLastResponse());
						SipUtilities.cleanSessionDescription(
								sessionDescription, Gateway
										.getParkServerCodecs());
						getRtpSession().getReceiver().setSessionDescription(
								sessionDescription);
						sendAck(sessionDescription);
					}
					mohCtx.terminate();
				} else {
					if (!DialogContext.get(mohCtx.getDialog()).terminateOnConfirm) {
						TransactionContext.get(mohCtx)
								.setDialogPendingSdpAnswer(dialog);
						DialogContext.get(mohCtx.getDialog()).setPendingAction(
								PendingDialogAction.PENDING_SDP_ANSWER_IN_ACK);
						DialogContext.get(mohCtx.getDialog()).setPeerDialog(dialog);
						mohCtx.sendRequest();
					} else {
						mohCtx.terminate();
					}
				}
			} catch (Exception ex) {
				logger.error("Error sending moh request", ex);
			}

		}
	}

	/*
	 * Constructor.
	 */
	private DialogContext(Dialog dialog) {
		this.sessionExpires = Gateway.getSessionExpires();
		this.dialog = dialog;
		// Kick off a task to test for session liveness.
		SipProvider provider = ((DialogExt) dialog).getSipProvider();
		if (Gateway.getSessionTimerMethod() != null
				&& provider != Gateway.getLanProvider()) {
			this.sessionTimer = new SessionTimerTask(Gateway
					.getSessionTimerMethod());
			Gateway.getTimer().schedule(
					this.sessionTimer,
					Gateway.SESSION_EXPIRES * 1000 - Gateway.TIMER_ADVANCE
							* 1000);
		}

	}

	/**
	 * Avoid interleaving of INVITE transactions for a given Dialog (some ITSPs
	 * return unreliable error codes when transactions are interleaved).
	 */
	private boolean isWaitingForAck(ClientTransaction ctx) {
		long seqno = SipUtilities.getSeqNumber(ctx.getRequest());
		if (this.reInviteTransaction == null) {
			return false;
		} else if (this.lastAck == null
				|| seqno != SipUtilities.getSeqNumber(this.lastAck) + 1) {
			return true;
		} else {
			return false;
		}

	}

	/**
	 * Create a dialog to dialog association.
	 * 
	 * @param dialog1
	 *            - first dialog.
	 * @param dialog2
	 *            - second dialog.
	 * 
	 */
	static void pairDialogs(Dialog dialog1, Dialog dialog2) {
		logger.debug("pairDialogs dialogs = " + dialog1 + " " + dialog2);

		DialogContext dad1 = DialogContext.get(dialog1);
		DialogContext dad2 = DialogContext.get(dialog2);
		dad1.setPeerDialog(dialog2);
		dad2.setPeerDialog(dialog1);
	}

	static BackToBackUserAgent getBackToBackUserAgent(Dialog dialog) {
		if (dialog == null) {
			logger.debug("null dialog -- returning null ");
			return null;
		} else if (dialog.getApplicationData() == null) {
			logger.debug("null dialog application data -- returning null");
			return null;
		} else {
			return ((DialogContext) dialog.getApplicationData())
					.getBackToBackUserAgent();
		}
	}

	/**
	 * Conveniance methods
	 */
	static Dialog getPeerDialog(Dialog dialog) {
		return ((DialogContext) dialog.getApplicationData()).peerDialog;
	}

	public static RtpSession getPeerRtpSession(Dialog dialog) {
		return get(getPeerDialog(dialog)).rtpSession;

	}

	static RtpSession getRtpSession(Dialog dialog) {
		logger.debug("DialogApplicationData.getRtpSession " + dialog);

		return ((DialogContext) dialog.getApplicationData()).rtpSession;
	}

	static DialogContext attach(BackToBackUserAgent backToBackUserAgent,
			Dialog dialog, Transaction transaction, Request request) {
		if (backToBackUserAgent == null)
			throw new NullPointerException("Null back2back ua");
		if (dialog.getApplicationData() != null) {
			throw new SipXbridgeException(
					"DialogContext: Context Already set!!");
		}
		DialogContext dat = new DialogContext(dialog);
		dat.transaction = transaction;
		if (transaction instanceof ClientTransaction) {
			dat.reInviteTransaction = (ClientTransaction) transaction;
		}
		dat.request = request;
		dat.setBackToBackUserAgent(backToBackUserAgent);
		dialog.setApplicationData(dat);

		return dat;
	}

	public Transaction getTransaction() {
		return transaction;
	}

	static DialogContext get(Dialog dialog) {
		return (DialogContext) dialog.getApplicationData();
	}

	/**
	 * @param rtpSession
	 *            the rtpSession to set
	 */
	void setRtpSession(RtpSession rtpSession) {
		this.rtpSession = rtpSession;
	}

	/**
	 * @return the rtpSession
	 */
	RtpSession getRtpSession() {
		return rtpSession;
	}

	void recordLastAckTime() {
		this.timeLastAckSent = System.currentTimeMillis();
	}

	/**
	 * @param backToBackUserAgent
	 *            the backToBackUserAgent to set
	 */
	void setBackToBackUserAgent(BackToBackUserAgent backToBackUserAgent) {
		this.backToBackUserAgent = backToBackUserAgent;
		// set the back pointer to our set of active dialogs.

		this.backToBackUserAgent.addDialog(this.dialog);
	}

	/**
	 * @return the backToBackUserAgent
	 */
	BackToBackUserAgent getBackToBackUserAgent() {
		return backToBackUserAgent;
	}

	/**
	 * @param itspInfo
	 *            the itspInfo to set
	 */
	void setItspInfo(ItspAccountInfo itspInfo) {
		if (this.itspInfo != null) {
			logger.warn("Re-Setting ITSP info to null!!");
		}
		this.itspInfo = itspInfo;
	}

	/**
	 * @return the itspInfo
	 */
	ItspAccountInfo getItspInfo() {
		return itspInfo;
	}

	/**
	 * Cancel the session timer.
	 */
	void cancelSessionTimer() {
		if (this.sessionTimer != null) {
			this.sessionTimer.cancel();
		}
	}

	void setSetExpires(int expires) {
		this.sessionExpires = expires;

	}

	/**
	 * Send ACK to the encapsulated dialog.
	 * 
	 * @throws Exception
	 */
	void sendAck(SessionDescription sessionDescription) throws Exception {
		if (this.getLastResponse() == null) {
			Gateway
					.logInternalError("Method was called with lastResponse null");

		}
		Request ackRequest = dialog.createAck(SipUtilities.getSeqNumber(this
				.getLastResponse()));
		this.setLastResponse(null);
		this.recordLastAckTime();
		SipUtilities.setSessionDescription(ackRequest, sessionDescription);
		/*
		 * Compensate for the quirks of some ITSPs which will play MOH.
		 */
		SipUtilities.setDuplexity(sessionDescription, "sendrecv");
		this.sendAck(ackRequest);
		setPendingAction(PendingDialogAction.NONE);
	}

	/**
	 * Check to see if the ITSP allows a REFER request.
	 * 
	 * @return true if REFER is allowed.
	 * 
	 * 
	 */
	@SuppressWarnings("unchecked")
	boolean isReferAllowed() {
		if (this.transaction instanceof ServerTransaction) {
			if (this.request == null) {
				return false;
			}
			ListIterator li = request.getHeaders(AllowHeader.NAME);

			while (li != null && li.hasNext()) {
				AllowHeader ah = (AllowHeader) li.next();
				if (ah.getMethod().equals(Request.REFER)) {
					return true;
				}
			}
			return false;

		} else {
			if (this.getLastResponse() == null) {
				return false;
			}

			ListIterator li = getLastResponse().getHeaders(AllowHeader.NAME);

			while (li != null && li.hasNext()) {
				AllowHeader ah = (AllowHeader) li.next();
				if (ah.getMethod().equals(Request.REFER)) {
					return true;
				}
			}
			return false;
		}
	}

	/**
	 * Send an INVITE with no SDP to the peer dialog. This solicits an SDP offer
	 * from the peer of the given dialog.
	 * 
	 * @param requestEvent
	 *            -- the request event for which we have to solicit the offer.
	 * @param continuationData
	 *            -- context information so we can process the continuation.
	 * 
	 * @return true if the offer is sent successfully. false if there is already
	 *         an offer in progress and hence we should not send an offer.
	 */
	boolean solicitSdpOfferFromPeerDialog(ContinuationData continuationData)
			throws Exception {
		try {

			Dialog peerDialog = DialogContext.getPeerDialog(dialog);
			/*
			 * There is already a re-negotiation in progress so return silently
			 */

			if (peerDialog != null
					&& peerDialog.getState() != DialogState.TERMINATED) {
				logger.debug("queryDialogFromPeer -- sending query to "
						+ peerDialog + " continuationOperation = "
						+ continuationData.getOperation());

				Request reInvite = peerDialog.createRequest(Request.INVITE);
				reInvite.removeHeader(SupportedHeader.NAME);
				SipUtilities.addWanAllowHeaders(reInvite);
				SipProvider provider = ((DialogExt) peerDialog)
						.getSipProvider();
				ItspAccountInfo peerAccountInfo = DialogContext
						.getPeerDialogContext(dialog).getItspInfo();
				ViaHeader viaHeader = SipUtilities.createViaHeader(provider,
						peerAccountInfo);
				reInvite.setHeader(viaHeader);
				ContactHeader contactHeader = SipUtilities.createContactHeader(
						provider, peerAccountInfo);

				reInvite.setHeader(contactHeader);
				AcceptHeader acceptHeader = ProtocolObjects.headerFactory
						.createAcceptHeader("application", "sdp");
				reInvite.setHeader(acceptHeader);
				ClientTransaction ctx = provider
						.getNewClientTransaction(reInvite);
				TransactionContext tad = TransactionContext.attach(ctx,
						Operation.SOLICIT_SDP_OFFER_FROM_PEER_DIALOG);
				/*
				 * Mark what we should do when we see the 200 OK response. This
				 * is what this dialog expects to see. Mark this as the pending
				 * operation for this dialog.
				 */
				DialogContext.get(peerDialog).setPendingAction(
						PendingDialogAction.PENDING_SDP_ANSWER_IN_ACK);

				/*
				 * The information we need to continue the operation when the
				 * Response comes in.
				 */
				tad.setContinuationData(continuationData);

				/*
				 * Send the Re-INVITE and try to avoid the Glare Race condition.
				 */
				new Thread(new ReInviteSender(DialogContext.get(peerDialog),
						ctx)).start();

			}
			return true;
		} catch (Exception ex) {
			logger.error("Exception occured. tearing down call! ", ex);
			this.backToBackUserAgent.tearDown();
			return true;
		}

	}

	public static RtpSession getPeerTransmitter(Dialog dialog) {
		return DialogContext.get(DialogContext.getPeerDialog(dialog)).rtpSession;
	}

	void setPendingAction(PendingDialogAction pendingAction) {
		/*
		 * A dialog can have only a single outstanding action.
		 */
		if (this.pendingAction != PendingDialogAction.NONE
				&& pendingAction != PendingDialogAction.NONE
				&& this.pendingAction != pendingAction) {
			logger.error("Replacing pending action " + this.pendingAction
					+ " with " + pendingAction);
			throw new SipXbridgeException("Pending dialog action is "
					+ this.pendingAction);
		}
		this.pendingAction = pendingAction;
	}

	PendingDialogAction getPendingAction() {
		return pendingAction;
	}

	public static PendingDialogAction getPendingAction(Dialog dialog) {
		return DialogContext.get(dialog).pendingAction;
	}

	public static DialogContext getPeerDialogContext(Dialog dialog) {
		return DialogContext.get(DialogContext.getPeerDialog(dialog));
	}

	void setLastResponse(Response lastResponse) {
		logger.debug("DialogContext.setLastResponse ");
		if (lastResponse == null) {
			logger.debug("lastResponse = " + null);
		} else {
			logger.debug("lastResponse = "
					+ ((SIPResponse) lastResponse).getFirstLine());
		}

		this.lastResponse = lastResponse;

	}

	Response getLastResponse() {
		return lastResponse;
	}

	/**
	 * Send an SDP re-OFFER to the other side of the B2BUA.
	 * 
	 * @param sdpOffer -- the sdp offer session description.
	 * @throws Exception -- if could not send
	 */
	void sendSdpReOffer(SessionDescription sdpOffer) throws Exception {
		if ( dialog.getState() == DialogState.TERMINATED) {
			logger.warn("Attempt to send SDP re-offer on a terminated dialog");
			return;
		}
		Request sdpOfferInvite = dialog.createRequest(Request.INVITE);

		/*
		 * Set and fix up the sdp offer to send to the opposite side.
		 */

		this.getRtpSession().getReceiver().setSessionDescription(sdpOffer);

		SipUtilities.incrementSessionVersion(sdpOffer);

		SipUtilities.fixupOutboundRequest(dialog, sdpOfferInvite);

		sdpOfferInvite.setContent(sdpOffer.toString(),
				ProtocolObjects.headerFactory.createContentTypeHeader(
						"application", "sdp"));

		ClientTransaction ctx = ((DialogExt) dialog).getSipProvider()
				.getNewClientTransaction(sdpOfferInvite);

		TransactionContext.attach(ctx, Operation.SEND_SDP_RE_OFFER);

		new Thread(new ReInviteSender(this, ctx)).start();

	}

	/**
	 * Send an ACK and record it. If some thread is wating to send re-INVITE
	 * based upon that ACK it will get up and and run the re-INVITE.
	 * 
	 * @param ack
	 * @throws SipException
	 */
	void sendAck(Request ack) throws SipException {
		this.recordLastAckTime();
		this.lastAck = ack;
		dialog.sendAck(ack);

		if (terminateOnConfirm) {
			Request byeRequest = dialog.createRequest(Request.BYE);

			ClientTransaction ctx = ((DialogExt) dialog).getSipProvider()
					.getNewClientTransaction(byeRequest);
			TransactionContext.attach(ctx, Operation.SEND_BYE_TO_MOH_SERVER);
			dialog.sendRequest(ctx);
		}
	}

	/**
	 * Set the "terminate on confirm" flag which will send BYE to the dialog If
	 * the flag is set as soon as the MOH server confirms the dialog, the flag
	 * is consulted and a BYE sent to the MOH server.
	 * 
	 */
	void setTerminateOnConfirm() {
		this.terminateOnConfirm = true;
		/*
		 * Fire off a timer to reap this guy if he does not die in 8 seconds.
		 */
		Gateway.getTimer().schedule(new TimerTask() {
			@Override
			public void run() {
				if (DialogContext.this.dialog.getState() != DialogState.TERMINATED) {
					DialogContext.this.dialog.delete();
				}
			}
		}, 8000);

	}

	void sendReInvite(ClientTransaction clientTransaction) {
		new Thread(new ReInviteSender(this, clientTransaction)).start();
	}

	void sendMohInvite(ClientTransaction mohClientTransaction) {
		Gateway.getTimer().schedule(new MohTimer(mohClientTransaction), 500);
	}

	void setPeerDialog(Dialog peerDialog) {
		logger.debug("DialogContext.setPeerDialog: " + peerDialog);
		this.peerDialog = peerDialog;
	}

	Dialog getPeerDialog() {
		return peerDialog;
	}

	void setForwardByeToPeer(boolean forwardByeToPeer) {
		logger.debug("setForwardByeToPeer " + forwardByeToPeer);
		this.forwardByeToPeer = forwardByeToPeer;
	}

	boolean isForwardByeToPeer() {
		return forwardByeToPeer;
	}

}
