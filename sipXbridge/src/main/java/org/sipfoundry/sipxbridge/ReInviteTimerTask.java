/*
 * Copyright (C) 2010 Avaya, certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 */
package org.sipfoundry.sipxbridge;

import java.util.TimerTask;

import javax.sip.Dialog;
import javax.sip.DialogState;
import javax.sip.InvalidArgumentException;
import javax.sip.ServerTransaction;
import javax.sip.SipException;
import javax.sip.TransactionState;
import javax.sip.message.Response;

import org.apache.log4j.Logger;


/**
 * A timer task that schedules a re-INVITE retry. This can happen when we are switching to MOH mid call transfer.
 */
class ReInviteTimerTask extends TimerTask {
	
	private RtpSession rtpSession;
	private ServerTransaction serverTransaction;
	private Dialog dialog;
	
	private static Logger logger = Logger.getLogger(ReInviteTimerTask.class);

	public ReInviteTimerTask(RtpSession rtpSession,
		ServerTransaction serverTransaction, Dialog dialog ) throws SipException, InvalidArgumentException {
		
		Response response = SipUtilities.createResponse(serverTransaction,
				Response.TRYING);
		serverTransaction.sendResponse(response);
		this.rtpSession =rtpSession;
		this.serverTransaction = serverTransaction;
		this.dialog = dialog;
		DialogContext.get(dialog).setPendingReInvite(serverTransaction);
	}

	@Override
	public void run() {
		try {
			Dialog peerDialog = DialogContext.getPeerDialog(dialog);
			
			if (peerDialog.getState() == null && peerDialog.getState() != DialogState.TERMINATED && 
					serverTransaction.getState() != TransactionState.TERMINATED ) {
				DialogContext.get(dialog).setPendingReInvite(null);
				RtpSessionUtilities.forwardReInvite(rtpSession, serverTransaction, dialog, false);
				this.cancel();
			} else if ( serverTransaction.getState() == TransactionState.TERMINATED) {
				logger.debug("server transaction has already been processed");
				this.cancel();
			} else if (peerDialog.getState() == DialogState.TERMINATED ) {
				CallControlUtilities.sendInternalError(serverTransaction, "Peer dialog is Terminated");
				this.cancel();
			} else {
				logger.trace("waiting to forward re-INVITE");
			}
		} catch (Exception ex) {
			logger.error("Error processing request" + serverTransaction.getRequest(), ex);
            CallControlUtilities.sendInternalError(serverTransaction, ex);
		}
	}
	
}

