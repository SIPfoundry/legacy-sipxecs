/*
 *  Copyright (C) 2009 Nortel Corp., certain elements licensed under a Contributor Agreement.
 *  Contributors retain copyright to elements licensed under a Contributor Agreement.
 *  Licensed to the User under the LGPL license.
 *
 */
package org.sipfoundry.sipxbridge;

import java.util.TimerTask;

import javax.sip.Dialog;
import javax.sip.message.Response;

import org.apache.log4j.Logger;

public class ReOfferTimerTask extends TimerTask {
	private static Logger logger = Logger.getLogger(ReOfferTimerTask.class);

	private Response response;
	private Dialog responseDialog;
	private Dialog reOfferDialog;

	public ReOfferTimerTask(Response response, Dialog responseDialog,
			Dialog reOfferDialog) {
		this.response = response;
		this.responseDialog = responseDialog;
		this.reOfferDialog = reOfferDialog;
	}

	@Override
	public void run() {
		BackToBackUserAgent b2bua = DialogContext.getBackToBackUserAgent(reOfferDialog);
		
	        
		try {
			b2bua.setPendingOperation(false);

			CallControlUtilities.sendSdpReOffer(response, responseDialog, reOfferDialog);
		} catch (Exception e) {
			logger.error("Problem sending sdp re-offer",e);
		     
            if (b2bua != null) {
                try {
                    b2bua.tearDown(ProtocolObjects.headerFactory.createReasonHeader("sipxbridge",
                            ReasonCode.UNCAUGHT_EXCEPTION,
                            "Unexpected exception sending Sdp Re-Offer"));
                } catch (Exception ex) {
                    logger.error("unexpected exception", ex);
                }
            }
		}
	}
	
}