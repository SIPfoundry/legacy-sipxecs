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
import gov.nist.javax.sip.header.extensions.MinSE;
import gov.nist.javax.sip.header.extensions.ReplacesHeader;
import gov.nist.javax.sip.message.SIPResponse;
import gov.nist.javax.sip.stack.SIPDialog;
import gov.nist.javax.sip.stack.SIPServerTransaction;

import java.text.ParseException;
import java.util.HashSet;
import java.util.Locale;
import java.util.TimerTask;

import javax.sdp.SessionDescription;
import javax.sip.ClientTransaction;
import javax.sip.Dialog;
import javax.sip.DialogState;
import javax.sip.InvalidArgumentException;
import javax.sip.RequestEvent;
import javax.sip.ResponseEvent;
import javax.sip.ServerTransaction;
import javax.sip.SipException;
import javax.sip.SipProvider;
import javax.sip.TransactionAlreadyExistsException;
import javax.sip.TransactionState;
import javax.sip.address.SipURI;
import javax.sip.header.AcceptHeader;
import javax.sip.header.AcceptLanguageHeader;
import javax.sip.header.CSeqHeader;
import javax.sip.header.ContactHeader;
import javax.sip.header.ContentTypeHeader;
import javax.sip.header.EventHeader;
import javax.sip.header.ReasonHeader;
import javax.sip.header.SubscriptionStateHeader;
import javax.sip.header.SupportedHeader;
import javax.sip.header.ToHeader;
import javax.sip.header.WarningHeader;
import javax.sip.message.Request;
import javax.sip.message.Response;

import org.apache.log4j.Logger;
import org.sipfoundry.sipxbridge.symmitron.KeepaliveMethod;
import org.sipfoundry.sipxbridge.symmitron.SymmitronResetHandler;

/**
 * This class does some pre-processing of requests and then acts as a high level
 * router for routing the request to the appropriate B2BUA. It processes INVITE,
 * REFER, ACK, OPTIONS, BYE.
 * 
 * @author M. Ranganathan
 * 
 */
class CallControlManager implements SymmitronResetHandler {

	static Logger logger = Logger.getLogger(CallControlManager.class);

	// ////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Internal classes.
	// ///////////////////////////////////////////////////////////////////////////////////////////////////////////////

	/**
	 * This timer task is kicked off when we get a 491 RequestPending. The timer
	 * task completes and re-tries the INVITE.
	 * 
	 */
	class RequestPendingTimerTask extends TimerTask {

		private ContinuationData continuationData;

		public RequestPendingTimerTask(ContinuationData continuationData) {
			this.continuationData = continuationData;
		}

		@Override
		public void run() {
			RequestEvent requestEvent = continuationData.getRequestEvent();

			if (continuationData.getOperation() == Operation.REFER_INVITE_TO_SIPX_PROXY) {
				processRefer(requestEvent);
			} else if (continuationData.getOperation() == Operation.PROCESS_INVITE) {
				processInvite(requestEvent);
			} else {
				logger.fatal("Unknown operation seen - INTERNAL ERROR");
			}
		}

	}

	/**
	 * Timer task to tear down the replaced dialog in case the other end went
	 * away and left us hanging.
	 */
	class TearDownReplacedDialogTimerTask extends TimerTask {
		Dialog replacedDialog;

		public TearDownReplacedDialogTimerTask(Dialog dialog) {
			this.replacedDialog = dialog;
		}

		public void run() {
			try {
				if (replacedDialog.getState() != DialogState.TERMINATED) {
					SipProvider lanProvider = ((DialogExt) replacedDialog)
							.getSipProvider();
					Request byeRequest = replacedDialog
							.createRequest(Request.BYE);
					ClientTransaction byeCtx = lanProvider
							.getNewClientTransaction(byeRequest);
					TransactionContext.attach(byeCtx,
							Operation.SEND_BYE_TO_REPLACED_DIALOG);
					replacedDialog.sendRequest(byeCtx);
				}
			} catch (Exception ex) {
				logger.error("Error sending bye to replaced dialog", ex);
			}
		}
	}

	// ///////////////////////////////////////////////////////////////////////////////////////////
	// Request handlers.
	// ///////////////////////////////////////////////////////////////////////////////////////////

	/**
	 * Does the request processing for a re-INVITATION. A re-INVITE can be seen
	 * on the following conditions: - codec renegotiation. - hold/resume - sdp
	 * solicitation.
	 * 
	 * @param requestEvent
	 * @throws Exception
	 */
	private void handleReInvite(RequestEvent requestEvent) throws Exception {
		logger.debug("Re-INVITE proessing !! ");

		ServerTransaction serverTransaction = requestEvent
				.getServerTransaction();
		Dialog dialog = serverTransaction.getDialog();
		Request request = requestEvent.getRequest();
		SipProvider provider = (SipProvider) requestEvent.getSource();

		DialogContext dat = (DialogContext) dialog.getApplicationData();

		Dialog peerDialog = dat.peerDialog;

		/*
		 * The peer dialog may not be found if our re-INVITE did not succeed.
		 */
		if (peerDialog == null) {
			Response response = SipUtilities.createResponse(serverTransaction,
					Response.CALL_OR_TRANSACTION_DOES_NOT_EXIST);
			ReasonHeader reasonHeader = ProtocolObjects.headerFactory
					.createReasonHeader(Gateway.SIPXBRIDGE_USER,
							ReasonCode.CALL_SETUP_ERROR,
							"Could not process re-INVITE : No peer!");
			response.setReasonPhrase("Peer dialog is null");
			serverTransaction.sendResponse(response);
			if (dat.getBackToBackUserAgent() != null) {
				dat.getBackToBackUserAgent().tearDown(reasonHeader);
			}
			return;
		}

		SipProvider peerDialogProvider = ((DialogExt) peerDialog)
				.getSipProvider();

		/*
		 * Is he other side trying to solicit an offer?
		 */

		if (SipUtilities.isSdpOfferSolicitation(request)) {
			/*
			 * This case occurs if MOH is turned OFF on sipxbridge and is turned
			 * ON on the phone. In this case the phone will solicit the ITSP for
			 * an offer See Issue 1739
			 */
			Request newRequest = peerDialog.createRequest(Request.INVITE);

			/*
			 * Contact header for the re-INVITE we are about to send.
			 */
			ContactHeader contactHeader = SipUtilities.createContactHeader(
					Gateway.SIPXBRIDGE_USER, peerDialogProvider);
			newRequest.setHeader(contactHeader);

			ClientTransaction ctx = peerDialogProvider
					.getNewClientTransaction(newRequest);

			/*
			 * Set up the transaction context.
			 */
			TransactionContext tad = TransactionContext.attach(ctx,
					Operation.FORWARD_SDP_SOLICITIATION);

			tad.setServerTransaction(serverTransaction);

			/*
			 * Set up the continuation data so we know what to do when the
			 * response arrives.
			 */
			tad.setContinuationData(new ForwardSdpSolicitationContinuationData(
					requestEvent));
			serverTransaction.setApplicationData(tad);

			DialogContext peerDat = DialogContext.get(peerDialog);

			/*
			 * Incoming rquest came in on the LAN side. Check if there is a
			 * record for the ITSP on the wan side of the association.
			 */
			if (provider == Gateway.getLanProvider()
					&& (peerDat.getItspInfo() == null || peerDat.getItspInfo()
							.isGlobalAddressingUsed())) {
				SipUtilities.setGlobalAddresses(newRequest);
			}
			/*
			 * Record in the corresponding dialog that that we solicited an
			 * offer so we can send the Ack along with the SDP that is offered.
			 */
			peerDat
					.setPendingAction(PendingDialogAction.PENDING_FORWARD_ACK_WITH_SDP_ANSWER);
			peerDialog.sendRequest(ctx);

		} else {

			RtpSession rtpSession = dat.getRtpSession();

			/*
			 * Associate the inbound session description with the TRANSMITTER
			 * side of the rtpSession.
			 */
			RtpSessionOperation operation = RtpSessionUtilities
					.reAssignRtpSessionParameters(serverTransaction);

			logger.debug("Rtp Operation " + operation);

			/*
			 * The request originated from the LAN side. Otherwise, the request
			 * originated from WAN we sent it along to the phone on the previous
			 * step. If we handled the request locally then send an ok back.
			 * This happens when the provider does not support re-INVITE
			 */
			if (operation == RtpSessionOperation.PLACE_HOLD) {
				if (Gateway.getMusicOnHoldUri() != null) {
					SendInviteToMohServerContinuationData cdata = new SendInviteToMohServerContinuationData(
							requestEvent);
					dat.solicitSdpOfferFromPeerDialog(cdata);
				} else {
					/*
					 * No MOH support on bridge so send OK right away.
					 */
					Response response = SipUtilities.createResponse(
							serverTransaction, Response.OK);
					SessionDescription sessionDescription = rtpSession
							.getReceiver().getSessionDescription();
					SipUtilities.setSessionDescription(response,
							sessionDescription);
					/*
					 * Send an OK to the other side with a SD that indicates
					 * that the HOLD operation is successful.
					 */
					serverTransaction.sendResponse(response);

				}
			} else if (operation == RtpSessionOperation.REMOVE_HOLD
					|| operation == RtpSessionOperation.CODEC_RENEGOTIATION
					|| operation == RtpSessionOperation.PORT_REMAP) {
				/*
				 * Remove hold and codec renegotiation require forwarding of
				 * re-INVITE.
				 */
				RtpSessionUtilities.forwardReInvite(rtpSession,
						serverTransaction, dialog);
			} else {
				/*
				 * This is a request that can be handled locally. Grab the
				 * previous session description from the receiver side.
				 */
				SessionDescription newDescription = rtpSession.getReceiver()
						.getSessionDescription();
				Response response = ProtocolObjects.messageFactory
						.createResponse(Response.OK, request);
				SupportedHeader sh = ProtocolObjects.headerFactory
						.createSupportedHeader("replaces");
				response.setHeader(sh);
				if (newDescription != null) {
					response.setContent(newDescription,
							ProtocolObjects.headerFactory
									.createContentTypeHeader("application",
											"sdp"));
				}

				ToHeader toHeader = (ToHeader) request.getHeader(ToHeader.NAME);
				String userName = ((SipURI) toHeader.getAddress().getURI())
						.getUser();
				ContactHeader contactHeader = SipUtilities.createContactHeader(
						userName, provider);
				response.setHeader(contactHeader);

				if (serverTransaction != null) {
					serverTransaction.sendResponse(response);
				} else {
					provider.sendResponse(response);

				}
			}

		}

		return;
	}

	/**
	 * Processes an incoming invite from the PBX or from the ITSP side. This
	 * method fields the inbound request and either routes it to the appropriate
	 * b2bua or forwards the request.
	 * 
	 * 
	 * @param requestEvent
	 */
	private void processInvite(RequestEvent requestEvent) {

		Request request = requestEvent.getRequest();

		SipProvider provider = (SipProvider) requestEvent.getSource();
		ServerTransaction serverTransaction = requestEvent
				.getServerTransaction();

		try {

			if (serverTransaction == null) {
				try {
					serverTransaction = provider
							.getNewServerTransaction(request);
				} catch (TransactionAlreadyExistsException ex) {
					if (logger.isDebugEnabled()) {
						logger.debug("Transaction already exists for "
								+ request);
					}
					return;
				}
			}
			Dialog dialog = serverTransaction.getDialog();

			if (dialog.getState() == DialogState.TERMINATED) {
				logger.debug("Got a stray request on a terminated dialog!");
				Response response = SipUtilities.createResponse(
						serverTransaction, Response.SERVER_INTERNAL_ERROR);
				serverTransaction.sendResponse(response);
				return;

			} else if (dialog.getState() == DialogState.CONFIRMED) {
				handleReInvite(requestEvent);
				return;

			}

			BackToBackUserAgent btobua;

			/*
			 * Look at the Dialog context. The B2BUA structure tracks the call
			 * and is pointed to by the dialog application data.
			 */
			if ((DialogContext) dialog.getApplicationData() != null) {
				btobua = ((DialogContext) dialog.getApplicationData())
						.getBackToBackUserAgent();
			} else if (request.getHeader(ReplacesHeader.NAME) != null) {

				/*
				 * Incoming INVITE has a call id that we don't know about but
				 * with a replaces header. This implies call pickup attempt.
				 */

				ReplacesHeader replacesHeader = (ReplacesHeader) request
						.getHeader(ReplacesHeader.NAME);
				Dialog replacesDialog = ((SipStackExt) ProtocolObjects.sipStack)
						.getReplacesDialog(replacesHeader);
				if (replacesDialog == null) {
					Response response = ProtocolObjects.messageFactory
							.createResponse(Response.SERVER_INTERNAL_ERROR,
									request);
					response.setReasonPhrase("Dialog Not Found");
					serverTransaction.sendResponse(response);
					return;
				}
				BackToBackUserAgent b2bua = DialogContext
						.getBackToBackUserAgent(replacesDialog);
				DialogContext dat = DialogContext.get(replacesDialog);
				DialogContext.attach(b2bua, dialog, serverTransaction, request);

				Dialog peerDialog = dat.peerDialog;
				logger.debug("replacesDialogState = "
						+ replacesDialog.getState());
				if (replacesDialog.getState() != DialogState.CONFIRMED) {
					dat.peerDialog = null;
				}

				DialogContext.pairDialogs(dialog, peerDialog);

				b2bua.handleInviteWithReplaces(requestEvent, replacesDialog,
						serverTransaction);
				return;

			} else {

				btobua = Gateway.getBackToBackUserAgentFactory()
						.getBackToBackUserAgent(provider, request,
								serverTransaction, dialog);
				/*
				 * Make sure we know about the incoming request. Otherwise we
				 * return an error here.
				 */
				if (btobua == null) {
					Response response = ProtocolObjects.messageFactory
							.createResponse(Response.NOT_FOUND, request);
					response
							.setReasonPhrase("Could not find account record for ITSP");
					serverTransaction.sendResponse(response);
					return;
				}

			}

			/*
			 * This method was seen from the LAN side. Create a WAN side
			 * association and send the INVITE on its way.
			 */
			if (provider == Gateway.getLanProvider()) {
				logger.debug("Request received from LAN side");
				String toDomain = null;
				// outbound call. better check for valid account

				ItspAccountInfo account = Gateway.getAccountManager()
						.getAccount(request);
				if (account == null) {
					Response response = SipUtilities.createResponse(
							serverTransaction, Response.NOT_FOUND);
					response.setReasonPhrase("ITSP account not found");
					return;
				}
				if (account.getState() == AccountState.INVALID) {
					Response response = ProtocolObjects.messageFactory
							.createResponse(Response.BAD_GATEWAY, request);
					response
							.setReasonPhrase("Account state is INVALID. Check config.");
					serverTransaction.sendResponse(response);
					return;
				}
				/*
				 * This case occurs when in and outbound proxy are different.
				 */

				toDomain = account.getSipDomain();

				/*
				 * Send the call setup invite out.
				 */
				btobua.sendInviteToItsp(requestEvent, serverTransaction,
						toDomain);
			} else {
				logger.debug("request received from Wan side");
				btobua.sendInviteToSipxProxy(requestEvent, serverTransaction);

			}

		} catch (RuntimeException ex) {
			logger.error(
					"Error processing request" + requestEvent.getRequest(), ex);
			CallControlUtilities.sendInternalError(serverTransaction, ex);
		} catch (Exception ex) {
			logger.error("Error processing request "
					+ requestEvent.getRequest(), ex);
			CallControlUtilities.sendBadRequestError(serverTransaction, ex);
		}
	}

	/**
	 * Process an OPTIONS request.
	 * 
	 * @param requestEvent
	 *            -- the requestEvent for the OPTIONS.
	 * 
	 */
	private void processOptions(RequestEvent requestEvent) {
		SipProvider provider = (SipProvider) requestEvent.getSource();
		Request request = requestEvent.getRequest();
		ServerTransaction st = requestEvent.getServerTransaction();

		try {
			if (st == null || requestEvent.getDialog() == null) {
				/*
				 * discard out of dialog requests.
				 */
				Response response = ProtocolObjects.messageFactory
						.createResponse(Response.NOT_ACCEPTABLE, request);
				if (st == null) {
					provider.sendResponse(response);
				} else {
					st.sendResponse(response);
				}
				return;

			}

			Response response = ProtocolObjects.messageFactory.createResponse(
					Response.OK, request);

			ContactHeader contactHeader = null;
			if (provider == Gateway.getLanProvider()) {
				SipUtilities.addLanAllowHeaders(response);
				contactHeader = SipUtilities
						.createContactHeader(null, provider);

				SupportedHeader sh = ProtocolObjects.headerFactory
						.createSupportedHeader("replaces");
				response.setHeader(sh);
			} else {

				SipUtilities.addWanAllowHeaders(response);
				contactHeader = SipUtilities
						.createContactHeader(null, provider);
			}

			AcceptHeader acceptHeader = ProtocolObjects.headerFactory
					.createAcceptHeader("application", "sdp");
			response.setHeader(contactHeader);
			response.setHeader(acceptHeader);
			// Should probably have a configurable option for this.
			Locale locale = Locale.ENGLISH;
			AcceptLanguageHeader acceptLanguage = ProtocolObjects.headerFactory
					.createAcceptLanguageHeader(locale);
			response.setHeader(acceptLanguage);
			Dialog dialog = requestEvent.getDialog();
			SipUtilities.addAllowHeaders(response, provider);
			if (dialog != null) {
				/*
				 * This is an In-dialog request. We add our session description
				 * to the response.
				 */
				DialogContext dat = DialogContext.get(dialog);
				if (dat != null) {
					RtpSession rtpSession = DialogContext.getRtpSession(dialog);
					if (rtpSession != null) {
						SessionDescription sd = rtpSession.getReceiver()
								.getSessionDescription();
						if (sd != null) {
							response.setContent(sd.toString(),
									ProtocolObjects.headerFactory
											.createContentTypeHeader(
													"application", "sdp"));
						}
					}
				}

			}

			/*
			 * If In-Dialog, then the stack will create a server transaction for
			 * you to respond stateufully.
			 */

			st.sendResponse(response);

		} catch (Exception ex) {
			logger.error("Internal error processing request ", ex);
			try {
				Response response = ProtocolObjects.messageFactory
						.createResponse(Response.SERVER_INTERNAL_ERROR, request);
				if (logger.isDebugEnabled()) {
					response.setReasonPhrase(ex.getStackTrace()[0]
							.getFileName()
							+ ":" + ex.getStackTrace()[0].getLineNumber());
				}

				if (st != null) {
					st.sendResponse(response);
				} else {
					provider.sendResponse(response);
				}

			} catch (Exception e) {
				throw new SipXbridgeException("Check gateway configuration", e);
			}
		}

	}

	/**
	 * Handle inbound REFER request.
	 * 
	 * @param requestEvent
	 */
	private void processRefer(RequestEvent requestEvent) {
		TransactionContext tad = null;

		BackToBackUserAgent btobua = null;
		try {

			logger
					.debug("Got a REFER - establishing new call leg and tearing down old call leg");
			Dialog dialog = requestEvent.getDialog();
			Request request = requestEvent.getRequest();
			SipProvider provider = (SipProvider) requestEvent.getSource();
			ServerTransaction stx = requestEvent.getServerTransaction();

			/*
			 * ONLY in-dialog stateful REFER handling is allowed.
			 */
			if (dialog == null || stx == null) {
				logger.error("Got an out of dialog REFER -- dropping");
				Response response = ProtocolObjects.messageFactory
						.createResponse(Response.NOT_IMPLEMENTED, request);
				response.setReasonPhrase("Can only handle In dialog REFER");
				if (stx != null)
					stx.sendResponse(response);
				else
					provider.sendResponse(response);
				return;
			}
			if (provider != Gateway.getLanProvider()) {
				/*
				 * For now do not accept refer from the WAN side later we can
				 * relax this restriction.
				 */
				Response response = SipUtilities.createResponse(stx,
						Response.NOT_ACCEPTABLE);
				response.setReasonPhrase("Can only handle REFER from LAN");
				stx.sendResponse(response);
				return;
			}

			DialogContext dat = (DialogContext) dialog.getApplicationData();
			btobua = dat.getBackToBackUserAgent();
			/*
			 * Check dialog state -- if terminated, tear down call.This should
			 * never happen unless our Dialog is Terminated when we retry.
			 */
			if (dialog.getState() == DialogState.TERMINATED) {
				Response response = SipUtilities.createResponse(stx,
						Response.SERVER_INTERNAL_ERROR);
				response.setReasonPhrase("Received REFER on TERMINATED Dialog");
				stx.sendResponse(response);
			}
			/*
			 * Blind transfer handled by ITSP? If so then forward it if the
			 * bridge is configured to do so. With out ITSP support, blind
			 * transfer can result in no RINGING and dropped calls (if we handle
			 * it locally). Note : REFER is not widely supported by ITSPs
			 * Gateway.getBridgeConfiguration.isReferForwarded() is always
			 * returning FALSE for now. We will enable this code when ITSPs
			 * become better about REFER processing.
			 */
			Dialog peerDialog = DialogContext.getPeerDialog(dialog);

			DialogContext peerDat = DialogContext.get(peerDialog);

			if (Gateway.getBridgeConfiguration().isReferForwarded()
					&& !SipUtilities.isReplacesHeaderPresent(requestEvent
							.getRequest()) && peerDat.isReferAllowed()) {
				btobua.forwardReferToItsp(requestEvent);
				return;
			}

			/*
			 * Re-INVITE the refer Target.
			 * 
			 * The ITSP supports re-invite. Send him a Re-INVITE to solicit an
			 * offer. So we can determine what Codec he supports.
			 */
			Request inviteRequest = btobua
					.createInviteFromReferRequest(requestEvent);

			ReferInviteToSipxProxyContinuationData continuation = new ReferInviteToSipxProxyContinuationData(
					inviteRequest, requestEvent);

			if (!dat.solicitSdpOfferFromPeerDialog(continuation)) {
				Gateway.getTimer().schedule(
						new RequestPendingTimerTask(continuation), 100);
			}

		} catch (ParseException ex) {
			// This should never happen
			logger.fatal("Internal error constructing message ", ex);
			throw new SipXbridgeException("Internal error", ex);

		} catch (InvalidArgumentException ex) {
			logger.fatal("Internal error -- invalid argument", ex);
			throw new SipXbridgeException("Internal error", ex);
		} catch (Exception ex) {
			logger.error("Unexpected exception while processing REFER", ex);
			if (tad != null) {
				ServerTransaction serverTransaction = tad
						.getServerTransaction();
				CallControlUtilities.sendBadRequestError(serverTransaction, ex);
			}

			if (btobua != null) {
				btobua.tearDown();
			}

		}

	}

	/**
	 * Processes an INCOMING ack.
	 * 
	 * @param requestEvent
	 *            -- the ACK request event.
	 */
	private void processAck(RequestEvent requestEvent) {
		try {
			BackToBackUserAgent btobua = DialogContext
					.getBackToBackUserAgent(requestEvent.getDialog());

			if (btobua == null) {
				logger.debug("Could not find B2BUA -- not forwarding ACK ");
				return;
			}
			DialogContext dialogContext = (DialogContext) requestEvent
					.getDialog().getApplicationData();

			Dialog peerDialog = dialogContext.peerDialog;

			if (peerDialog == null) {
				logger
						.debug("Could not find peer dialog -- not forwarding ACK!");
				return;
			}

			/*
			 * Forward the ACK if we have not already done so.
			 */

			Request inboundAck = requestEvent.getRequest();

			DialogContext peerDialogContext = (DialogContext) peerDialog
					.getApplicationData();

			if (logger.isDebugEnabled()) {
				logger.debug("peerDialog = " + peerDialog);
				logger.debug("peerDialogContext " + peerDialogContext);
				if (peerDialogContext.getLastResponse() == null) {
					logger.debug("lastResponse "
							+ peerDialogContext.getLastResponse());
				}
			}

			if (peerDialogContext != null
					&& peerDialog.getState() == DialogState.CONFIRMED
					&& peerDialogContext.getLastResponse() != null
					&& peerDialogContext.getLastResponse().getStatusCode() == Response.OK
					&& ((CSeqHeader) peerDialogContext.getLastResponse()
							.getHeader(CSeqHeader.NAME)).getMethod().equals(
							Request.INVITE)) {
				logger.debug("createAck: " + peerDialog);

				/*
				 * This case happens in loopback calls. We can query sdp from a
				 * peer that is in the pbx.
				 */

				Request ack;

				if (inboundAck.getContentLength().getContentLength() != 0) {
					if (peerDialogContext.getPendingAction() == PendingDialogAction.PENDING_FORWARD_ACK_WITH_SDP_ANSWER) {
						/*
						 * ACK had a content length so we extract the sdp
						 * answer, we re-write it and forward it.
						 */
						ack = peerDialog.createAck(SipUtilities
								.getSeqNumber(peerDialogContext
										.getLastResponse()));

						ContentTypeHeader cth = ProtocolObjects.headerFactory
								.createContentTypeHeader("application", "sdp");
						SessionDescription sd = SipUtilities
								.getSessionDescription(inboundAck);
						SipUtilities.incrementSessionVersion(sd);
						peerDialogContext.getRtpSession().getReceiver()
								.setSessionDescription(sd);
						/*
						 * HACK ALERT Some ITPSs do not like sendonly so make
						 * sure it is sendrecv
						 */
						if (SipUtilities
								.getSessionDescriptionMediaAttributeDuplexity(sd) != null) {
							SipUtilities.setDuplexity(sd, "sendrecv");

						}
						ack.setContent(sd.toString(), cth);
					} else {
						/*
						 * Should I Strip off the SDP answer here? Just loging a
						 * warning for now.
						 */
						logger
								.warn("Got an ACK with SDP but other side does not expect it -- not forwarding ACK");
						return;
					}
				} else {
					/*
					 * Inbound ack had no sdp answer. so we just replay the old
					 * sdp back. This ACK came back as a result of codec
					 * negotiation failure. This is another HACK to try to
					 * support ITSPs that do not respond correctly to SDP offer
					 * solicitations.
					 */

					if (peerDialogContext.getPendingAction() == PendingDialogAction.PENDING_FORWARD_ACK_WITH_SDP_ANSWER) {
						/*
						 * The content length is 0. There is no answer but the
						 * other side expects one. Just silently return.
						 */
						logger
								.debug("no SDP body in ACK but the other side expects one");
						return;

					} else if (peerDialogContext.getPendingAction() == PendingDialogAction.PENDING_SDP_ANSWER_IN_ACK) {
						logger
								.debug("Pending SDP Answer in ACK  -- not forwarding inbound ACK");
						return;
					} else {
						/*
						 * There is no answer and no last response and the other
						 * side does NOT expect one. This is just the default
						 * forwarding action.
						 */
						ack = peerDialog.createAck(SipUtilities
								.getSeqNumber(peerDialogContext
										.getLastResponse()));
					}
				}

				DialogContext.get(peerDialog).recordLastAckTime();

				peerDialog.sendAck(ack);

				/*
				 * Setting this to null here handles the case of Re-invitations.
				 */
				peerDialogContext.setLastResponse(null);

				/*
				 * Set the pending flag to false.
				 */
				peerDialogContext.setPendingAction(PendingDialogAction.NONE);

			}

		} catch (Exception ex) {
			logger.error("Problem sending ack ", ex);
		}

	}

	/**
	 * Processes an INCOMING CANCEL.
	 * 
	 * @param requestEvent
	 *            -- the inbound CANCEL.
	 * 
	 */
	private void processCancel(RequestEvent requestEvent) {

		Dialog dialog = requestEvent.getDialog();

		try {
			Response cancelOk = SipUtilities.createResponse(requestEvent
					.getServerTransaction(), Response.OK);
			requestEvent.getServerTransaction().sendResponse(cancelOk);

			if (requestEvent.getServerTransaction() == null) {
				logger.debug("Null ServerTx: Late arriving cancel");
				return;
			}
			ServerTransaction inviteServerTransaction = ((SIPServerTransaction) requestEvent
					.getServerTransaction()).getCanceledInviteTransaction();

			if (inviteServerTransaction.getState() == TransactionState.PROCEEDING) {
				Response response = SipUtilities.createResponse(
						inviteServerTransaction, Response.REQUEST_TERMINATED);
				inviteServerTransaction.sendResponse(response);
			} else {
				// Too late to cancel.
				logger.debug(String.format(
						"Transaction State is %s too late to cancel",
						inviteServerTransaction.getState()));
				return;
			}
			TransactionContext tad = (TransactionContext) inviteServerTransaction
					.getApplicationData();
			if (tad == null) {
				logger
						.error("No transaction application context state found -- returning");
				return;
			}
			ClientTransaction ct = tad.getClientTransaction();
			ItspAccountInfo itspAccount = DialogContext.get(ct.getDialog())
					.getItspInfo();

			String transport = itspAccount != null ? itspAccount
					.getOutboundTransport() : Gateway.DEFAULT_ITSP_TRANSPORT;
			if (ct.getState() == TransactionState.CALLING
					|| ct.getState() == TransactionState.PROCEEDING) {
				Request cancelRequest = ct.createCancel();

				SipProvider provider = SipUtilities.getPeerProvider(
						(SipProvider) requestEvent.getSource(), transport);
				ClientTransaction clientTransaction = provider
						.getNewClientTransaction(cancelRequest);
				clientTransaction.sendRequest();
			} else {
				logger.debug("CallControlManager:processCancel -- sending BYE "
						+ ct.getState());
				DialogContext dialogApplicationData = (DialogContext) dialog
						.getApplicationData();

				Dialog peerDialog = dialogApplicationData.peerDialog;
				if (peerDialog != null) {
					Request byeRequest = peerDialog.createRequest(Request.BYE);
					SipProvider provider = SipUtilities.getPeerProvider(
							(SipProvider) requestEvent.getSource(), transport);
					ClientTransaction byeCt = provider
							.getNewClientTransaction(byeRequest);
					peerDialog.sendRequest(byeCt);
				}
			}
		} catch (Exception ex) {
			logger.error("Unexpected exception processing cancel", ex);
		}

	}

	/**
	 * Processes an INCOMING BYE
	 */
	private void processBye(RequestEvent requestEvent) {
		try {
			BackToBackUserAgent b2bua = DialogContext
					.getBackToBackUserAgent(requestEvent.getDialog());

			if (requestEvent.getServerTransaction() != null) {
				logger
						.debug("serverTransaction Not found -- stray request -- discarding ");
			}

			if (b2bua != null) {

				b2bua.processBye(requestEvent);

			} else {
				// Respond with an error
				Response response = SipUtilities.createResponse(requestEvent
						.getServerTransaction(),
						Response.CALL_OR_TRANSACTION_DOES_NOT_EXIST);
				requestEvent.getServerTransaction().sendResponse(response);
			}

		} catch (Exception ex) {
			logger.error("Problem sending bye", ex);
		}

	}

	// ///////////////////////////////////////////////////////////////////////////////////////////////
	// Response Handlers
	// ///////////////////////////////////////////////////////////////////////////////////////////////
	/**
	 * 
	 * Handle an ERROR response.
	 * 
	 * @param responseEvent
	 *            -- the incoming error response event.
	 * 
	 */
	private void inviteErrorResponse(ResponseEvent responseEvent)
			throws Exception {
		/*
		 * Grab the response from the IB response event.
		 */

		Response response = responseEvent.getResponse();

		/*
		 * Dialog for the response.
		 */
		Dialog dialog = responseEvent.getDialog();

		/*
		 * The dialog context associated with this dialog.
		 */
		DialogContext dialogContext = DialogContext.get(dialog);

		/*
		 * The call context from the Dialog context.
		 */

		BackToBackUserAgent b2bua = dialogContext.getBackToBackUserAgent();

		if (responseEvent.getClientTransaction() == null) {
			logger.warn("null client transaction");
			return;
		}

		logger.debug("Processing ERROR Response " + response.getStatusCode());

		// Processing an error resonse.
		ClientTransaction ct = responseEvent.getClientTransaction();
		TransactionContext tad = (TransactionContext) ct.getApplicationData();
		if (tad != null) {
			ServerTransaction serverTransaction = tad.getServerTransaction();
			/*
			 * We do not forward back error responses for requests such as REFER
			 * that we are handling locally.
			 */
			Operation continuationOperation = tad.getContinuationOperation();
			if (continuationOperation != Operation.NONE) {
				ContinuationData cdata = (ContinuationData) tad
						.getContinuationData();
				Response errorResponse = SipUtilities.createResponse(cdata
						.getRequestEvent().getServerTransaction(), response
						.getStatusCode());
				if (cdata.getRequestEvent().getServerTransaction().getState() != TransactionState.TERMINATED) {
					cdata.getRequestEvent().getServerTransaction()
							.sendResponse(errorResponse);
				}
			}

			if (tad.getOperation() == Operation.CANCEL_REPLACED_INVITE) {
				logger.debug("ingoring 4xx response " + tad.getOperation());
			} else if (tad.getOperation() != Operation.REFER_INVITE_TO_SIPX_PROXY) {
				if (serverTransaction != null) {
					if (serverTransaction.getState() != TransactionState.TERMINATED) {
						Response newResponse = SipUtilities.createResponse(
								serverTransaction, response.getStatusCode());
						serverTransaction.sendResponse(newResponse);
					} else {
						logger
								.error("Received an error response after final response sent -- ignoring the response");
					}
				} else {
					b2bua.tearDown(ProtocolObjects.headerFactory
							.createReasonHeader(Gateway.SIPXBRIDGE_USER,
									ReasonCode.CALL_SETUP_ERROR,
									"CallControlManager: Call transfer error"));
				}
			} else {
				Dialog referDialog = tad.getReferingDialog();
				Request referRequest = tad.getReferRequest();
				if (referDialog != null
						&& referDialog.getState() == DialogState.CONFIRMED) {
					this.notifyReferDialog(referRequest, referDialog, response);
				}
				/*
				 * Tear down the call.
				 */
				b2bua.tearDown(ProtocolObjects.headerFactory
						.createReasonHeader(Gateway.SIPXBRIDGE_USER,
								ReasonCode.CALL_SETUP_ERROR,
								"CallControlManager: Call setup error"));
			}
		}
	}

	private void inviteToItspOrProxyResponse(ResponseEvent responseEvent)
			throws Exception {

		Dialog dialog = responseEvent.getDialog();
		DialogContext dialogContext = DialogContext.get(dialog);
		ClientTransaction ctx = responseEvent.getClientTransaction();
		TransactionContext transactionContext = TransactionContext.get(ctx);
		Response response = responseEvent.getResponse();

		BackToBackUserAgent b2bua = dialogContext.getBackToBackUserAgent();


		if (logger.isDebugEnabled()) {
			logger.debug("dialogContext = " + dialogContext);
			logger.debug("dialogPeer = " + DialogContext.getPeerDialog(dialog));
			logger.debug("dialog  = " + dialog);
		}
		/*
		 * Store away our incoming response - get ready for ACKL
		 */
		dialogContext.setLastResponse(response);

		/*
		 * Now send the respose to the server side of the transaction.
		 */
		ServerTransaction serverTransaction = transactionContext
				.getServerTransaction();

		Response newResponse = ProtocolObjects.messageFactory.createResponse(
				response.getStatusCode(), serverTransaction.getRequest());
		SupportedHeader sh = ProtocolObjects.headerFactory
				.createSupportedHeader("replaces");

		newResponse.setHeader(sh);

		ToHeader toHeader = (ToHeader) transactionContext
				.getServerTransaction().getRequest().getHeader(ToHeader.NAME);

		String user = ((SipURI) toHeader.getAddress().getURI()).getUser();
		ContactHeader contactHeader = null;

		/*
		 * Set the contact address for the OK. Note that ITSP may want global
		 * addressing.
		 */
		if (transactionContext.getOperation() == Operation.SEND_INVITE_TO_ITSP) {
			contactHeader = SipUtilities.createContactHeader(user,
					transactionContext.getServerTransactionProvider());
		} else {
			contactHeader = SipUtilities.createContactHeader(transactionContext
					.getServerTransactionProvider(), transactionContext
					.getItspAccountInfo());
		}

		newResponse.setHeader(contactHeader);
		ToHeader newToHeader = (ToHeader) newResponse.getHeader(ToHeader.NAME);
		String toTag = transactionContext.createToTag();

		newToHeader.setTag(toTag);

		/*
		 * Fix up the media session using the port in the incoming sdp answer.
		 */
		ContentTypeHeader cth = (ContentTypeHeader) response
				.getHeader(ContentTypeHeader.NAME);

		SessionDescription newSd = null;
		if (response.getRawContent() != null
				&& cth.getContentType().equalsIgnoreCase("application")
				&& cth.getContentSubType().equalsIgnoreCase("sdp")) {
			/*
			 * The incoming media session.
			 */
			SessionDescription sessionDescription = SipUtilities
					.getSessionDescription(response);
			if (logger.isDebugEnabled()) {
				logger.debug("SessionDescription = "
						+ new String(response.getRawContent()));
			}

			/*
			 * Get the outbound RTP session.
			 */
			RtpSession rtpSession = dialogContext.getRtpSession();
			RtpTransmitterEndpoint hisEndpoint = null;
			if (rtpSession != null) {
				hisEndpoint = rtpSession.getTransmitter();
			}

			if (hisEndpoint == null) {
				hisEndpoint = new RtpTransmitterEndpoint(rtpSession, b2bua
						.getSymmitronClient());
				rtpSession.setTransmitter(hisEndpoint);
			}

			KeepaliveMethod keepaliveMethod;

			if (transactionContext.getOperation() == Operation.SEND_INVITE_TO_ITSP) {
				keepaliveMethod = transactionContext.getItspAccountInfo()
						.getRtpKeepaliveMethod();
			} else {
				keepaliveMethod = KeepaliveMethod.NONE;
			}
			hisEndpoint.setKeepAliveMethod(keepaliveMethod);

			hisEndpoint.setSessionDescription(sessionDescription, false);

			Dialog peerDialog = DialogContext.getPeerDialog(dialog);
			RtpReceiverEndpoint incomingEndpoint = DialogContext
					.get(peerDialog).getRtpSession().getReceiver();
			newSd = SipUtilities.getSessionDescription(response);

			/*
			 * Set and update the session description of the inbound session.
			 * This updates the session description.
			 */
			incomingEndpoint.setSessionDescription(newSd);

			newResponse.setContent(newSd.toString(), cth);

			transactionContext.getBackToBackUa().getRtpBridge().start();

		} else if (response.getRawContent() != null) {
			// Cannot recognize header.
			logger.warn("content type is not application/sdp");
			String body = new String(response.getRawContent());
			WarningHeader warningHeader = ProtocolObjects.headerFactory
					.createWarningHeader(Gateway.SIPXBRIDGE_USER,
							WarningCode.UNRECOGNIZED_CONTENT_TYPE,
							"Could not recognize content type");
			newResponse.setHeader(warningHeader);
			newResponse.setContent(body, cth);
		}

		/*
		 * If the inbound dialog is not our peer, by sending an ACK right away.
		 * If he sends a BYE do not forward it. Set a Flag indicating BYE 
		 * should not be forwarded.
		 */
		if (DialogContext.getPeerDialog(serverTransaction.getDialog()) != dialog) {
			if (response.getStatusCode() == 200) {
				Request ackRequest = dialog.createAck(SipUtilities
						.getSeqNumber(response));
				DialogContext.get(dialog).forwardByeToPeer = false;
				DialogContext.get(dialog).setLastResponse(null);
				dialog.sendAck(ackRequest);
			}
		}
		serverTransaction.sendResponse(newResponse);
	}

	/**
	 * Handle the response for the sdp solicitation. This method handles the
	 * response to the sdp offer solicitation re-invite.
	 * 
	 * @param responseEvent
	 */
	private void forwardSdpSolicitationResponse(ResponseEvent responseEvent)
			throws Exception {
		Dialog dialog = responseEvent.getDialog();
		DialogContext dialogContext = DialogContext.get(dialog);

		ClientTransaction ctx = responseEvent.getClientTransaction();
		TransactionContext transactionContext = TransactionContext.get(ctx);
		Response response = responseEvent.getResponse();
		dialogContext.setLastResponse(response);
		ServerTransaction st = transactionContext.getServerTransaction();

		Response newResponse = SipUtilities.createResponse(st, Response.OK);

		if (response.getContentLength().getContentLength() != 0) {
			SessionDescription responseSessionDescription = SipUtilities
					.getSessionDescription(response);
			/*
			 * This is an inbound SDP offer. We record the last offer from the
			 * ITSP in the transmitter.
			 */
			SessionDescription sdCloned = SipUtilities
					.cloneSessionDescription(responseSessionDescription);

			/*
			 * Fix up the session descriptions.
			 */

			DialogContext.getRtpSession(dialog).getTransmitter()
					.setSessionDescription(sdCloned, true);
			DialogContext.getPeerRtpSession(dialog).getReceiver()
					.setSessionDescription(responseSessionDescription);

			SipProvider wanProvider = (SipProvider) ((TransactionExt) st)
					.getSipProvider();

			ContactHeader contactHeader = SipUtilities.createContactHeader(
					wanProvider, dialogContext.getItspInfo());
			ContentTypeHeader cth = ProtocolObjects.headerFactory
					.createContentTypeHeader("application", "sdp");

			// SipUtilities.incrementSessionVersion(sd);

			newResponse.setContent(responseSessionDescription.toString(), cth);
			newResponse.setHeader(contactHeader);
			/*
			 * Mark that we should forward the sdp answer in ACK. We expect an
			 * ACK with sdp answer on the peer dialog. This is what the other
			 * side expects. He wants an ACK with SDP answer in it.
			 */
			dialogContext
					.setPendingAction(PendingDialogAction.PENDING_FORWARD_ACK_WITH_SDP_ANSWER);
			st.sendResponse(newResponse);
		}
	}

	private void handleInviteWithReplacesResponse(ResponseEvent responseEvent)
			throws Exception {

		Dialog dialog = responseEvent.getDialog();
		DialogContext dialogContext = DialogContext.get(dialog);
		ClientTransaction ctx = responseEvent.getClientTransaction();
		TransactionContext tad = TransactionContext.get(ctx);
		Response response = responseEvent.getResponse();

		dialogContext.setLastResponse(response);
		ServerTransaction serverTransaction = tad.getServerTransaction();
		Dialog replacedDialog = tad.getReplacedDialog();
		Request request = serverTransaction.getRequest();
		SipProvider peerProvider = ((TransactionExt) serverTransaction)
				.getSipProvider();
		ContactHeader contactHeader = SipUtilities.createContactHeader(
				Gateway.SIPXBRIDGE_USER, peerProvider);

		Response serverResponse = ProtocolObjects.messageFactory
				.createResponse(response.getStatusCode(), request);
		serverResponse.setHeader(contactHeader);
		if (response.getContentLength().getContentLength() != 0) {
			SessionDescription sdes = SipUtilities
					.getSessionDescription(response);
			SessionDescription sdesCloned = SipUtilities
					.getSessionDescription(response);

			RtpSession inboundRtpSession = DialogContext.getRtpSession(dialog);
			inboundRtpSession.getTransmitter().setSessionDescription(
					sdesCloned, false);

			RtpSession outboundRtpSession = DialogContext.get(replacedDialog)
					.getRtpSession();
			outboundRtpSession.getTransmitter().setOnHold(false);

			outboundRtpSession.getReceiver().setSessionDescription(sdes);
			ContentTypeHeader cth = ProtocolObjects.headerFactory
					.createContentTypeHeader("application", "sdp");
			serverResponse.setContent(sdes.toString(), cth);
		}

		/*
		 * Bid adieu to the replaced dialog if we have not already done so.
		 * Since this might already have been done, we schedule a timer to do so
		 * in due course.
		 */
		if (response.getStatusCode() == Response.OK) {
			Gateway.getTimer().schedule(
					new TearDownReplacedDialogTimerTask(replacedDialog),
					30 * 1000);
		}

		/*
		 * accept the dialog that replaces this dialog.
		 */
		DialogContext serverDat = DialogContext.get(serverTransaction
				.getDialog());
		serverDat.peerDialog = dialog;
		serverTransaction.sendResponse(serverResponse);
		serverDat.setRtpSession(DialogContext.get(replacedDialog)
				.getRtpSession());

		if (replacedDialog.getState() != DialogState.TERMINATED) {
			DialogContext replacedDat = DialogContext.get(replacedDialog);
			replacedDat.setRtpSession(null);
			replacedDat.peerDialog = null;
		}
	}

	/**
	 * Response from a re-invite forwarding
	 * 
	 * @param responseEvent
	 *            - inbound response event.
	 * 
	 */
	private void forwardReInviteResponse(ResponseEvent responseEvent)
			throws Exception {

		Dialog dialog = responseEvent.getDialog();
		DialogContext dialogContext = DialogContext.get(dialog);
		ClientTransaction ctx = responseEvent.getClientTransaction();
		TransactionContext tad = TransactionContext.get(ctx);
		Response response = responseEvent.getResponse();

		BackToBackUserAgent b2bua = DialogContext
				.getBackToBackUserAgent(dialog);

		/*
		 * Store away our incoming response - get ready for ACKL
		 */
		dialogContext.setLastResponse(response);
		dialogContext.setBackToBackUserAgent(b2bua);

		dialog.setApplicationData(dialogContext);

		/*
		 * Now send the respose to the server side of the transaction.
		 */
		ServerTransaction serverTransaction = tad.getServerTransaction();

		/*
		 * If we have a server transaction associated with the response, we ack
		 * when the other side acks.
		 */
		if (serverTransaction != null) {
			Response newResponse = ProtocolObjects.messageFactory
					.createResponse(response.getStatusCode(), serverTransaction
							.getRequest());
			SupportedHeader sh = ProtocolObjects.headerFactory
					.createSupportedHeader("replaces");

			newResponse.setHeader(sh);
			ContactHeader contactHeader = SipUtilities
					.createContactHeader(Gateway.SIPXBRIDGE_USER, tad
							.getServerTransactionProvider());
			newResponse.setHeader(contactHeader);
			Dialog peerDialog = DialogContext.getPeerDialog(dialog);
			SipProvider peerProvider = ((DialogExt) peerDialog)
					.getSipProvider();
			if (response.getContentLength().getContentLength() != 0) {

				RtpSession originalRtpSession = DialogContext
						.getRtpSession(dialog);
				if (originalRtpSession.getTransmitter() != null) {
					SessionDescription transmitterSd = SipUtilities
							.getSessionDescription(response);
					originalRtpSession.getTransmitter().setSessionDescription(
							transmitterSd, false);
				}

				SessionDescription receiverSd = SipUtilities
						.getSessionDescription(response);
				ContentTypeHeader cth = ProtocolObjects.headerFactory
						.createContentTypeHeader("application", "sdp");
				RtpSession rtpSession = DialogContext.getRtpSession(peerDialog);
				rtpSession.getReceiver().setSessionDescription(receiverSd);
				newResponse.setContent(receiverSd.toString(), cth);

			}
			if (peerProvider != Gateway.getLanProvider()) {
				DialogContext peerDat = DialogContext.get(peerDialog);
				if (peerDat.getItspInfo() == null
						|| peerDat.getItspInfo().isGlobalAddressingUsed()) {
					SipUtilities.setGlobalAddress(newResponse);
				}
			}

			serverTransaction.sendResponse(newResponse);
		} else {
			Request ack = dialog.createAck(SipUtilities.getSeqNumber(response));
			dialog.sendAck(ack);
		}

	}

	/**
	 * Handles responses for ReferInviteToSipxProxy or Blind transfer to ITSP.
	 * 
	 * @param responseEvent
	 * @throws Exception
	 */
	private void referInviteToSipxProxyResponse(ResponseEvent responseEvent)
			throws Exception {

		Dialog dialog = responseEvent.getDialog();
		DialogContext dialogContext = DialogContext.get(dialog);
		ClientTransaction ctx = responseEvent.getClientTransaction();
		TransactionContext tad = TransactionContext.get(ctx);
		Response response = responseEvent.getResponse();

		BackToBackUserAgent b2bua = DialogContext
				.getBackToBackUserAgent(dialog);

		/*
		 * This is the case of Refer redirection. In this case, we have already
		 * established a call leg with transfer agent. We already have a RTP
		 * session established with the transfer agent. We need to redirect the
		 * outbound RTP stream to the transfer target. To do this, we fix up the
		 * media session using the port in the incoming sdp answer.
		 */
		ContentTypeHeader cth = (ContentTypeHeader) response
				.getHeader(ContentTypeHeader.NAME);
		Dialog referDialog = tad.getReferingDialog();
		Request referRequest = tad.getReferRequest();
		Dialog peerDialog = DialogContext.getPeerDialog(dialog);
		DialogContext peerDat = DialogContext.get(peerDialog);

		if (response.getRawContent() != null
				&& cth.getContentType().equalsIgnoreCase("application")
				&& cth.getContentSubType().equalsIgnoreCase("sdp")) {
			/*
			 * The incoming media session.
			 */

			SessionDescription sessionDescription = SipUtilities
					.getSessionDescription(response);

			RtpSession rtpSession = ((DialogContext) referDialog
					.getApplicationData()).getRtpSession();

			if (rtpSession != null) {

				/*
				 * Note that we are just pointing the transmitter to another
				 * location. The receiver stays as is.
				 */
				rtpSession.getTransmitter().setSessionDescription(
						sessionDescription, false);
				if (logger.isDebugEnabled()) {
					logger.debug("Receiver State : "
							+ rtpSession.getReceiverState());
				}

				/*
				 * Grab the RTP session previously pointed at by the REFER
				 * dialog.
				 */
				b2bua.getRtpBridge().addSym(rtpSession);

				((DialogContext) dialog.getApplicationData())
						.setRtpSession(rtpSession);

				/*
				 * Check if we need to forward that response and do so if
				 * needed. see issue 1718
				 */
				if (peerDat.transaction != null
						&& peerDat.transaction instanceof ServerTransaction
						&& peerDat.transaction.getState() != TransactionState.TERMINATED) {

					Request request = ((ServerTransaction) peerDat.transaction)
							.getRequest();
					Response forwardedResponse = ProtocolObjects.messageFactory
							.createResponse(response.getStatusCode(), request);
					SipUtilities.setSessionDescription(forwardedResponse,
							sessionDescription);
					ContactHeader contact = SipUtilities.createContactHeader(
							((TransactionExt) peerDat.transaction)
									.getSipProvider(), peerDat.getItspInfo());
					forwardedResponse.setHeader(contact);
					((ServerTransaction) peerDat.transaction)
							.sendResponse(forwardedResponse);
				} else {
					logger
							.debug("not forwarding response peerDat.transaction  = "
									+ peerDat.transaction);
				}

			} else {
				logger
						.debug("Processing ReferRedirection: Could not find RtpSession for referred dialog");
			}

		} else if (response.getRawContent() != null) {
			/*
			 * Got content but it was not SDP.
			 */
			logger
					.error("Encountered unexpected content type from response - not forwarding response");
			ReasonHeader reasonHeader = ProtocolObjects.headerFactory
					.createReasonHeader(Gateway.SIPXBRIDGE_USER,
							ReasonCode.UNEXPECTED_CONTENT_TYPE,
							"unknown content type encountered");
			dialogContext.getBackToBackUserAgent().tearDown(reasonHeader);
			return;
		}

		/*
		 * Got an OK for the INVITE ( that means that somebody picked up ) so we
		 * can hang up the call. We have already redirected the RTP media to the
		 * redirected party at this point.
		 */

		if (referDialog.getState() == DialogState.CONFIRMED) {
			this.notifyReferDialog(referRequest, referDialog, response);
		}

		/*
		 * SDP was returned from the transfer target.
		 */
		if (response.getContentLength().getContentLength() != 0) {
			if (tad.getDialogPendingSdpAnswer() != null
					&& DialogContext.get(tad.getDialogPendingSdpAnswer())
							.getPendingAction() == PendingDialogAction.PENDING_SDP_ANSWER_IN_ACK) {
				Dialog dialogToAck = tad.getDialogPendingSdpAnswer();
				tad.setDialogPendingSdpAnswer(null);
				CallControlUtilities.sendSdpAnswerInAck(response, dialogToAck);
			} else if (dialogContext.getPendingAction() == PendingDialogAction.PENDING_RE_INVITE_WITH_SDP_OFFER) {
				dialogContext.setPendingAction(PendingDialogAction.NONE);
				CallControlUtilities.sendSdpReOffer(response, peerDialog);
			}
		}

		/*
		 * We directly send ACK.
		 */
		if (response.getStatusCode() == Response.OK) {

			b2bua.addDialog(dialog);
			// Thread.sleep(100);
			Request ackRequest = dialog.createAck(((CSeqHeader) response
					.getHeader(CSeqHeader.NAME)).getSeqNumber());
			dialog.sendAck(ackRequest);

		}
		/*
		 * If there is a Music on hold dialog -- tear it down
		 */

		if (response.getStatusCode() == Response.OK) {
			b2bua.sendByeToMohServer();
		}

	}

	/**
	 * Handles responses for ReferInviteToSipxProxy or Blind transfer to ITSP.
	 * 
	 * @param responseEvent
	 * @throws Exception
	 */
	private void blindTransferToItspResponse(ResponseEvent responseEvent)
			throws Exception {

		Dialog dialog = responseEvent.getDialog();
		DialogContext dialogContext = DialogContext.get(dialog);
		ClientTransaction ctx = responseEvent.getClientTransaction();
		TransactionContext tad = TransactionContext.get(ctx);
		Response response = responseEvent.getResponse();

		/*
		 * Do not record the last response here in the dialogContext as we are
		 * going to ack the dialog right away.
		 */

		BackToBackUserAgent b2bua = DialogContext
				.getBackToBackUserAgent(dialog);

		/*
		 * This is the case of Refer redirection. In this case, we have already
		 * established a call leg with transfer agent. We already have a RTP
		 * session established with the transfer agent. We need to redirect the
		 * outbound RTP stream to the transfer target. To do this, we fix up the
		 * media session using the port in the incoming sdp answer.
		 */
		ContentTypeHeader cth = (ContentTypeHeader) response
				.getHeader(ContentTypeHeader.NAME);
		Dialog referDialog = tad.getReferingDialog();
		Request referRequest = tad.getReferRequest();
		Dialog peerDialog = DialogContext.getPeerDialog(dialog);
		DialogContext peerDat = DialogContext.get(peerDialog);

		if (response.getRawContent() != null
				&& cth.getContentType().equalsIgnoreCase("application")
				&& cth.getContentSubType().equalsIgnoreCase("sdp")) {

			/*
			 * The incoming media session.
			 */

			SessionDescription sessionDescription = SipUtilities
					.getSessionDescription(response);

			RtpSession rtpSession = ((DialogContext) referDialog
					.getApplicationData()).getRtpSession();

			if (rtpSession != null) {

				/*
				 * Note that we are just pointing the transmitter to another
				 * location. The receiver stays as is.
				 */
				rtpSession.getTransmitter().setSessionDescription(
						sessionDescription, false);
				if (logger.isDebugEnabled()) {
					logger.debug("Receiver State : "
							+ rtpSession.getReceiverState());
				}

				/*
				 * Grab the RTP session previously pointed at by the REFER
				 * dialog.
				 */
				b2bua.getRtpBridge().addSym(rtpSession);

				((DialogContext) dialog.getApplicationData())
						.setRtpSession(rtpSession);

				/*
				 * Check if we need to forward that response and do so if
				 * needed. see issue 1718
				 */

				if (tad.getServerTransaction() != null
						&& tad.getServerTransaction().getState() != TransactionState.TERMINATED) {

					Request request = tad.getServerTransaction().getRequest();
					Response forwardedResponse = ProtocolObjects.messageFactory
							.createResponse(response.getStatusCode(), request);
					SipUtilities.setSessionDescription(forwardedResponse,
							sessionDescription);
					ContactHeader contact = SipUtilities.createContactHeader(
							((TransactionExt) tad.getServerTransaction())
									.getSipProvider(), peerDat.getItspInfo());
					forwardedResponse.setHeader(contact);
					tad.getServerTransaction().sendResponse(forwardedResponse);

				} else {
					logger
							.debug("not forwarding response peerDat.transaction  = "
									+ tad.getServerTransaction());
				}

			} else {
				logger
						.debug("Processing ReferRedirection: Could not find RtpSession for referred dialog");
			}

		} else if (response.getRawContent() != null) {
			/*
			 * Got content but it was not SDP.
			 */
			logger
					.error("Encountered unexpected content type from response - not forwarding response");
			ReasonHeader reasonHeader = ProtocolObjects.headerFactory
					.createReasonHeader(Gateway.SIPXBRIDGE_USER,
							ReasonCode.UNEXPECTED_CONTENT_TYPE,
							"unknown content type encountered");
			dialogContext.getBackToBackUserAgent().tearDown(reasonHeader);
			return;
		}

		/*
		 * Got an OK for the INVITE ( that means that somebody picked up ) so we
		 * can hang up the call. We have already redirected the RTP media to the
		 * redirected party at this point.
		 */

		if (referDialog.getState() == DialogState.CONFIRMED) {
			this.notifyReferDialog(referRequest, referDialog, response);
		}

		/*
		 * SDP was returned from the transfer target.
		 */
		if (response.getContentLength().getContentLength() != 0) {
			if (tad.getDialogPendingSdpAnswer() != null
					&& DialogContext.get(tad.getDialogPendingSdpAnswer())
							.getPendingAction() == PendingDialogAction.PENDING_SDP_ANSWER_IN_ACK) {
				Dialog dialogToAck = tad.getDialogPendingSdpAnswer();
				tad.setDialogPendingSdpAnswer(null);
				CallControlUtilities.sendSdpAnswerInAck(response, dialogToAck);
			} else if (dialogContext.getPendingAction() == PendingDialogAction.PENDING_RE_INVITE_WITH_SDP_OFFER) {
				dialogContext.setPendingAction(PendingDialogAction.NONE);
				CallControlUtilities.sendSdpReOffer(response, peerDialog);
			}
		}

		/*
		 * We directly send ACK.
		 */
		if (response.getStatusCode() == Response.OK) {

			b2bua.addDialog(dialog);
			// Thread.sleep(100);
			Request ackRequest = dialog.createAck(((CSeqHeader) response
					.getHeader(CSeqHeader.NAME)).getSeqNumber());
			dialog.sendAck(ackRequest);
			b2bua.sendByeToMohServer();
		}

	}

	/**
	 * Handle an SDP offer received in a response which is as a result of
	 * sending an SDP solicitation with 0 length SDP content to the peer of a
	 * dialog.
	 * 
	 * 
	 * @param responseEvent
	 * @throws Exception
	 */

	private void solicitSdpOfferFromPeerDialogResponse(
			ResponseEvent responseEvent) throws Exception {

		/*
		 * Grab the response from the IB response event.
		 */

		Response response = responseEvent.getResponse();

		if (response.getContentLength().getContentLength() == 0) {
			logger
					.warn("PROTOCOL ERROR -- Expecting a content length != 0. Re-use previous SDP answer ");

		}

		/*
		 * Grab the client tx from the inbound response.
		 */
		ClientTransaction clientTransaction = responseEvent
				.getClientTransaction();

		/*
		 * Grab the transaction context from the inbound transaction.
		 */
		TransactionContext transactionContext = TransactionContext
				.get(clientTransaction);
		/*
		 * Dialog for the response.
		 */
		Dialog dialog = responseEvent.getDialog();

		/*
		 * Sequence Number for the response.
		 */
		long seqno = SipUtilities.getSeqNumber(response);

		/*
		 * The dialog context associated with this dialog.
		 */
		DialogContext dialogContext = DialogContext.get(dialog);

		/*
		 * The call context from the Dialog context.
		 */

		BackToBackUserAgent b2bua = dialogContext.getBackToBackUserAgent();

		/*
		 * The continuation context.
		 */
		Operation continuationOperation = transactionContext
				.getContinuationOperation();

		RtpSession peerRtpSession = DialogContext
				.getPeerRtpSession(transactionContext.getContinuationData()
						.getDialog());

		SessionDescription responseSessionDescription;

		if (response.getContentLength().getContentLength() != 0) {
			/*
			 * Reasonable response.
			 */
			responseSessionDescription = SipUtilities
					.getSessionDescription(response);
		} else {
			/*
			 * HACK ALERT : This tries to compensate for the protocol error.
			 * Some ITSPs do not properly handle sdp offers solicitation. They
			 * return back a 0 length sdp answer. In this case, we re-use the
			 * previously sent sdp answer.
			 */
			responseSessionDescription = peerRtpSession.getReceiver()
					.getSessionDescription();

		}

		dialogContext.setLastResponse(response);

		logger.debug("continuationOperation = " + continuationOperation);

		if (continuationOperation == Operation.REFER_INVITE_TO_SIPX_PROXY) {

			/*
			 * ACK the query operation with the previous response. This prevents
			 * the Dialog from timing out while the transfer target waits for
			 * phone pickup.
			 */
			SessionDescription clonedSessionDescription = SipUtilities
					.cloneSessionDescription(responseSessionDescription);

			DialogContext.getRtpSession(dialog).getTransmitter()
					.setSessionDescription(clonedSessionDescription, true);

			if (dialog.getState() != DialogState.TERMINATED) {
				/*
				 * If we do not support MOH or if the park server codecs are not
				 * supported in the answer, ACK right away. Otherwise Send an
				 * INVITE with the answer to the Park Server and concurrently
				 * send another invite to the phone. The Park server will play
				 * music when the phone is ringing ( simulates RINGING ).
				 */
				ReferInviteToSipxProxyContinuationData continuation = (ReferInviteToSipxProxyContinuationData) transactionContext
						.getContinuationData();

				if (!Gateway.getBridgeConfiguration()
						.isMusicOnHoldSupportEnabled()
						|| !SipUtilities.isCodecSupported(
								responseSessionDescription, Gateway
										.getParkServerCodecs())
						|| (b2bua.getMusicOnHoldDialog() != null && b2bua
								.getMusicOnHoldDialog().getState() != DialogState.TERMINATED)) {

					Request ack = dialog.createAck(seqno);

					SessionDescription sessionDescription = SipUtilities
							.getSessionDescription(response);
					HashSet<Integer> codecs = SipUtilities
							.getCodecNumbers(sessionDescription);

					logger.debug("Codecs " + codecs);

					SessionDescription ackSd = dialogContext.getRtpSession()
							.getReceiver().getSessionDescription();
					/*
					 * Restrict the answer to the set of codecs in the offer.
					 */
					SipUtilities.restictToSpecifiedCodecs(ackSd, codecs);

					SipUtilities.setSessionDescription(ack, ackSd);

					/*
					 * Send an ACK back to the WAN side and replay the same
					 * Session description as before. This completes the
					 * handshake so the Dialog will not time out.
					 */
					DialogContext.get(dialog).setLastResponse(null);
					/*
					 * We already ACKED him so we dont owe him an SDP Answer in
					 * the ACK
					 */
					DialogContext.get(dialog).setPendingAction(
							PendingDialogAction.NONE);
					/*
					 * Mark that we need to send a re-INVITE to the peer dialog
					 * when an SDP answer comes in.
					 */

					DialogContext
							.get(continuation.getDialog())
							.setPendingAction(
									PendingDialogAction.PENDING_RE_INVITE_WITH_SDP_OFFER);

					dialog.sendAck(ack);
				} else {
					DialogContext.get(dialog).setLastResponse(response);
					DialogContext
							.get(continuation.getDialog())
							.setPendingAction(
									PendingDialogAction.PENDING_RE_INVITE_WITH_SDP_OFFER);
					/*
					 * We do owe him an SDP answer. Mark it as such so when we
					 * get an answer from Park server, we can pass it on.
					 */

					DialogContext.getRtpSession(continuation.getDialog())
							.getReceiver().setSessionDescription(
									responseSessionDescription);
					ClientTransaction mohCtx = b2bua
							.createClientTxToMohServer(responseSessionDescription);
					/*
					 * Note the dialog to ACK when we get an SDP answer in our
					 * transaction context.
					 */
					TransactionContext.get(mohCtx).setDialogPendingSdpAnswer(
							dialog);
					DialogContext.get(mohCtx.getDialog()).setPendingAction(
							PendingDialogAction.PENDING_SDP_ANSWER_IN_ACK);
					DialogContext.get(mohCtx.getDialog()).peerDialog = dialog;
					mohCtx.sendRequest();

				}

				/*
				 * Need to set the attribute to sendrecv because the MOH client
				 * tx recvonly.
				 */
				SessionDescription referSessionDescription = SipUtilities
						.cloneSessionDescription(responseSessionDescription);
				SipUtilities.setDuplexity(referSessionDescription, "sendrecv");
				DialogContext.getRtpSession(continuation.getDialog())
						.getReceiver().setSessionDescription(
								responseSessionDescription);

				b2bua.referInviteToSipxProxy(continuation.getRequest(),
						continuation.getRequestEvent(),
						responseSessionDescription);
			}

		} else if (continuationOperation == Operation.SEND_INVITE_TO_MOH_SERVER) {
			/*
			 * This is a query for MOH server. Lets see if he returned a codec
			 * that park server handle in the query. If the ITSP does not return
			 * a codec that the park server supports, we just ACK with the
			 * previously negotiated codec. MOH will not play in this case. Note
			 * that MOH answers right away so we can forward the request.
			 */
			if (response.getContentLength().getContentLength() == 0) {
				logger
						.warn("PROTOCOL ERROR -- Expecting a content length != 0. Re-use previous SDP answer ");

				long cseq = SipUtilities.getSeqNumber(response);
				Request ack = dialog.createAck(cseq);

				dialog.sendAck(ack);
				return;

			}

			/*
			 * Does our park server support the codecs in the SDP offer? If not,
			 * or if we already have an existing MOH dialog with the park
			 * server, we just reply back with a suitable ACK right away. Note
			 * that this ACK should be an SDP answer.
			 */
			if (!SipUtilities.isCodecSupported(responseSessionDescription,
					Gateway.getParkServerCodecs())
					|| (b2bua.getMusicOnHoldDialog() != null && b2bua
							.getMusicOnHoldDialog().getState() != DialogState.TERMINATED)) {
				/*
				 * If codec is not supported by park server then we simply do
				 * not forward the answer to the park server in an INIVTE. We
				 * just replay the old respose (Example: this case happens for
				 * AT&T HIPCS ).
				 */
				DialogContext.get(dialog).setLastResponse(response);

				SessionDescription ackSd = dialogContext.getRtpSession()
						.getReceiver().getSessionDescription();
				/*
				 * Limit the Answer to the codec set found in the offer. Note
				 * that the OFFER is in the INBOUND OK.
				 */
				HashSet<Integer> codecs = SipUtilities
						.getCodecNumbers(responseSessionDescription);
				SipUtilities.restictToSpecifiedCodecs(ackSd, codecs);

				/*
				 * Now reply back to the original Transaction and put the WAN
				 * side on hold. Note tha this case MOH will not play.
				 */
				SendInviteToMohServerContinuationData continuation = (SendInviteToMohServerContinuationData) transactionContext
						.getContinuationData();

				/*
				 * Send an ACK back to the WAN side and replay the same Session
				 * description as before.
				 */
				DialogContext.get(dialog).sendAck(ackSd);

				DialogContext.get(dialog).setPendingAction(
						PendingDialogAction.NONE);
				Request request = continuation.getServerTransaction()
						.getRequest();
				Response newResponse = ProtocolObjects.messageFactory
						.createResponse(Response.OK, request);

				RtpSession lanRtpSession = DialogContext
						.getPeerRtpSession(dialog);
				RtpSession wanRtpSession = DialogContext.getRtpSession(dialog);

				SipUtilities.setDuplexity(lanRtpSession.getReceiver()
						.getSessionDescription(), "recvonly");
				SipUtilities.incrementSessionVersion(lanRtpSession
						.getReceiver().getSessionDescription());

				/*
				 * Put the rtp session on hold.
				 */
				wanRtpSession.getTransmitter().setOnHold(true);

				SupportedHeader sh = ProtocolObjects.headerFactory
						.createSupportedHeader("replaces");
				newResponse.setHeader(sh);
				SessionDescription newDescription = lanRtpSession.getReceiver()
						.getSessionDescription();
				if (newDescription != null) {
					newResponse.setContent(newDescription,
							ProtocolObjects.headerFactory
									.createContentTypeHeader("application",
											"sdp"));
				}

				ToHeader toHeader = (ToHeader) request.getHeader(ToHeader.NAME);
				String userName = ((SipURI) toHeader.getAddress().getURI())
						.getUser();
				ContactHeader contactHeader = SipUtilities.createContactHeader(
						userName, ((DialogExt) continuation.getDialog())
								.getSipProvider());
				newResponse.setHeader(contactHeader);
				response.setReasonPhrase("OK MOH Codec not supported by ITSP");

				continuation.getServerTransaction().sendResponse(newResponse);

				return;
			} else {
				SendInviteToMohServerContinuationData continuation = (SendInviteToMohServerContinuationData) transactionContext
						.getContinuationData();

				SessionDescription clonedSd = SipUtilities
						.cloneSessionDescription(responseSessionDescription);
				/*
				 * Set the session description on the wan side.
				 */
				DialogContext.getRtpSession(dialog).getTransmitter()
						.setSessionDescription(responseSessionDescription,
								false);

				/*
				 * Update the ports of the SD to forward to to the lan side.
				 */
				DialogContext.getRtpSession(continuation.getDialog())
						.getReceiver().setSessionDescription(clonedSd);

				/*
				 * Make sure we have not been beaten to the punch. If not, we
				 * can set up a dialog with the MOH server.
				 */
				if (b2bua.getMusicOnHoldDialog() == null
						|| b2bua.getMusicOnHoldDialog().getState() == DialogState.TERMINATED) {
					ClientTransaction ctx = b2bua
							.createClientTxToMohServer(clonedSd);
					/*
					 * Note that we owe the dialog an sdp answer when we get it.
					 */
					TransactionContext.get(ctx).setDialogPendingSdpAnswer(
							dialog);
					ctx.sendRequest();
				}

				/*
				 * We have slipped in an INVITE to the MOH server. Now send the
				 * response to the other side. He thinks we are on hold but we
				 * are now playing MOH to him.
				 */

				Response newResponse = SipUtilities.createResponse(continuation
						.getServerTransaction(), response.getStatusCode());
				ContactHeader contactHeader = SipUtilities.createContactHeader(
						Gateway.SIPXBRIDGE_USER, ((DialogExt) continuation
								.getDialog()).getSipProvider());
				newResponse.setHeader(contactHeader);
				SipUtilities.setSessionDescription(newResponse, clonedSd);
				continuation.getServerTransaction().sendResponse(newResponse);

			}

		}
	}

	/**
	 * Process an INVITE response.
	 * 
	 * @param responseEvent
	 *            -- the response event.
	 */
	private void processInviteResponse(ResponseEvent responseEvent) {

		Response response = responseEvent.getResponse();
		/*
		 * Dialog for the response.
		 */
		Dialog dialog = responseEvent.getDialog();

		if (logger.isDebugEnabled()) {
			logger.debug("processInviteResponse : "
					+ ((SIPResponse) response).getFirstLine() + " dialog = "
					+ dialog);
		}

		/*
		 * Sequence Number for the response.
		 */
		long seqno = SipUtilities.getSeqNumber(response);

		/*
		 * The dialog context associated with this dialog.
		 */
		DialogContext dialogContext = DialogContext.get(dialog);
		BackToBackUserAgent b2bua;

		try {

			if (dialogContext == null) {
				logger
						.fatal("Could not find DialogApplicationData -- dropping the response");
				try {
					if (response.getStatusCode() == 200) {
						Request ack = dialog.createAck(((CSeqHeader) response
								.getHeader(CSeqHeader.NAME)).getSeqNumber());
						DialogContext.get(dialog).recordLastAckTime();
						dialog.sendAck(ack);

					}

					return;
				} catch (Exception ex) {
					logger.error("Unexpected error sending ACK for 200 OK");
					return;
				}

			}
			/*
			 * The call context.
			 */
			b2bua = dialogContext.getBackToBackUserAgent();
			if (responseEvent.getClientTransaction() == null) {
				/*
				 * This is an OK retransmission. We ACK it right away. NOTE :
				 * This should never happen unless the dialog has terminated
				 * before we saw the OK. This is a catch for extremely late
				 * arriving OKs.
				 */
				logger
						.debug("Could not find client transaction -- must be stray response.");
				if (response.getStatusCode() == 200 && dialog != null) {
					Request ack = dialog.createAck(seqno);
					DialogContext.get(dialog).recordLastAckTime();
					dialog.sendAck(ack);

				}
				return;
			} else if (((TransactionContext) responseEvent
					.getClientTransaction().getApplicationData())
					.getOperation() == Operation.SESSION_TIMER
					&& dialog != null) {

			}
		} catch (Exception ex) {
			logger.error("Unexpected error sending ACK for 200 OK", ex);
			return;
		}

		if (b2bua == null) {
			logger
					.fatal("Could not find a BackToBackUA -- dropping the response");
			throw new SipXbridgeException(
					"Could not find a B2BUA for this response : " + response);
		}

		/*
		 * At this point we have weeded out the
		 */
		try {

			if (response.getStatusCode() == Response.TRYING) {
				/*
				 * We store away our outgoing sdp offer in the application data
				 * of the client tx.
				 */
				TransactionContext tad = (TransactionContext) responseEvent
						.getClientTransaction().getApplicationData();
				if (tad.getOperation() == Operation.REFER_INVITE_TO_SIPX_PROXY
						|| tad.getOperation() == Operation.SPIRAL_BLIND_TRANSFER_INVITE_TO_ITSP) {
					Dialog referDialog = tad.getReferingDialog();
					Request referRequest = tad.getReferRequest();
					if (referDialog.getState() == DialogState.CONFIRMED) {
						this.notifyReferDialog(referRequest, referDialog,
								response);
					}
				}

			} else if (response.getStatusCode() > 100
					&& response.getStatusCode() <= 200) {

				/*
				 * Set our final dialog. Note that the 1xx Dialog may be
				 * different.
				 */
				b2bua.addDialog(dialog);

				/*
				 * We store away our outgoing sdp offer in the application data
				 * of the client tx.
				 */
				TransactionContext tad = (TransactionContext) responseEvent
						.getClientTransaction().getApplicationData();

				logger.debug("Operation = " + tad.getOperation());

				/*
				 * The TransactionApplicationData operator will indicate what
				 * the OK is for.
				 */
				/*
				 * This is an OK for the session timer
				 */
				if (tad.getOperation() == Operation.SESSION_TIMER) {
					if (response.getStatusCode() == 200) {
						Request ack = dialog.createAck(((CSeqHeader) response
								.getHeader(CSeqHeader.NAME)).getSeqNumber());
						dialogContext.recordLastAckTime();
						dialog.sendAck(ack);

					}
				} else if (tad.getOperation() == Operation.SEND_SDP_RE_OFFER) {
					/*
					 * We sent the other side a re-OFFER.
					 */

					if (response.getStatusCode() == Response.OK) {
						RtpSession rtpSession = DialogContext
								.getRtpSession(dialog);
						SessionDescription inboundSessionDescription = SipUtilities
								.getSessionDescription(response);
						rtpSession.getTransmitter().setSessionDescription(
								inboundSessionDescription, false);
						rtpSession.getTransmitter().setOnHold(false);
						Request ack = dialog.createAck(seqno);
						dialog.sendAck(ack);
					}
					return;
				} else if (tad.getOperation() == Operation.FORWARD_SDP_SOLICITIATION) {
					this.forwardSdpSolicitationResponse(responseEvent);
				} else if (tad.getOperation() == Operation.SOLICIT_SDP_OFFER_FROM_PEER_DIALOG
						&& response.getStatusCode() == 200) {
					this.solicitSdpOfferFromPeerDialogResponse(responseEvent);
				} else if (tad.getOperation() == Operation.SEND_INVITE_TO_ITSP
						|| tad.getOperation() == Operation.SEND_INVITE_TO_SIPX_PROXY) {
					this.inviteToItspOrProxyResponse(responseEvent);
				} else if (tad.getOperation() == Operation.REFER_INVITE_TO_SIPX_PROXY) {
					this.referInviteToSipxProxyResponse(responseEvent);
				} else if (tad.getOperation() == Operation.SPIRAL_BLIND_TRANSFER_INVITE_TO_ITSP) {
					this.blindTransferToItspResponse(responseEvent);
				} else if (tad.getOperation().equals(
						Operation.SEND_INVITE_TO_MOH_SERVER)) {
					if (response.getStatusCode() == Response.OK) {
						Request ack = dialog.createAck(((CSeqHeader) response
								.getHeader(CSeqHeader.NAME)).getSeqNumber());

						dialog.sendAck(ack);
						if (tad.getDialogPendingSdpAnswer() != null
								&& DialogContext.getPendingAction(tad
										.getDialogPendingSdpAnswer()) == PendingDialogAction.PENDING_SDP_ANSWER_IN_ACK) {
							CallControlUtilities.sendSdpAnswerInAck(response,
									tad.getDialogPendingSdpAnswer());
						}

					}
				} else if (tad.getOperation()
						.equals(Operation.FORWARD_REINVITE)) {
					this.forwardReInviteResponse(responseEvent);
				} else if (tad.getOperation() == Operation.HANDLE_SPIRAL_INVITE_WITH_REPLACES) {
					if (response.getStatusCode() == Response.OK) {
						Request ack = dialog.createAck(((CSeqHeader) response
								.getHeader(CSeqHeader.NAME)).getSeqNumber());

						dialog.sendAck(ack);

						if (tad.getDialogPendingSdpAnswer() != null
								&& DialogContext.getPendingAction(tad
										.getDialogPendingSdpAnswer()) == PendingDialogAction.PENDING_SDP_ANSWER_IN_ACK) {
							CallControlUtilities.sendSdpAnswerInAck(response,
									tad.getDialogPendingSdpAnswer());
						}
					}
				} else if (tad.getOperation() == Operation.HANDLE_INVITE_WITH_REPLACES) {
					handleInviteWithReplacesResponse(responseEvent);
				} else {
					logger
							.fatal("CallControlManager: Unknown Case in if statement ");
				}
			} else if (response.getStatusCode() == Response.REQUEST_PENDING) {
				/*
				 * A glare condition was detected. Start a timer and retry the
				 * operation after timeout later.
				 */
				TransactionContext tad = (TransactionContext) responseEvent
						.getClientTransaction().getApplicationData();
				if (tad.getContinuationData() == null
						|| tad.getContinuationData().getOperation() != Operation.REFER_INVITE_TO_SIPX_PROXY) {
					logger.warn("Unexpected REQUEST_PENDING");
					b2bua.tearDown();
					return;

				}
				Gateway.getTimer().schedule(
						new RequestPendingTimerTask(tad.getContinuationData()),
						1000);
			} else if (response.getStatusCode() == Response.INTERVAL_TOO_BRIEF) {
				MinSE minSe = (MinSE) response.getHeader(MinSE.NAME);
				if (minSe != null) {
					dialogContext.setSetExpires(minSe.getExpires());
				}

			} else if (response.getStatusCode() > 200) {
				this.inviteErrorResponse(responseEvent);
			}

		} catch (ParseException ex) {
			logger.error("Unexpected parse exception", ex);
			throw new SipXbridgeException("Unexpected exception", ex);
		} catch (InvalidArgumentException ex) {
			logger.error("Unpexpected exception", ex);
			throw new SipXbridgeException("Unexpected exception", ex);
		} catch (Exception ex) {
			// Some other exception occured during processing of the request.
			logger.error("Exception while processing inbound response ", ex);

			if (b2bua != null) {
				try {
					b2bua
							.tearDown(ProtocolObjects.headerFactory
									.createReasonHeader("sipxbridge",
											ReasonCode.UNCAUGHT_EXCEPTION,
											"Unexpected exception processing response"));
				} catch (Exception e) {
					logger.error("unexpected exception", e);
				}
			}
		}

	}

	/**
	 * Process response to an OPTIONS request.
	 * 
	 * @param responseEvent
	 */
	private void processOptionsResponse(ResponseEvent responseEvent) {
		logger.debug("processOptionsResponse ");
	}

	/**
	 * Sends a NOTIFY to the transfer agent containing a SipFrag with the
	 * response received from the Tansafer Target.
	 * 
	 * @param referRequest
	 *            -- the REFER request
	 * @param referDialog
	 *            -- The REFER dialog in which we send NOTIFY.
	 * @param response
	 *            -- the response from the transfer target
	 * @throws SipException
	 */
	private void notifyReferDialog(Request referRequest, Dialog referDialog,
			Response response) throws SipException {
		try {
			Request notifyRequest = referDialog.createRequest(Request.NOTIFY);
			EventHeader eventHeader = ProtocolObjects.headerFactory
					.createEventHeader("refer");
			CSeqHeader cseq = (CSeqHeader) referRequest
					.getHeader(CSeqHeader.NAME);
			long cseqValue = cseq.getSeqNumber();
			eventHeader.setEventId(Integer.toString((int) cseqValue));
			notifyRequest.addHeader(eventHeader);
			String subscriptionState = "active";
			if (response.getStatusCode() >= 200) {
				/*
				 * Final response so TERMINATE the subscription.
				 */
				subscriptionState = "terminated";
			}
			SubscriptionStateHeader subscriptionStateHeader = ProtocolObjects.headerFactory
					.createSubscriptionStateHeader(subscriptionState);
			notifyRequest.addHeader(subscriptionStateHeader);
			// Content-Type: message/sipfrag;version=2.0
			ContentTypeHeader contentTypeHeader = ProtocolObjects.headerFactory
					.createContentTypeHeader("message", "sipfrag");
			// contentTypeHeader.setParameter("version", "2.0");

			String content = ((SIPResponse) response).getStatusLine()
					.toString();
			notifyRequest.setContent(content, contentTypeHeader);
			SipUtilities.addLanAllowHeaders(notifyRequest);
			SipProvider referProvider = ((SIPDialog) referDialog)
					.getSipProvider();
			ClientTransaction ctx = referProvider
					.getNewClientTransaction(notifyRequest);
			referDialog.sendRequest(ctx);
		} catch (ParseException ex) {
			logger.error("Unexpected parse exception ", ex);
			throw new SipXbridgeException("Unexpected parse exception ", ex);
		}

	}

	/**
	 * Process a cancel response.
	 * 
	 * @param responseEvent
	 */
	private void processCancelResponse(ResponseEvent responseEvent) {
		logger.debug("CallControlManager: processCancelResponse");

	}

	/**
	 * Process a NOTIFY response.
	 * 
	 * @param responseEvent
	 */
	private void processNotifyResponse(ResponseEvent responseEvent) {
		logger.debug("CallControlManager: processNotifyResponse");
	}

	/**
	 * Process a REFER response.
	 * 
	 * @param responseEvent
	 */
	private void processReferResponse(ResponseEvent responseEvent) {
		try {
			logger.debug("CallControlManager: processReferResponse");
			ClientTransaction ctx = responseEvent.getClientTransaction();
			TransactionContext tad = (TransactionContext) ctx
					.getApplicationData();
			ServerTransaction referServerTx = tad.getServerTransaction();
			Response referResponse = SipUtilities.createResponse(referServerTx,
					responseEvent.getResponse().getStatusCode());
			if (referServerTx.getState() != TransactionState.TERMINATED) {
				referServerTx.sendResponse(referResponse);
			}
		} catch (Exception ex) {
			try {
				logger.error("Unexpected exception sending response", ex);
				if (responseEvent.getDialog() != null) {
					DialogContext dat = DialogContext.get(responseEvent
							.getDialog());
					BackToBackUserAgent b2bua = dat.getBackToBackUserAgent();
					b2bua.tearDown(ProtocolObjects.headerFactory
							.createReasonHeader("sipxbridge",
									ReasonCode.UNCAUGHT_EXCEPTION,
									"Error processing REFER response"));

				}
			} catch (Exception ex1) {
				logger.fatal("Unexpected exception tearing down call", ex1);

			}
		}

	}

	/**
	 * Process a bye RESPONSE.
	 * 
	 * @param responseEvent
	 */

	private void processByeResponse(ResponseEvent responseEvent) {
		try {
			logger.debug("CallControlManager: processByeResponse");
			ClientTransaction ct = responseEvent.getClientTransaction();
			TransactionContext tad = (TransactionContext) ct
					.getApplicationData();
			if (tad != null) {
				ServerTransaction st = tad.getServerTransaction();
				if (st != null) {
					Response response = responseEvent.getResponse();
					Response newResponse = SipUtilities.createResponse(st,
							response.getStatusCode());
					st.sendResponse(newResponse);
				}
			}

		} catch (Exception ex) {
			logger.error("Exception forwarding bye response", ex);
		}
	}

	// /////////////////////////////////////////////////////////////////////////////////////////////////

	/**
	 * Process an incoming request.
	 */
	void processRequest(RequestEvent requestEvent) {
		Request request = requestEvent.getRequest();
		String method = request.getMethod();
		if (method.equals(Request.INVITE)) {
			processInvite(requestEvent);
		} else if (method.equals(Request.ACK)) {
			processAck(requestEvent);
		} else if (method.equals(Request.BYE)) {
			processBye(requestEvent);
		} else if (method.equals(Request.CANCEL)) {
			processCancel(requestEvent);
		} else if (method.equals(Request.OPTIONS)) {
			processOptions(requestEvent);
		} else if (method.equals(Request.REFER)) {
			processRefer(requestEvent);
		}

	}

	/**
	 * Process an incoming response
	 */
	void processResponse(ResponseEvent responseEvent) {
		Response response = responseEvent.getResponse();
		String method = ((CSeqHeader) response.getHeader(CSeqHeader.NAME))
				.getMethod();
		if (method.equals(Request.INVITE)) {
			processInviteResponse(responseEvent);
		} else if (method.equals(Request.CANCEL)) {
			processCancelResponse(responseEvent);
		} else if (method.equals(Request.BYE)) {
			processByeResponse(responseEvent);
		} else if (method.equals(Request.NOTIFY)) {
			processNotifyResponse(responseEvent);
		} else if (method.equals(Request.REFER)) {
			processReferResponse(responseEvent);
		} else if (method.equals(Request.OPTIONS)) {
			processOptionsResponse(responseEvent);
		}
	}

	/**
	 * The Reset handler for the symmitron. Just tear down all ongoing calls.
	 * This is detected by the Ping operation to the symmitron.
	 * 
	 * Note that each b2bua may be using a different symitron.
	 * 
	 */
	public void reset(String serverHandle) {
		for (BackToBackUserAgent btobua : Gateway
				.getBackToBackUserAgentFactory().getBackToBackUserAgents()) {
			if (serverHandle.equals(btobua.getSymmitronServerHandle())) {
				try {
					btobua.tearDown();
				} catch (Exception ex) {
					logger.error("Error tearing down call ", ex);
				}
			}
		}

	}

}
