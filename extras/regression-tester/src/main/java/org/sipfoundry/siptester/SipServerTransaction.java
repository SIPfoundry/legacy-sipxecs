/*
 * Copyright (C) 2010 Avaya, certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 */
package org.sipfoundry.siptester;

import gov.nist.javax.sip.DialogExt;
import gov.nist.javax.sip.message.RequestExt;
import gov.nist.javax.sip.message.ResponseExt;
import gov.nist.javax.sip.message.SIPRequest;
import gov.nist.javax.sip.message.SIPResponse;

import java.util.Collection;
import java.util.HashSet;
import java.util.Hashtable;
import java.util.Iterator;
import java.util.LinkedList;
import java.util.concurrent.ConcurrentSkipListSet;
import java.util.concurrent.TimeUnit;

import javax.sip.Dialog;
import javax.sip.ServerTransaction;
import javax.sip.TransactionState;
import javax.sip.header.RSeqHeader;
import javax.sip.header.RequireHeader;
import javax.sip.header.ViaHeader;
import javax.sip.message.Request;
import javax.sip.message.Response;

import org.apache.log4j.Logger;

public class SipServerTransaction extends SipTransaction implements
		Comparable<SipServerTransaction> {
	private static Logger logger = Logger.getLogger(SipServerTransaction.class);

	private ConcurrentSkipListSet<SipResponse> responses = new ConcurrentSkipListSet<SipResponse>();

	private SipClientTransaction matchingClientTransaction;

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

	private String dialogId;

	private Collection<SipServerTransaction> matchingServerTransactions = new HashSet<SipServerTransaction>();

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
		logger.debug("retransmittedRequest -- ignoring "
				+ sipRequest.getSipRequest().getFirstLine());
	}

	public void addResponse(SipResponse sipResponse) {
		/*
		 * Check to see if this is a reliable provisional response
		 * retransmission.
		 */

		for (SipResponse response : this.responses) {
			ResponseExt prevResponse = response.getSipResponse();
			RSeqHeader prevRseq = (RSeqHeader) prevResponse
					.getHeader(RSeqHeader.NAME);
			ResponseExt currentResponse = sipResponse.getSipResponse();
			RSeqHeader thisRseq = (RSeqHeader) currentResponse
					.getHeader(RSeqHeader.NAME);
			if (prevRseq != null
					&& thisRseq != null
					&& prevRseq.getSequenceNumber() == thisRseq
							.getSequenceNumber()) {
				logger.debug("Duplicate reliable response transmission");
				return;
			}
		}

		logger.debug("adding response " + this.sipRequest.getFrameId()
				+ " frameId = " + sipResponse.getFrameId());
		boolean added = this.responses.add(sipResponse);

		if (!added) {
			logger.debug("response already added");
		}
	}

	/**
	 * Get the dialog ID for this server transaction.
	 * 
	 * @return
	 */
	public String getDialogId() {
		if (dialogId != null) {
			return dialogId;
		} else {
			if (this.endpoint.getTraceEndpoint().getBehavior() == Behavior.PROXY  || 
					this.sipRequest.getSipRequest().getMethod().equals(Request.ACK)) {			
				this.dialogId = SipUtilities.getDialogId(this.sipRequest.getSipRequest(), true, this.endpoint.getTraceEndpoint().getBehavior());
			} else {
				for (SipResponse sipResponse : this.getResponses()) {
					if (sipResponse.getSipResponse().getFromHeader().getTag() != null
							&& sipResponse.getSipResponse().getToHeader().getTag() != null) {						
							this.dialogId = ((SIPResponse) sipResponse
									.getSipResponse()).getDialogId(true);
							break;
					}
				}
			}
			return this.dialogId;
		}
	}

	/**
	 * @param endpoint
	 *            the endpoint to set
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
		logger.debug("setBranch " + this + " branch = " + branch + " Endpoint "
				+ this.endpoint.getPort());
	}

	public String getBranch() {
		logger.debug("SipServerTransaction : " + this + " getBranch : "
				+ this.getSipRequest().getSipRequest().getMethod()
				+ " branch  " + this.branch);
		return this.branch;
	}

	public void setMatchingClientTransaction(
			SipClientTransaction sipClientTransaction) {
		this.matchingClientTransaction = sipClientTransaction;
	}

	public void printServerTransaction() {
		SipTester.getPrintWriter().println("<server-transaction>");
		SipTester.getPrintWriter().println(
				"<transaction-id>" + this.getTransactionId()
						+ "</transaction-id>");
		SipTester.getPrintWriter().println("<frame>" + this.getSipRequest().getFrameId() + "</frame>");

		SipTester.getPrintWriter().println("<sip-request><![CDATA[");
		SipTester.getPrintWriter().print(this.sipRequest.getSipRequest());
		SipTester.getPrintWriter().println("]]></sip-request>");
		SipTester.getPrintWriter().println("<responses>");

		for (SipResponse response : this.responses) {
			SipTester.getPrintWriter().println("<sip-response >");
			SipTester.getPrintWriter().println("<frameId>" + response.getFrameId() + "</frameId>");
			SipTester.getPrintWriter().println("<![CDATA[");
			SipTester.getPrintWriter().println(
					response.getSipResponse() + "]]></sip-response>");
		}
		SipTester.getPrintWriter().println("</responses>");
		SipTester.getPrintWriter().println("</server-transaction>");
	}

	public void sendResponses() {
		int responseToSendFrameId = 0;

		try {
			logger.debug("serverTransactionId " + this.getBranch() + " tid = "
					+ this.getTransactionId());
			RequestExt request = (RequestExt) serverTransaction.getRequest();
			if (responses.isEmpty()) {
				logger.debug("no Responses to SEND ");
			}

			SipResponse finalResponse = this.responses.last();
			Iterator<SipResponse> it = this.responses.iterator();
			while (it.hasNext()) {
				SipResponse nextResponse = it.next();
				nextResponse.waitForPrecondition();
				System.out.println("sendResponse FrameId "
						+ nextResponse.getFrameId()
						+ " request FrameId = "
						+ this.sipRequest.getFrameId()
						+ " method = "
						+ nextResponse.getSipResponse().getCSeqHeader()
								.getMethod() + " statusCode = "
						+ nextResponse.getStatusCode());
				ResponseExt newResponse = SipUtilities.createResponse(endpoint,
						request, nextResponse);
				if (serverTransaction.getState() == TransactionState.TERMINATED) {
					continue;
				}
				if (nextResponse == finalResponse) {
					String correlator = SipUtilities.getCorrelator(newResponse);
					for (SipServerTransaction st : this.matchingServerTransactions) {
						st.setBranch(correlator);
						SipDialog peerDialog = SipTester.getDialog(st
								.getDialogId(), endpoint);
						peerDialog.setPeerDialog(SipTester.getDialog(this
								.getDialogId(), endpoint));
					}
				}
				it.remove();

				Iterator headerIterator = newResponse
						.getHeaders(RequireHeader.NAME);
				boolean isReliableResponse = false;
				while (headerIterator != null && headerIterator.hasNext()) {
					RequireHeader requireHeader = (RequireHeader) headerIterator
							.next();
					if (requireHeader.getOptionTag().equals("100rel")) {
						isReliableResponse = true;
					}
				}
				nextResponse.firePermits();
				
				if (!isReliableResponse
						|| newResponse.getStatusCode() / 100 >= 2) {
					String dialogId = this.getDialogId();
					logger.debug("dialogId = " + dialogId + " request "
							+ this.sipRequest.getFrameId());
					SipDialog sipDialog = SipTester.getDialog(dialogId,
							endpoint);
					sipDialog.setDialog((DialogExt) serverTransaction
							.getDialog());
					if (newResponse.getStatusCode() == 200
							&& newResponse.getCSeqHeader().getMethod().equals(
									Request.INVITE)) {
						sipDialog.setResponseToSend(newResponse);
					}
					responseToSendFrameId = nextResponse.getFrameId();
				    serverTransaction.sendResponse(newResponse);
				} else {
					if (serverTransaction.getDialog() != null) {
						serverTransaction.getDialog()
								.sendReliableProvisionalResponse(newResponse);
					} else {
						serverTransaction.sendResponse(newResponse);
					}
					new Thread(new Runnable() {

						@Override
						public void run() {
							try {
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
			System.out.println("Error sending response "
					+ responseToSendFrameId);
			SipTester.fail("Unexpected exception ", ex);

		}

	}

	public ConcurrentSkipListSet<SipResponse> getResponses() {
		return this.responses;
	}

	public void setServerTransaction(ServerTransaction serverTransaction) {
		this.serverTransaction = serverTransaction;
		serverTransaction.setApplicationData(this);
		DialogExt dialog = (DialogExt) this.serverTransaction.getDialog();
		String dialogId = this.getDialogId();
		logger.debug("setServerTransaction: dialogId = " + dialogId);
		if (this.dialog != null && dialog != null) {
			SipDialog sipDialog = this.dialog;
			dialog.setApplicationData(sipDialog);
			sipDialog.setDialog(dialog);
		} else {
			logger.debug("dialog is not set for SipServerTransaction ");
		}
		RequestExt request = (RequestExt) serverTransaction.getRequest();
		SipTester.mapFromTag(this.getSipRequest().getSipRequest(), request);
	}

	public SipClientTransaction getMatchingClientTransaction() {
		return this.matchingClientTransaction;
	}

	public SipResponse getFinalResponse() {
		return this.responses.last();
	}

	public void setMatchingServerTransactions(
			Collection<SipServerTransaction> matchingServerTransactions) {
		this.matchingServerTransactions = matchingServerTransactions;
	}
	
	/**
     * Is this transaction of interest to the emulator?
     */
    public boolean isOfInterest() {
		HostPort sourceHostPort = this.getSipRequest().getSourceHostPort();
		HostPort targetHostPort = this.getSipRequest().getTargetHostPort();
		return ! sourceHostPort.equals(targetHostPort) &&
		  SipTester.isHostPortOfInterest(sourceHostPort) &&
		  SipTester.isHostPortOfInterest(targetHostPort) &&  
		  SipTester.isHostPortEmulated(targetHostPort) ;
		
	}

}
