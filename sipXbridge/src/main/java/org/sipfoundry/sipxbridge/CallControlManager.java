/*
 *  Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 *  Contributors retain copyright to elements licensed under a Contributor Agreement.
 *  Licensed to the User under the LGPL license.
 *
 */
package org.sipfoundry.sipxbridge;

import gov.nist.javax.sip.DialogExt;
import gov.nist.javax.sip.SipStackExt;
import gov.nist.javax.sip.SipStackImpl;
import gov.nist.javax.sip.TransactionExt;
import gov.nist.javax.sip.header.extensions.MinSE;
import gov.nist.javax.sip.header.extensions.ReplacesHeader;
import gov.nist.javax.sip.header.ims.PAssertedIdentityHeader;
import gov.nist.javax.sip.message.SIPResponse;
import gov.nist.javax.sip.stack.SIPDialog;
import gov.nist.javax.sip.stack.SIPServerTransaction;

import java.io.IOException;
import java.text.ParseException;
import java.util.Collection;
import java.util.HashSet;
import java.util.Iterator;
import java.util.Locale;
import java.util.Random;
import java.util.TimerTask;
import java.util.concurrent.ConcurrentHashMap;

import javax.sdp.SdpFactory;
import javax.sdp.SdpParseException;
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
import javax.sip.address.Address;
import javax.sip.address.SipURI;
import javax.sip.header.AcceptHeader;
import javax.sip.header.AcceptLanguageHeader;
import javax.sip.header.AllowHeader;
import javax.sip.header.CSeqHeader;
import javax.sip.header.ContactHeader;
import javax.sip.header.ContentTypeHeader;
import javax.sip.header.EventHeader;
import javax.sip.header.FromHeader;
import javax.sip.header.SubscriptionStateHeader;
import javax.sip.header.SupportedHeader;
import javax.sip.header.ToHeader;
import javax.sip.header.ViaHeader;
import javax.sip.header.WarningHeader;
import javax.sip.message.Message;
import javax.sip.message.Request;
import javax.sip.message.Response;

import org.apache.log4j.Logger;
import org.sipfoundry.sipxbridge.symmitron.KeepaliveMethod;
import org.sipfoundry.sipxbridge.symmitron.SymmitronException;
import org.sipfoundry.sipxbridge.symmitron.SymmitronResetHandler;

/**
 * The main job of this class is to manage BackToBackUserAgents. It mantains a
 * hash map of BackToBackUserAgents indexed by call id. It acts as a factory for
 * creating the BackToBackUserAgent by looking up the callId in this HashMap and
 * creating a new one if needed. It also acts as a high level router for routing
 * the request to the appropriate B2BUA. It processes INVITE, REFER, ACK,
 * OPTIONS, BYE.
 * 
 * @author M. Ranganathan
 * 
 */
class CallControlManager implements SymmitronResetHandler {

	static Logger logger = Logger.getLogger(CallControlManager.class);

	// ////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Internal classes.
	// ///////////////////////////////////////////////////////////////////////////////////////////////////////////////

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
					TransactionApplicationData.attach(byeCtx,
							Operation.SEND_BYE_TO_REPLACED_DIALOG);
					replacedDialog.sendRequest(byeCtx);
				}
			} catch (Exception ex) {
				logger.error("Error sending bye to replaced dialog", ex);
			}
		}
	}

	// ///////////////////////////////////////////////////////////////////////////////////////////
	// Private methods.
	// ///////////////////////////////////////////////////////////////////////////////////////////
	private void adjustSessionParameters(ServerTransaction serverTransaction,
			Request request, Dialog dialog, SipProvider provider,
			Dialog peerDialog) throws Exception {
		DialogApplicationData dat = (DialogApplicationData) dialog
				.getApplicationData();

		RtpSession lanRtpSession = dat.getRtpSession();

		SessionDescription newDescription = lanRtpSession
				.reAssignSessionParameters(request, serverTransaction, dialog,
						peerDialog);

		/*
		 * The request originated from the LAN side. Otherwise, the request
		 * originated from WAN we sent it along to the phone on the previous
		 * step. If we handled the request locally then send an ok back. This
		 * happens when the provider does not support re-INVITE
		 */
		if (provider == Gateway.getLanProvider()
				&& !lanRtpSession.isReInviteForwarded()) {
			Response response = ProtocolObjects.messageFactory.createResponse(
					Response.OK, request);
			SupportedHeader sh = ProtocolObjects.headerFactory
					.createSupportedHeader("replaces");
			response.setHeader(sh);
			if (newDescription != null) {
				response.setContent(newDescription,
						ProtocolObjects.headerFactory.createContentTypeHeader(
								"application", "sdp"));
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
				logger.debug("Re-INVITE proessing !! ");
				DialogApplicationData dat = (DialogApplicationData) dialog
						.getApplicationData();
				RtpSession rtpSession = dat.getRtpSession();
				Dialog peerDialog = dat.peerDialog;
				SipProvider peerDialogProvider = ((DialogExt) peerDialog)
						.getSipProvider();

				/*
				 * See if we need to re-INVITE MOH server. If the session is
				 * already set up with right codec we do not need to do that.
				 */

				if (!dat.sendReInviteOnResume
						&& provider == Gateway.getLanProvider()
						&& ((!Gateway.isReInviteSupported()) || Gateway
								.getMusicOnHoldAddress() == null)) {
					/*
					 * Can handle this request locally without re-Inviting
					 * server (optimization)
					 */
					if (SipUtilities.isSdpQuery(request)) {
						/*
						 * This case occurs if MOH is turned OFF on sipxbridge
						 * and is turned ON on the phone. In this case the phone
						 * will query the ITSP See Issue 1739
						 */
						Request newRequest = peerDialog
								.createRequest(Request.INVITE);
						if (newRequest.getHeader(ContentTypeHeader.NAME) == null) {
							newRequest.setHeader(ProtocolObjects.headerFactory
									.createContentTypeHeader("application",
											"sdp"));
						}
						ClientTransaction ctx = peerDialogProvider
								.getNewClientTransaction(newRequest);
						TransactionApplicationData tad = TransactionApplicationData
								.attach(ctx,
										Operation.QUERY_SDP_FROM_PEER_DIALOG);

						tad.setServerTransaction(serverTransaction);
						DialogApplicationData peerDat = DialogApplicationData
								.get(peerDialog);
						/*
						 * Record that we queried the answer from the peer
						 * dialog so we can send the Ack along.
						 */
						peerDat.isSdpAnswerPending = true;
						peerDialog.sendRequest(ctx);
						return;
					} else {
						this.adjustSessionParameters(serverTransaction,
								request, dialog, provider, null);
						return;
					}
				} else {
					if (SipUtilities.isSdpQuery(request)) {
						Request newRequest = peerDialog
								.createRequest(Request.INVITE);
						ContactHeader contactHeader = SipUtilities
								.createContactHeader(Gateway.SIPXBRIDGE_USER,
										peerDialogProvider);
						newRequest.setHeader(contactHeader);
						ClientTransaction ctx = peerDialogProvider
								.getNewClientTransaction(newRequest);
						TransactionApplicationData tad = TransactionApplicationData
								.attach(ctx,
										Operation.QUERY_SDP_FROM_PEER_DIALOG);
						tad.setServerTransaction(serverTransaction);
						ctx.setApplicationData(tad);
						DialogApplicationData peerDat = DialogApplicationData
								.get(peerDialog);
						/*
						 * Record that we queried the answer from the peer
						 * dialog so we can send the Ack along.
						 */
						peerDat.isSdpAnswerPending = true;
						peerDialog.sendRequest(ctx);
						return;
					} else if (rtpSession.isHoldRequest(request)) {
						dat.sendReInviteOnResume = true;
						ReInviteProcessingContinuationData continuationData = new ReInviteProcessingContinuationData(
								requestEvent, dialog, provider,
								serverTransaction, request);
						if (!dat.getBackToBackUserAgent()
								.querySdpFromPeerDialog(requestEvent,
										Operation.SEND_INVITE_TO_MOH_SERVER,
										continuationData)) {
							ProcessInviteContinuationData continuation = new ProcessInviteContinuationData(
									requestEvent);
							Gateway.getTimer().schedule(
									new RequestPendingTimerTask(continuation),
									100);
						}
						return;
					} else {
						/*
						 * Say BYE to MOH server and continue processing.
						 */
						dat.sendReInviteOnResume = false;
						rtpSession.reAssignSessionParameters(request,
								serverTransaction, dialog, null);

						return;
					}
				}

			}

			BackToBackUserAgent btobua = null;

			/*
			 * Look at the Dialog context. The B2BUA structure tracks the call
			 * and is pointed to by the dialog application data.
			 */
			if ((DialogApplicationData) dialog.getApplicationData() != null) {
				btobua = ((DialogApplicationData) dialog.getApplicationData())
						.getBackToBackUserAgent();
			} else if (request.getHeader(ReplacesHeader.NAME) != null) {

				/*
				 * Incoming INVITE has a call id that we dont know about but
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
				BackToBackUserAgent b2bua = DialogApplicationData
						.getBackToBackUserAgent(replacesDialog);
				DialogApplicationData dat = DialogApplicationData
						.get(replacesDialog);
				DialogApplicationData.attach(b2bua, dialog, serverTransaction,
						request);

				Dialog peerDialog = dat.peerDialog;
				logger.debug("replacesDialogState = "
						+ replacesDialog.getState());
				if (replacesDialog.getState() != DialogState.CONFIRMED) {
					dat.peerDialog = null;
				}
				b2bua.pairDialogs(dialog, peerDialog);

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
				String toDomain = null;
				// outbound call. better check for valid account
				ItspAccountInfo account = Gateway.getAccountManager()
						.getAccount(request);
				if (account == null) {
					Response response = ProtocolObjects.messageFactory
							.createResponse(Response.NOT_FOUND, request);
					response
							.setReasonPhrase("Could not find account record for ITSP");
					serverTransaction.sendResponse(response);
					return;

				} else if (account.getState() == AccountState.INVALID) {
					Response response = ProtocolObjects.messageFactory
							.createResponse(Response.BAD_GATEWAY, request);
					response
							.setReasonPhrase("Configuration problem for ITSP - check logs");
					serverTransaction.sendResponse(response);
					return;
				}
				/*
				 * This case occurs when in and outbound proxy are different.
				 */
				btobua.setItspAccount(account);
				toDomain = account.getSipDomain();

				/*
				 * Send the call setup invite out.
				 */
				btobua.sendInviteToItsp(requestEvent, serverTransaction,
						toDomain);
			} else {

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
	 */
	private void processOptions(RequestEvent requestEvent) {
		SipProvider provider = (SipProvider) requestEvent.getSource();
		Request request = requestEvent.getRequest();
		ServerTransaction st = requestEvent.getServerTransaction();

		try {

			Response response = ProtocolObjects.messageFactory.createResponse(
					Response.OK, request);

			ContactHeader contactHeader = null;
			if (provider == Gateway.getLanProvider()) {
				SipUtilities.addAllowHeaders(response);
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
			if (dialog != null) {
				/*
				 * This is an In-dialog request. We add our session description
				 * to the response.
				 */
				DialogApplicationData dat = DialogApplicationData.get(dialog);
				if (dat != null) {
					BackToBackUserAgent b2bua = dat.getBackToBackUserAgent();
					RtpSession sym = null;
					if (provider == Gateway.getLanProvider()) {
						sym = b2bua.getLanRtpSession(dialog);
					} else {
						sym = b2bua.getWanRtpSession(dialog);
					}
					SessionDescription sd = sym.getReceiver()
							.getSessionDescription();
					if (sd != null) {
						response.setContent(sd.toString(),
								ProtocolObjects.headerFactory
										.createContentTypeHeader("application",
												"sdp"));
					}
				}

			}

			/*
			 * If In-Dialog, then the stack will create a server transaction for
			 * you to respond stateufully. Hence that ST should be used to
			 * respond. If out of dialog, then we simply respond statelessly (
			 * no need to create a Server Transaction ).
			 */

			if (st == null) {
				provider.sendResponse(response);
			} else {
				st.sendResponse(response);
			}
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
				throw new GatewayConfigurationException(
						"Check gateway configuration", e);
			}
		}

	}

	/**
	 * Handle inbound REFER request.
	 * 
	 * @param requestEvent
	 */
	private void processRefer(RequestEvent requestEvent) {
		TransactionApplicationData tad = null;

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

			DialogApplicationData dat = (DialogApplicationData) dialog
					.getApplicationData();
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
			 * bridge is configured to do so. Withut ITSP support, blind
			 * transfer can result in no RINGING and dropped calls (if we handle
			 * it locally). Note : REFER is not widely supported by ITSPs
			 */
			Dialog peerDialog = DialogApplicationData.getPeerDialog(dialog);

			DialogApplicationData peerDat = DialogApplicationData
					.get(peerDialog);

			if (Gateway.getBridgeConfiguration().isReferForwarded()
					&& !SipUtilities.isReplacesHeaderPresent(requestEvent
							.getRequest()) && peerDat.isReferAllowed()) {
				btobua.forwardReferToItsp(requestEvent);
				return;
			}

			/*
			 * Re-INVITE the refer Target.
			 */
			if (Gateway.isReInviteSupported()) {
				/*
				 * The ITSP supports re-invite. Send him a Re-INVITE to solicit
				 * an offer. So we can determine what Codec he supports.
				 */
				Request inviteRequest = btobua
						.createInviteFromReferRequest(requestEvent);

				ReferInviteToSipxProxyContinuationData continuation = new ReferInviteToSipxProxyContinuationData(
						inviteRequest, requestEvent);

				if (!btobua.querySdpFromPeerDialog(requestEvent,
						Operation.REFER_INVITE_TO_SIPX_PROXY, continuation)) {
					Gateway.getTimer().schedule(
							new RequestPendingTimerTask(continuation), 100);
				}
			} else {
				/*
				 * Sdp query Re-INIVTE is not supported directly send an INVITE
				 * to the target.
				 */
				Request inviteRequest = btobua
						.createInviteFromReferRequest(requestEvent);
				btobua
						.referInviteToSipxProxy(inviteRequest, requestEvent,
								null);
			}

		} catch (ParseException ex) {
			// This should never happen
			logger.fatal("Internal error constructing message ", ex);
			throw new RuntimeException("Internal error", ex);

		} catch (InvalidArgumentException ex) {
			logger.fatal("Internal error -- invalid argument", ex);
			throw new RuntimeException("Internal error", ex);
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
	 * @param requestEvent -- the ACK request event.
	 */
	private void processAck(RequestEvent requestEvent) {
		try {
			BackToBackUserAgent btobua = DialogApplicationData
					.getBackToBackUserAgent(requestEvent.getDialog());

			if (btobua == null) {
				logger.debug("Could not find B2BUA -- not forwarding ACK ");
				return;
			}
			DialogApplicationData dad = (DialogApplicationData) requestEvent
					.getDialog().getApplicationData();

			Dialog dialog = dad.peerDialog;

			if (dialog == null) {
				logger
						.debug("Could not find peer dialog -- not forwarding ACK!");
				return;
			}

			/*
			 * Forward the ACK if we have not already done so.
			 */

			Request inboundAck = requestEvent.getRequest();

			DialogApplicationData dat = (DialogApplicationData) dialog
					.getApplicationData();

			if (dat != null
					&& dialog.getState() == DialogState.CONFIRMED
					&& dat.lastResponse != null
					&& dat.lastResponse.getStatusCode() == Response.OK
					&& ((CSeqHeader) dat.lastResponse
							.getHeader(CSeqHeader.NAME)).getMethod().equals(
							Request.INVITE)) {
				logger.debug("createAck: " + dialog);

				/*
				 * This case happens in loopback calls. We can query sdp from a
				 * peer that is in the pbx.
				 */

				Request ack = null;

				if (inboundAck.getContentLength().getContentLength() != 0) {
					/*
					 * ACK had a content length so we extract the sdp answer, we
					 * re-write it and forward it.
					 */
					ack = dialog.createAck(SipUtilities
							.getSeqNumber(dat.lastResponse));

					ContentTypeHeader cth = ProtocolObjects.headerFactory
							.createContentTypeHeader("application", "sdp");
					SessionDescription sd = SipUtilities
							.getSessionDescription(inboundAck);
					SipUtilities.incrementSessionVersion(sd);
					dat.getRtpSession().getReceiver().setSessionDescription(sd);
					/*
					 * HACK ALERT Some ITPSs do not like sendonly so make sure
					 * it is sendrecv
					 */
					SipUtilities.setDuplexity(sd, "sendrecv");
					ack.setContent(sd.toString(), cth);
				} else {
					/*
					 * inbound ack had no sdp answer. so we just replay the old
					 * sdp back. This ACK came back as a result of codec
					 * negotiation failure.
					 */
					if (dat.isSdpAnswerPending && dad.mohCodecNegotiationFailed) {
						logger
								.debug("sdpAnswer is pending and none is in ACK -- replay old sdp");
						ack = dialog.createAck(SipUtilities
								.getSeqNumber(dat.lastResponse));

						ContentTypeHeader cth = ProtocolObjects.headerFactory
								.createContentTypeHeader("application", "sdp");

						SessionDescription sd = dat.getRtpSession()
								.getReceiver().getSessionDescription();

						SipUtilities.setDuplexity(sd, "sendrecv");
						ack.setContent(sd.toString(), cth);
						dad.mohCodecNegotiationFailed = false;
					} else if (dat.isSdpAnswerPending) {
						/*
						 * The content length is 0. There is no answer but the
						 * other side expects one. Just silently return.
						 */
						return;
					} else {
						/*
						 * There is no answer and no last response and the other
						 * side does NOT expect one.
						 */
						ack = dialog.createAck(SipUtilities
								.getSeqNumber(dat.lastResponse));
					}
				}

				DialogApplicationData.get(dialog).recordLastAckTime();

				dialog.sendAck(ack);

				/*
				 * Setting this to null here handles the case of Re-invitations.
				 */
				dat.lastResponse = null;

				/*
				 * Set the pending flag to false.
				 */
				dat.isSdpAnswerPending = false;
			}

		} catch (Exception ex) {
			logger.error("Problem sending ack ", ex);
		}

	}

	/**
	 * Processes an INCOMING CANCEL.
	 * 
	 * @param requestEvent -- the inbound CANCEL.
	 * 
	 */
	private void processCancel(RequestEvent requestEvent) {

		Dialog dialog = requestEvent.getDialog();
		BackToBackUserAgent btobua = DialogApplicationData
				.getBackToBackUserAgent(requestEvent.getDialog());

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
			TransactionApplicationData tad = (TransactionApplicationData) inviteServerTransaction
					.getApplicationData();
			if (tad == null) {
				logger
						.error("No transaction application context state found -- returning");
				return;
			}
			ClientTransaction ct = tad.getClientTransaction();
			ItspAccountInfo itspAccount = btobua.getItspAccountInfo();
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
				DialogApplicationData dialogApplicationData = (DialogApplicationData) dialog
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
			BackToBackUserAgent b2bua = DialogApplicationData
					.getBackToBackUserAgent(requestEvent.getDialog());

			if (requestEvent.getServerTransaction() != null) {
				logger
						.debug("serverTransaction Not found -- stray request -- discarding ");
			}

			TransactionApplicationData tad = TransactionApplicationData
					.get(requestEvent.getServerTransaction());

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

	/**
	 * Sends an SDP Offer to the peer of this dialog.
	 * 
	 * @param response
	 * @param dialog
	 * @throws Exception
	 */
	private void sendSdpOffer(Response response, Dialog dialog)
			throws Exception {
		DialogApplicationData dat = (DialogApplicationData) dialog
				.getApplicationData();
		BackToBackUserAgent b2bua = dat.getBackToBackUserAgent();
		Dialog peerDialog = DialogApplicationData.getPeerDialog(dialog);
		DialogApplicationData peerDat = (DialogApplicationData) peerDialog
				.getApplicationData();
		if (logger.isDebugEnabled()) {
			logger.debug("sendSdpOffer : peerDialog = " + peerDialog
					+ " peerDialogApplicationData = " + peerDat
					+ "\nlastResponse = " + peerDat.lastResponse);
		}

		if (!peerDat.isSdpOfferPending) {
			logger
					.warn("This method should not be called with isSdpOfferPending off");
			return;
		}
		peerDat.isSdpOfferPending = false;

		b2bua.sendByeToMohServer();

		/*
		 * Create a new INVITE to send to the ITSP.
		 */

		if (response.getContentLength().getContentLength() != 0) {
			/*
			 * Possibly filter the outbound SDP ( if user sets up to do so ).
			 */
			SessionDescription sd = SipUtilities.cleanSessionDescription(
					SipUtilities.getSessionDescription(response), Gateway
							.getCodecName());

			Request sdpOfferInvite = peerDialog.createRequest(Request.INVITE);

			/*
			 * Got a Response to our SDP query. Shuffle to the other end.
			 */

			if (((DialogExt) dialog).getSipProvider() == Gateway
					.getLanProvider()) {
				/*
				 * We did a SDP query. So we need to put an SDP Answer in the
				 * response.
				 */

				b2bua.getWanRtpSession(peerDialog).getTransmitter().setOnHold(
						false);

				b2bua.getWanRtpSession(peerDialog).getReceiver()
						.setSessionDescription(sd);

				SipUtilities.incrementSessionVersion(sd);

				SipUtilities.addWanAllowHeaders(sdpOfferInvite);
				sdpOfferInvite.removeHeader(SupportedHeader.NAME);

			} else {

				b2bua.getLanRtpSession(peerDialog).getTransmitter().setOnHold(
						false);

				b2bua.getLanRtpSession(peerDialog).getReceiver()
						.setSessionDescription(sd);

				SipUtilities.incrementSessionVersion(sd);

				SipUtilities.addAllowHeaders(sdpOfferInvite);
				sdpOfferInvite.setHeader(ProtocolObjects.headerFactory
						.createSupportedHeader("replaces"));

			}

			sdpOfferInvite.setContent(sd.toString(),
					ProtocolObjects.headerFactory.createContentTypeHeader(
							"application", "sdp"));

			/*
			 * Set global addressing in SDP offer if needed.
			 */
			if (DialogApplicationData.get(peerDialog).getItspInfo() == null
					|| DialogApplicationData.get(peerDialog).getItspInfo().isGlobalAddressingUsed()) {
				SipUtilities.setGlobalAddresses(sdpOfferInvite);
			}

			ClientTransaction ctx = ((DialogExt) peerDialog).getSipProvider()
					.getNewClientTransaction(sdpOfferInvite);

			TransactionApplicationData.attach(ctx, Operation.SEND_SDP_RE_OFFER);

			peerDialog.sendRequest(ctx);

		}

	}

	/**
	 * Sends an SDP answer to the peer of this dialog.
	 * 
	 * @param response
	 *            - response from which we are going to extract the SDP answer
	 * @param dialog
	 *            -- dialog for the interaction
	 * 
	 * @throws Exception
	 *             - if there was a problem extacting sdp or sending ACK
	 */
	private void sendSdpAnswerInAck(Response response, Dialog dialog)
			throws Exception {

		DialogApplicationData dat = (DialogApplicationData) dialog
				.getApplicationData();
		BackToBackUserAgent b2bua = dat.getBackToBackUserAgent();
		Dialog peerDialog = DialogApplicationData.getPeerDialog(dialog);
		DialogApplicationData peerDialogApplicationData = (DialogApplicationData) peerDialog
				.getApplicationData();
		if (logger.isDebugEnabled()) {
			logger.debug("sendSdpAnswerInAck : peerDialog = " + peerDialog
					+ " peerDialogApplicationData = "
					+ peerDialogApplicationData + "\nlastResponse = "
					+ peerDialogApplicationData.lastResponse);
		}

		if (!peerDialogApplicationData.isSdpAnswerPending) {
			logger
					.warn("This method should not be called with isSdpAnswerPending off");
			return;
		}
		peerDialogApplicationData.isSdpAnswerPending = false;

		if (response.getContentLength().getContentLength() != 0) {

			SessionDescription answerSessionDescription = SipUtilities
					.cleanSessionDescription(SipUtilities
							.getSessionDescription(response), Gateway
							.getCodecName());
			/*
			 * Get the codecs in the answer.
			 */
			HashSet<Integer> answerCodecs = SipUtilities
					.getCodecNumbers(answerSessionDescription);

			/*
			 * Get the transmitter session description for the peer. This is
			 * either our old answer or our old offer.
			 */
			SessionDescription transmitterSd = b2bua
					.getTransmitterSd(peerDialog);
			/*
			 * Extract the codec numbers previously offered.
			 */
			HashSet<Integer> transmitterCodecs = SipUtilities
					.getCodecNumbers(transmitterSd);

			/*
			 * The session description to send back in the ACK.
			 */
			SessionDescription ackSd = null;
			/*
			 * Could not find a codec match. We do not want to drop the call in
			 * this case, so just fake it and send the original answer back.
			 */

			if (answerCodecs.size() == 0) {
				/*
				 * Got a Response to our SDP query. Shuffle to the other end.
				 */

				if (((DialogExt) dialog).getSipProvider() == Gateway
						.getLanProvider()) {
					/*
					 * We did a SDP query. So we need to put an SDP Answer in
					 * the response. Retrieve the previously offered session
					 * description.
					 */

					ackSd = b2bua.getWanRtpSession(peerDialog).getReceiver()
							.getSessionDescription();

					/*
					 * Only pick the codecs that the other side will support.
					 */
					SipUtilities.cleanSessionDescription(ackSd,
							transmitterCodecs);

					b2bua.getWanRtpSession(peerDialog).getTransmitter()
							.setOnHold(false);

				} else {
					/*
					 * Get the codec numbers we offered initially.
					 */
					ackSd = b2bua.getLanRtpSession(peerDialog).getReceiver()
							.getSessionDescription();

					/*
					 * Only pick the codecs that the other side will support.
					 */
					SipUtilities.cleanSessionDescription(ackSd,
							transmitterCodecs);

					b2bua.getLanRtpSession(peerDialog).getTransmitter()
							.setOnHold(false);

				}
			} else {
				/*
				 * Got a Response to our SDP query. Shuffle to the other end.
				 * Note that we have to carefully replay a paired down session
				 * description from the original offer -- intersecting it with
				 * the original offer.
				 */

				if (((DialogExt) dialog).getSipProvider() == Gateway
						.getLanProvider()) {

					ackSd = answerSessionDescription;

					/*
					 * Fix up the ports.
					 */
					b2bua.getWanRtpSession(peerDialog).getReceiver()
							.setSessionDescription(ackSd);

					b2bua.getWanRtpSession(peerDialog).getTransmitter()
							.setOnHold(false);

					SipUtilities.incrementSessionVersion(ackSd);

				} else {

					ackSd = answerSessionDescription;

					b2bua.getLanRtpSession(peerDialog).getTransmitter()
							.setOnHold(false);

					/*
					 * Get the codec numbers we offered initially.
					 */
					b2bua.getLanRtpSession(peerDialog).getReceiver()
							.setSessionDescription(ackSd);

					SipUtilities.incrementSessionVersion(ackSd);
				}
			}

			/*
			 * HACK ALERT -- some ITSPs look at sendonly and start playing their
			 * own MOH. This hack is to get around that nasty behavior.
			 */
			if (SipUtilities
					.getSessionDescriptionMediaAttributeDuplexity(ackSd) != null
					&& SipUtilities
							.getSessionDescriptionMediaAttributeDuplexity(ackSd)
							.equals("sendonly")) {
				SipUtilities.setDuplexity(ackSd, "sendrecv");
			}
			Request ackRequest = peerDialog.createAck(SipUtilities
					.getSeqNumber(peerDialogApplicationData.lastResponse));
			/*
			 * Consume the last response.
			 */
			peerDialogApplicationData.lastResponse = null;

			/*
			 * Send the SDP answer in an ACK.
			 */
			if (Gateway.isReInviteSupported()) {
				ackRequest.setContent(ackSd.toString(),
						ProtocolObjects.headerFactory.createContentTypeHeader(
								"application", "sdp"));
				peerDialog.sendAck(ackRequest);
			}

		} else {
			logger.error("ERROR  0 contentLength ");
		}

	}

	/**
	 * Process an INVITE response.
	 * 
	 * @param responseEvent
	 *            -- the response event.
	 */
	private void processInviteResponse(ResponseEvent responseEvent) {

		ServerTransaction serverTransaction = null;
		BackToBackUserAgent b2bua = null;

		Response response = responseEvent.getResponse();
		logger.debug("processInviteResponse : "
				+ ((SIPResponse) response).getFirstLine());

		Dialog dialog = responseEvent.getDialog();

		try {

			if (responseEvent.getClientTransaction() == null) {
				logger
						.debug("Could not find client transaction -- must be stray response.");
				if (response.getStatusCode() == 200 && dialog != null) {
					Request ack = dialog.createAck(((CSeqHeader) response
							.getHeader(CSeqHeader.NAME)).getSeqNumber());
					DialogApplicationData.get(dialog).recordLastAckTime();
					dialog.sendAck(ack);

				}
				return;
			} else if (((TransactionApplicationData) responseEvent
					.getClientTransaction().getApplicationData()).operation == Operation.SESSION_TIMER
					&& dialog != null) {
				if (response.getStatusCode() == 200) {
					Request ack = dialog.createAck(((CSeqHeader) response
							.getHeader(CSeqHeader.NAME)).getSeqNumber());
					DialogApplicationData.get(dialog).recordLastAckTime();
					dialog.sendAck(ack);

				} else if (response.getStatusCode() > 200) {
					b2bua = DialogApplicationData.get(dialog)
							.getBackToBackUserAgent();
					b2bua.tearDown(ProtocolObjects.headerFactory
							.createWarningHeader("sipxbridge", 104,
									"Session timer failure"));
				}
				return;

			}
		} catch (Exception ex) {
			logger.error("Unexpected error sending ACK for 200 OK", ex);
			return;
		}

		DialogApplicationData dat = (DialogApplicationData) dialog
				.getApplicationData();
		if (dat == null) {
			logger
					.error("Could not find DialogApplicationData -- dropping the response");
			try {
				if (response.getStatusCode() == 200) {
					Request ack = dialog.createAck(((CSeqHeader) response
							.getHeader(CSeqHeader.NAME)).getSeqNumber());
					DialogApplicationData.get(dialog).recordLastAckTime();
					dialog.sendAck(ack);

				}

				return;
			} catch (Exception ex) {
				logger.error("Unexpected error sending ACK for 200 OK");
				return;
			}

		}

		b2bua = dat.getBackToBackUserAgent();
		if (b2bua == null) {
			logger
					.fatal("Could not find a BackToBackUA -- dropping the response");
			throw new RuntimeException(
					"Could not find a B2BUA for this response : " + response);
		}

		try {

			if (response.getStatusCode() == Response.TRYING) {
				/*
				 * We store away our outgoing sdp offer in the application data
				 * of the client tx.
				 */
				TransactionApplicationData tad = (TransactionApplicationData) responseEvent
						.getClientTransaction().getApplicationData();
				if (tad.operation == Operation.REFER_INVITE_TO_SIPX_PROXY
						|| tad.operation == Operation.SPIRAL_BLIND_TRANSFER_INVITE_TO_ITSP) {
					Dialog referDialog = tad.referingDialog;
					Request referRequest = tad.referRequest;
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
				TransactionApplicationData tad = (TransactionApplicationData) responseEvent
						.getClientTransaction().getApplicationData();

				logger.debug("Operation = " + tad.operation);

				/*
				 * The TransactionApplicationData operator will indicate what
				 * the OK is for.
				 */
				if (tad.operation == Operation.SEND_SDP_RE_OFFER) {

					if (response.getStatusCode() == 200) {
						RtpSession wanRtpSession = b2bua
								.getWanRtpSession(dialog);
						SessionDescription sd = SipUtilities
								.getSessionDescription(response);
						int port = SipUtilities
								.getSessionDescriptionMediaPort(sd);
						String host = SipUtilities
								.getSessionDescriptionMediaIpAddress(sd);
						wanRtpSession.getTransmitter().setIpAddressAndPort(
								host, port);
						wanRtpSession.getTransmitter().setOnHold(false);
						long seqno = SipUtilities.getSeqNumber(response);
						Request ack = dialog.createAck(seqno);
						dialog.sendAck(ack);
					}

					return;

				} else if (tad.operation == Operation.QUERY_SDP_FROM_PEER_DIALOG
						&& response.getStatusCode() == 200) {
					Operation operation = tad.continuationOperation;
					logger.debug("continuationOperation = " + operation);

					if (response.getContentLength().getContentLength() == 0) {
						logger
								.warn("PROTOCOL ERROR -- Expecting a content length != 0. Re-use previous SDP answer ");

					}

					/*
					 * This tries to compensate for the protocol error. Some
					 * ITSPs do not properly handle sdp query operation. They
					 * return back a 0 length sdp answer. In this case, we
					 * re-use the previously sent sdp answer.
					 */
					SessionDescription responseSessionDescription = response
							.getContentLength().getContentLength() != 0 ? SipUtilities
							.getSessionDescription(response)
							: b2bua.getLanRtpSession(
									tad.continuationData.getDialog())
									.getReceiver().getSessionDescription();

					dat.lastResponse = response;
					if (operation == null) {
						ServerTransaction st = tad.getServerTransaction();
						Request serverRequest = st.getRequest();
						Response newResponse = ProtocolObjects.messageFactory
								.createResponse(Response.OK, serverRequest);
						Dialog peerDialog = dat.peerDialog;
						RtpSession wanRtpSession = b2bua
								.getWanRtpSession(peerDialog);
						wanRtpSession.getReceiver().setSessionDescription(
								responseSessionDescription);

						SipProvider wanProvider = (SipProvider) ((TransactionExt) st)
								.getSipProvider();

						ContactHeader contactHeader = SipUtilities
								.createContactHeader(wanProvider, dat
										.getItspInfo());
						ContentTypeHeader cth = ProtocolObjects.headerFactory
								.createContentTypeHeader("application", "sdp");

						// SipUtilities.incrementSessionVersion(sd);

						newResponse.setContent(responseSessionDescription
								.toString(), cth);
						newResponse.setHeader(contactHeader);
						dat.isSdpAnswerPending = true;
						st.sendResponse(newResponse);
					} else if (operation == Operation.REFER_INVITE_TO_SIPX_PROXY) {

						/*
						 * ACK the query operation with the previous response.
						 * This prevents the Dialog from timing out while the
						 * transfer target waits for phone pickup.
						 */
						long cseq = SipUtilities.getSeqNumber(response);
						if (dialog.getState() != DialogState.TERMINATED) {
							/*
							 * If we do not support MOH or if the park server
							 * codecs are not supported in the answer, ACK right
							 * away. Otherwise Send an INVITE with the answer to
							 * the Park Server and concurrently send another
							 * invite to the phone. The Park server will play
							 * music when the phone is ringing ( simulates
							 * RINGING ).
							 */
							ReferInviteToSipxProxyContinuationData continuation = (ReferInviteToSipxProxyContinuationData) tad.continuationData;

							if (!Gateway.getBridgeConfiguration()
									.isMusicOnHoldSupportEnabled()
									|| !SipUtilities.isCodecSupported(
											responseSessionDescription, Gateway
													.getParkServerCodecs())
									|| (b2bua.getMusicOnHoldDialog() != null && b2bua
											.getMusicOnHoldDialog().getState() != DialogState.TERMINATED)) {

								Request ack = dialog.createAck(cseq);

								SessionDescription sessionDescription = SipUtilities
										.getSessionDescription(response);
								HashSet<Integer> codecs = SipUtilities
										.getCodecNumbers(sessionDescription);

								logger.debug("Codecs " + codecs);

								SessionDescription ackSd = dat.getRtpSession()
										.getReceiver().getSessionDescription();
								/*
								 * Restrict the answer to the set of codecs in
								 * the offer.
								 */
								SipUtilities.cleanSessionDescription(ackSd,
										codecs);

								SipUtilities.setSessionDescription(ack, ackSd);

								/*
								 * Send an ACK back to the WAN side and replay
								 * the same Session description as before. This
								 * completes the handshake so the Dialog will
								 * not time out.
								 */
								DialogApplicationData.get(dialog).lastResponse = null;
								DialogApplicationData.get(dialog).isSdpAnswerPending = false;
								DialogApplicationData.get(dialog).isSdpOfferPending = true;

								dialog.sendAck(ack);
							} else {
								DialogApplicationData.get(dialog).lastResponse = response;
								DialogApplicationData.get(dialog).isSdpOfferPending = true;
								DialogApplicationData.get(dialog).isSdpAnswerPending = true;
								b2bua
										.getLanRtpSession(
												continuation.getDialog())
										.getReceiver().setSessionDescription(
												responseSessionDescription);
								ClientTransaction ctx = b2bua
										.createClientTxToMohServer(responseSessionDescription);
								DialogApplicationData.get(ctx.getDialog()).peerDialog = dialog;
								ctx.sendRequest();

							}

							SessionDescription clonedSessionDescription = SipUtilities
									.cloneSessionDescription(responseSessionDescription);

							b2bua.getWanRtpSession(dialog).getTransmitter()
									.setSessionDescription(
											clonedSessionDescription, true);

							b2bua.getLanRtpSession(continuation.getDialog())
									.getReceiver().setSessionDescription(
											responseSessionDescription);

							b2bua.referInviteToSipxProxy(continuation
									.getRequest(), continuation
									.getRequestEvent(),
									responseSessionDescription);
						}

					} else if (operation == Operation.SEND_INVITE_TO_MOH_SERVER) {
						/*
						 * This is a query for MOH server. Lets see if he
						 * returned a codec that park server handle in the
						 * query. If the ITSP does not return a codec that the
						 * park server supports, we just ACK with the previously
						 * negotiated codec. MOH will not play in this case.
						 * Note that MOH answers right away so we can forward
						 * the request.
						 */
						if (response.getContentLength().getContentLength() == 0) {
							logger
									.warn("PROTOCOL ERROR -- Expecting a content length != 0. Re-use previous SDP answer ");
							dat.isSdpAnswerPending = false;
							long cseq = SipUtilities.getSeqNumber(response);
							Request ack = dialog.createAck(cseq);

							dialog.sendAck(ack);
							return;

						}

						/*
						 * Does our park server support the codecs in the SDP
						 * offer? If not, we just reply back with a suitable ACK
						 * right away.
						 */
						if (!SipUtilities.isCodecSupported(
								responseSessionDescription, Gateway
										.getParkServerCodecs())) {
							/*
							 * If codec is not supported by park server then we
							 * simply do not forward the answer to the park
							 * server in an INIVTE. We just replay the old
							 * respose (Example: this case happens for AT&T
							 * HIPCS ).
							 */
							long cseq = SipUtilities.getSeqNumber(response);
							Request ack = dialog.createAck(cseq);

							SessionDescription ackSd = dat.getRtpSession()
									.getReceiver().getSessionDescription();
							/*
							 * Limit the Answer to the codec set found in the
							 * offer. Note that the OFFER is in the INBOUND OK.
							 */
							HashSet<Integer> codecs = SipUtilities
									.getCodecNumbers(responseSessionDescription);
							SipUtilities.cleanSessionDescription(ackSd, codecs);
							SipUtilities.setSessionDescription(ack, ackSd);

							/*
							 * Now reply back to the original Transaction and
							 * put the WAN side on hold. Note tha this case MOH
							 * will not play.
							 */
							ReInviteProcessingContinuationData continuation = (ReInviteProcessingContinuationData) tad.continuationData;
							DialogApplicationData.get(continuation.dialog).mohCodecNegotiationFailed = true;

							/*
							 * Send an ACK back to the WAN side and replay the
							 * same Session description as before.
							 */
							DialogApplicationData.get(dialog).lastResponse = null;
							DialogApplicationData.get(dialog).isSdpAnswerPending = false;
							dialog.sendAck(ack);
							Request request = continuation.serverTransaction
									.getRequest();
							Response newResponse = ProtocolObjects.messageFactory
									.createResponse(Response.OK, request);

							/*
							 * Replay the response to the LAN side locally.
							 */
							RtpSession lanRtpSession = b2bua
									.getLanRtpSession(continuation.dialog);

							SipUtilities.setDuplexity(lanRtpSession
									.getReceiver().getSessionDescription(),
									"recvonly");
							SipUtilities.incrementSessionVersion(lanRtpSession
									.getReceiver().getSessionDescription());
							RtpSession wanRtpSession = dat.rtpSession;
							/*
							 * Put the rtp session on hold.
							 */
							wanRtpSession.getTransmitter().setOnHold(true);

							SupportedHeader sh = ProtocolObjects.headerFactory
									.createSupportedHeader("replaces");
							newResponse.setHeader(sh);
							SessionDescription newDescription = lanRtpSession
									.getReceiver().getSessionDescription();
							if (newDescription != null) {
								newResponse.setContent(newDescription,
										ProtocolObjects.headerFactory
												.createContentTypeHeader(
														"application", "sdp"));
							}

							ToHeader toHeader = (ToHeader) request
									.getHeader(ToHeader.NAME);
							String userName = ((SipURI) toHeader.getAddress()
									.getURI()).getUser();
							ContactHeader contactHeader = SipUtilities
									.createContactHeader(userName,
											((DialogExt) continuation.dialog)
													.getSipProvider());
							newResponse.setHeader(contactHeader);
							response
									.setReasonPhrase("RTP Session Parameters Changed");
							WarningHeader warningHeader = ProtocolObjects.headerFactory
									.createWarningHeader("SipXbridge", 101,
											"Codec negotion for MOH failed.");
							response.setHeader(warningHeader);
							continuation.serverTransaction
									.sendResponse(newResponse);

							return;
						} else {
							ReInviteProcessingContinuationData continuation = (ReInviteProcessingContinuationData) tad.continuationData;

							SessionDescription clonedSd = SipUtilities
									.cloneSessionDescription(responseSessionDescription);
							/*
							 * Set the session description on the wan side.
							 */
							b2bua.getWanRtpSession(dialog).getTransmitter()
									.setSessionDescription(
											responseSessionDescription, false);

							/*
							 * Update the ports of the SD to forward to to the
							 * lan side.
							 */
							b2bua.getLanRtpSession(continuation.dialog)
									.getReceiver().setSessionDescription(
											clonedSd);

							this.adjustSessionParameters(
									continuation.serverTransaction,
									continuation.request, continuation.dialog,
									continuation.provider, dialog);
						}

					}

				} else if (tad.operation == Operation.SEND_INVITE_TO_ITSP
						|| tad.operation == Operation.SEND_INVITE_TO_SIPX_PROXY) {

					/*
					 * Store away our incoming response - get ready for ACKL
					 */
					dat.lastResponse = response;
					dat.setBackToBackUserAgent(b2bua);

					dialog.setApplicationData(dat);

					/*
					 * Now send the respose to the server side of the
					 * transaction.
					 */
					serverTransaction = tad.getServerTransaction();
					Response newResponse = ProtocolObjects.messageFactory
							.createResponse(response.getStatusCode(),
									serverTransaction.getRequest());
					SupportedHeader sh = ProtocolObjects.headerFactory
							.createSupportedHeader("replaces");

					newResponse.setHeader(sh);

					String toTag = tad.toTag;
					if (toTag == null) {
						toTag = Integer.toString(Math.abs(new Random()
								.nextInt()));
						tad.toTag = toTag;
					}

					ToHeader toHeader = (ToHeader) tad.getServerTransaction()
							.getRequest().getHeader(ToHeader.NAME);

					String user = ((SipURI) toHeader.getAddress().getURI())
							.getUser();
					ContactHeader contactHeader = null;

					/*
					 * Set the contact address for the OK. Note that ITSP may
					 * want global addressing.
					 */
					if (tad.operation == Operation.SEND_INVITE_TO_ITSP) {
						contactHeader = SipUtilities.createContactHeader(user,
								tad.serverTransactionProvider);
					} else {
						contactHeader = SipUtilities.createContactHeader(
								tad.serverTransactionProvider,
								tad.itspAccountInfo);
					}

					newResponse.setHeader(contactHeader);
					ToHeader newToHeader = (ToHeader) newResponse
							.getHeader(ToHeader.NAME);
					newToHeader.setTag(toTag);

					/*
					 * Fix up the media session using the port in the incoming
					 * sdp answer.
					 */
					ContentTypeHeader cth = (ContentTypeHeader) response
							.getHeader(ContentTypeHeader.NAME);

					SessionDescription newSd = null;
					if (response.getRawContent() != null
							&& cth.getContentType().equalsIgnoreCase(
									"application")
							&& cth.getContentSubType().equalsIgnoreCase("sdp")) {
						/*
						 * The incoming media session.
						 */
						SessionDescription sessionDescription = SdpFactory
								.getInstance().createSessionDescription(
										new String(response.getRawContent()));
						if (logger.isDebugEnabled()) {
							logger.debug("SessionDescription = "
									+ new String(response.getRawContent()));
						}

						DialogApplicationData dialogApplicationData = (DialogApplicationData) dialog
								.getApplicationData();

						RtpSession rtpSession = dialogApplicationData
								.getRtpSession();
						RtpTransmitterEndpoint hisEndpoint = null;
						if (rtpSession != null) {
							hisEndpoint = rtpSession.getTransmitter();
						}

						if (hisEndpoint == null) {
							hisEndpoint = new RtpTransmitterEndpoint(
									rtpSession, b2bua.getSymmitronClient());
						}

						tad.outgoingSession.setTransmitter(hisEndpoint);

						KeepaliveMethod keepaliveMethod;

						if (tad.operation == Operation.SEND_INVITE_TO_ITSP) {
							keepaliveMethod = tad.itspAccountInfo
									.getRtpKeepaliveMethod();
						} else {
							keepaliveMethod = KeepaliveMethod.NONE;
						}
						hisEndpoint.setKeepAliveMethod(keepaliveMethod);

						hisEndpoint.setSessionDescription(sessionDescription,
								false);

						RtpReceiverEndpoint incomingEndpoint = tad.incomingSession
								.getReceiver();
						newSd = SdpFactory.getInstance()
								.createSessionDescription(
										new String(response.getRawContent()));
						incomingEndpoint.setSessionDescription(newSd);

						newResponse.setContent(newSd.toString(), cth);

						tad.backToBackUa.getRtpBridge().start();

					} else if (response.getRawContent() != null) {
						// Cannot recognize header.
						logger.warn("content type is not application/sdp");
						String body = new String(response.getRawContent());
						WarningHeader warningHeader = ProtocolObjects.headerFactory
								.createWarningHeader("sipxbridge", 106,
										"Could not recognize content type");
						newResponse.setHeader(warningHeader);
						newResponse.setContent(body, cth);
					}

					serverTransaction.sendResponse(newResponse);
				} else if (tad.operation == Operation.REFER_INVITE_TO_SIPX_PROXY
						|| tad.operation == Operation.SPIRAL_BLIND_TRANSFER_INVITE_TO_ITSP) {

					/*
					 * This is the case of Refer redirection. In this case, we
					 * have already established a call leg with transfer agent.
					 * We already have a RTP session established with the
					 * transfer agent. We need to redirect the outbound RTP
					 * stream to the transfer target. To do this, we fix up the
					 * media session using the port in the incoming sdp answer.
					 */
					ContentTypeHeader cth = (ContentTypeHeader) response
							.getHeader(ContentTypeHeader.NAME);
					Dialog referDialog = tad.referingDialog;
					Request referRequest = tad.referRequest;
					Dialog peerDialog = DialogApplicationData
							.getPeerDialog(dialog);
					DialogApplicationData peerDat = DialogApplicationData
							.get(peerDialog);

					if (response.getRawContent() != null
							&& cth.getContentType().equalsIgnoreCase(
									"application")
							&& cth.getContentSubType().equalsIgnoreCase("sdp")) {
						/*
						 * The incoming media session.
						 */

						SessionDescription sessionDescription = SipUtilities
								.getSessionDescription(response);

						RtpSession rtpSession = ((DialogApplicationData) referDialog
								.getApplicationData()).getRtpSession();

						if (rtpSession != null) {

							/*
							 * Note that we are just pointing the transmitter to
							 * another location. The receiver stays as is.
							 */
							rtpSession.getTransmitter().setSessionDescription(
									sessionDescription, false);
							logger.debug("Receiver State : "
									+ rtpSession.getReceiverState());

							/*
							 * Grab the RTP session previously pointed at by the
							 * REFER dialog.
							 */
							b2bua.getRtpBridge().addSym(rtpSession);

							((DialogApplicationData) dialog
									.getApplicationData())
									.setRtpSession(rtpSession);

							/*
							 * Check if we need to forward that response and do
							 * so if needed. see issue 1718
							 */
							if (peerDat.transaction != null
									&& peerDat.transaction instanceof ServerTransaction
									&& peerDat.transaction.getState() != TransactionState.TERMINATED) {

								Request request = ((ServerTransaction) peerDat.transaction)
										.getRequest();
								Response forwardedResponse = ProtocolObjects.messageFactory
										.createResponse(response
												.getStatusCode(), request);
								SipUtilities.setSessionDescription(
										forwardedResponse, sessionDescription);
								ContactHeader contact = SipUtilities
										.createContactHeader(
												((TransactionExt) peerDat.transaction)
														.getSipProvider(),
												peerDat.getItspInfo());
								forwardedResponse.setHeader(contact);
								((ServerTransaction) peerDat.transaction)
										.sendResponse(forwardedResponse);
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
						WarningHeader warningHeader = ProtocolObjects.headerFactory
								.createWarningHeader("sipxbridge", 107,
										"unknown content type encountered");
						dat.getBackToBackUserAgent().tearDown(warningHeader);
						return;
					}

					/*
					 * Got an OK for the INVITE ( that means that somebody
					 * picked up ) so we can hang up the call. We have already
					 * redirected the RTP media to the redirected party at this
					 * point.
					 */

					if (referDialog.getState() == DialogState.CONFIRMED) {
						this.notifyReferDialog(referRequest, referDialog,
								response);
					}

					/*
					 * SDP was returned from the transfer target.
					 */
					if (response.getContentLength().getContentLength() != 0) {
						if (peerDat.isSdpAnswerPending
								&& peerDialog.getState() == DialogState.CONFIRMED) {
							this.sendSdpAnswerInAck(response, dialog);
						} else if (peerDat.isSdpOfferPending) {
							this.sendSdpOffer(response, dialog);

						}
					}

					/*
					 * We directly send ACK.
					 */
					if (response.getStatusCode() == Response.OK) {

						b2bua.addDialog(dialog);
						// Thread.sleep(100);
						Request ackRequest = dialog
								.createAck(((CSeqHeader) response
										.getHeader(CSeqHeader.NAME))
										.getSeqNumber());
						dialog.sendAck(ackRequest);

					}
					/*
					 * If there is a Music on hold dialog -- tear it down
					 */

					if (response.getStatusCode() == Response.OK) {
						b2bua.sendByeToMohServer();
					}

				} else if (tad.operation
						.equals(Operation.SEND_INVITE_TO_MOH_SERVER)) {
					if (response.getStatusCode() == Response.OK) {
						Request ack = dialog.createAck(((CSeqHeader) response
								.getHeader(CSeqHeader.NAME)).getSeqNumber());

						dialog.sendAck(ack);
						Dialog peerDialog = DialogApplicationData
								.getPeerDialog(dialog);
						if (Gateway.isReInviteSupported()
								&& response.getContentLength()
										.getContentLength() != 0
								&& DialogApplicationData.get(peerDialog).isSdpAnswerPending
								&& peerDialog != null) {
							this.sendSdpAnswerInAck(response, dialog);
						}

					}
				} else if (tad.operation.equals(Operation.FORWARD_REINVITE)) {

					/*
					 * Store away our incoming response - get ready for ACKL
					 */
					dat.lastResponse = response;
					dat.setBackToBackUserAgent(b2bua);

					dialog.setApplicationData(dat);

					/*
					 * Now send the respose to the server side of the
					 * transaction.
					 */
					serverTransaction = tad.getServerTransaction();

					/*
					 * If we have a server transaction associated with the
					 * response, we ack when the other side acks.
					 */
					if (serverTransaction != null) {
						Response newResponse = ProtocolObjects.messageFactory
								.createResponse(response.getStatusCode(),
										serverTransaction.getRequest());
						SupportedHeader sh = ProtocolObjects.headerFactory
								.createSupportedHeader("replaces");

						newResponse.setHeader(sh);
						ContactHeader contactHeader = SipUtilities
								.createContactHeader(Gateway.SIPXBRIDGE_USER,
										tad.serverTransactionProvider);
						newResponse.setHeader(contactHeader);
						Dialog peerDialog = DialogApplicationData
								.getPeerDialog(dialog);
						SipProvider peerProvider = ((DialogExt) peerDialog)
								.getSipProvider();
						if (response.getContentLength().getContentLength() != 0) {
							SessionDescription sd = SipUtilities
									.getSessionDescription(response);
							ContentTypeHeader cth = ProtocolObjects.headerFactory
									.createContentTypeHeader("application",
											"sdp");
							RtpSession originalRtpSession = b2bua
									.getLanRtpSession(dialog);
							originalRtpSession.setTransmitterPort(response);

							RtpSession rtpSession = b2bua
									.getLanRtpSession(peerDialog);
							rtpSession.getReceiver().setSessionDescription(sd);
							newResponse.setContent(sd.toString(), cth);

						}
						if (peerProvider != Gateway.getLanProvider()) {
							DialogApplicationData peerDat = DialogApplicationData
									.get(peerDialog);
							if (peerDat.getItspInfo() == null
									|| peerDat.getItspInfo()
											.isGlobalAddressingUsed()) {
								SipUtilities.setGlobalAddress(newResponse);
							}
						}

						serverTransaction.sendResponse(newResponse);
					} else {
						Request ack = dialog.createAck(SipUtilities
								.getSeqNumber(response));
						dialog.sendAck(ack);
					}

				} else if (tad.operation == Operation.HANDLE_SPIRAL_INVITE_WITH_REPLACES) {

					Request ack = dialog.createAck(((CSeqHeader) response
							.getHeader(CSeqHeader.NAME)).getSeqNumber());

					dialog.sendAck(ack);

					this.sendSdpAnswerInAck(response, dialog);

				} else if (tad.operation == Operation.HANDLE_INVITE_WITH_REPLACES) {

					dat.lastResponse = response;
					serverTransaction = tad.getServerTransaction();
					Dialog replacedDialog = tad.replacedDialog;
					Request request = serverTransaction.getRequest();
					SipProvider peerProvider = ((TransactionExt) serverTransaction)
							.getSipProvider();
					ContactHeader contactHeader = SipUtilities
							.createContactHeader(Gateway.SIPXBRIDGE_USER,
									peerProvider);

					Response serverResponse = ProtocolObjects.messageFactory
							.createResponse(response.getStatusCode(), request);
					serverResponse.setHeader(contactHeader);
					if (response.getContentLength().getContentLength() != 0) {
						SessionDescription sdes = SipUtilities
								.getSessionDescription(response);
						RtpSession rtpSession = b2bua
								.getLanRtpSession(replacedDialog);
						rtpSession.getReceiver().setSessionDescription(sdes);
						ContentTypeHeader cth = ProtocolObjects.headerFactory
								.createContentTypeHeader("application", "sdp");
						serverResponse.setContent(sdes.toString(), cth);
					}

					/*
					 * Bid adieu to the replaced dialog if we have not already
					 * done so.
					 */
					if (response.getStatusCode() == Response.OK) {
						Gateway.getTimer().schedule(
								new TearDownReplacedDialogTimerTask(
										replacedDialog), 30 * 1000);
					}

					/*
					 * accept the dialog that replaces this dialog.
					 */
					DialogApplicationData serverDat = DialogApplicationData
							.get(serverTransaction.getDialog());
					serverDat.peerDialog = dialog;
					serverTransaction.sendResponse(serverResponse);
					serverDat.setRtpSession(DialogApplicationData.get(
							replacedDialog).getRtpSession());

					if (replacedDialog.getState() != DialogState.TERMINATED) {
						DialogApplicationData replacedDat = DialogApplicationData
								.get(replacedDialog);
						replacedDat.setRtpSession(null);
						replacedDat.peerDialog = null;
					}

				} else {
					logger
							.fatal("CallControlManager: Unknown Case in if statement ");
				}
			} else if (response.getStatusCode() == Response.REQUEST_PENDING) {
				/*
				 * A glare condition was detected. Start a timer and retry the
				 * operation after timeout later.
				 */
				TransactionApplicationData tad = (TransactionApplicationData) responseEvent
						.getClientTransaction().getApplicationData();
				if (tad.continuationData == null
						|| tad.continuationData.getOperation() != Operation.REFER_INVITE_TO_SIPX_PROXY) {
					logger.warn("Unexpected REQUEST_PENDING");
					b2bua.tearDown();
					return;

				}
				Gateway.getTimer()
						.schedule(
								new RequestPendingTimerTask(
										tad.continuationData), 1000);
			} else if (response.getStatusCode() == Response.INTERVAL_TOO_BRIEF) {
				MinSE minSe = (MinSE) response.getHeader(MinSE.NAME);
				if (minSe != null) {
					dat.setSetExpires(minSe.getExpires());
				}

			} else if (response.getStatusCode() > 200) {

				if (responseEvent.getClientTransaction() == null) {
					logger.warn("null client transaction");
					return;
				}

				logger.debug("Processing ERROR Response "
						+ response.getStatusCode());

				// Processing an error resonse.
				ClientTransaction ct = responseEvent.getClientTransaction();
				TransactionApplicationData tad = (TransactionApplicationData) ct
						.getApplicationData();
				if (tad != null) {
					serverTransaction = tad.getServerTransaction();
					/*
					 * We do not forward back error responses for requests such
					 * as REFER that we are handling locally.
					 */
					if (tad.operation != Operation.REFER_INVITE_TO_SIPX_PROXY) {

						if (serverTransaction != null) {
							if (serverTransaction.getState() != TransactionState.TERMINATED) {
								Request originalRequest = serverTransaction
										.getRequest();
								Response newResponse = ProtocolObjects.messageFactory
										.createResponse(response
												.getStatusCode(),
												originalRequest);
								SupportedHeader sh = ProtocolObjects.headerFactory
										.createSupportedHeader("replaces");
								newResponse.setHeader(sh);
								serverTransaction.sendResponse(newResponse);
							} else {
								logger
										.error("Received an error response after final response sent -- ignoring the response");
							}
						} else {
							b2bua.tearDown();
						}
					} else {
						Dialog referDialog = tad.referingDialog;
						Request referRequest = tad.referRequest;
						if (referDialog != null
								&& referDialog.getState() == DialogState.CONFIRMED) {
							this.notifyReferDialog(referRequest, referDialog,
									response);
						}
						/*
						 * Tear down the call.
						 */
						b2bua.tearDown();
					}
				}
			}

		} catch (ParseException ex) {
			logger.error("Unexpected parse exception", ex);
			throw new RuntimeException("Unexpected exception", ex);
		} catch (InvalidArgumentException ex) {
			logger.error("Unpexpected exception", ex);
			throw new RuntimeException("Unexpected exception", ex);
		} catch (Exception ex) {
			// Some other exception occured during processing of the request.
			logger.error("Exception while processing inbound response ", ex);

			if (serverTransaction != null) {
				CallControlUtilities.sendBadRequestError(serverTransaction, ex);
			}
			if (b2bua != null) {
				try {
					b2bua.tearDown();
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

		Response response = responseEvent.getResponse();
		if (response.getStatusCode() == Response.CALL_OR_TRANSACTION_DOES_NOT_EXIST) {
			Dialog dialog = responseEvent.getClientTransaction().getDialog();
			BackToBackUserAgent b2bua = DialogApplicationData.get(dialog)
					.getBackToBackUserAgent();
			b2bua.tearDown();
		}

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
			SipUtilities.addAllowHeaders(notifyRequest);
			SipProvider referProvider = ((SIPDialog) referDialog)
					.getSipProvider();
			ClientTransaction ctx = referProvider
					.getNewClientTransaction(notifyRequest);
			referDialog.sendRequest(ctx);
		} catch (ParseException ex) {
			logger.error("Unexpected parse exception ", ex);
			throw new RuntimeException("Unexpected parse exception ", ex);
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
			TransactionApplicationData tad = (TransactionApplicationData) ctx
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
					DialogApplicationData dat = DialogApplicationData
							.get(responseEvent.getDialog());
					BackToBackUserAgent b2bua = dat.getBackToBackUserAgent();
					WarningHeader warningHeader = ProtocolObjects.headerFactory
							.createWarningHeader("sipxbridge", 108,
									"Error processing REFER response");
					b2bua.tearDown(warningHeader);

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
			TransactionApplicationData tad = (TransactionApplicationData) ct
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

	void stop() {
		/*
		 * The following code assumes that the sip stack is being used just for
		 * this service.
		 */
		for (Dialog dialog : ((SipStackImpl) ProtocolObjects.sipStack)
				.getDialogs()) {
			dialog.delete();
		}

	}

	/**
	 * The Reset handler for the symmitron.
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
