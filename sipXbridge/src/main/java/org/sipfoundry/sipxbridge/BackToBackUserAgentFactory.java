/*
 * 
 * 
 * Copyright (C) 2008 Nortel Corp., certain elements licensed under a Contributor Agreement.  
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 * 
 *
 */

package org.sipfoundry.sipxbridge;

import gov.nist.javax.sip.SipStackImpl;

import java.util.Collection;

import javax.sip.Dialog;
import javax.sip.ServerTransaction;
import javax.sip.SipProvider;
import javax.sip.header.ViaHeader;
import javax.sip.message.Request;

import org.apache.log4j.Logger;

/**
 * Manages a set of back to back user agents indexed by call id.
 * 
 * @author M. Ranganathan
 * 
 * 
 */
public class BackToBackUserAgentFactory {

	private static final Logger logger = Logger
			.getLogger(BackToBackUserAgentFactory.class);

	/**
	 * Get a new B2bua instance or return an existing one for a call Id.
	 * 
	 * @param provider
	 *            -- provider associated with request.
	 * @param serverTransaction
	 *            -- server transaction.
	 * @param dialog
	 *            -- dialog for request.
	 */

	synchronized BackToBackUserAgent getBackToBackUserAgent(
			SipProvider provider, Request request,
			ServerTransaction serverTransaction, Dialog dialog) {

		boolean callOriginatedFromLan = provider == Gateway.getLanProvider();
		BackToBackUserAgent b2bua = null;

		try {

			ItspAccountInfo accountInfo = null;
			if (callOriginatedFromLan) {
				accountInfo = Gateway.getAccountManager().getAccount(request);
				if (accountInfo == null) {
					logger
							.error("Could not find iTSP account - check caller ID/domain");
					return null;
				}
				if (accountInfo.getState() == AccountState.INVALID) {
					logger
							.error("Could not find an itsp account -- the account is not valid");
					return null;
				}
			} else {
				/*
				 * Check the Via header of the inbound request to see if this is
				 * an account we know about. This will be the case when there is
				 * a registration for the request and we have a hop to its
				 * proxy. If we know where the request is coming from, we can
				 * set up various response fields accordingly.
				 */
				ViaHeader viaHeader = (ViaHeader) request
						.getHeader(ViaHeader.NAME);
				String host = viaHeader.getHost();
				int port = viaHeader.getPort();
				accountInfo = Gateway.getAccountManager().getItspAccount(host,
						port);
			}

			/*
			 * Do we have a B2BUA for this call ID? If so, point at it.
			 * Note that this will happen only once during an INVITE.
			 */

			DialogContext dialogContext = DialogContext.get(dialog);

			/*
			 * The dialog context is null. This is a fresh dialog that we have
			 * never seen before. See if we have an ogoing call already associated
			 * with it.
			 */
			if (dialogContext == null) {
				Collection<Dialog> dialogs = ((SipStackImpl) ProtocolObjects.sipStack)
						.getDialogs();
				String callId = SipUtilities.getCallId(request);

				/*
				 * Linear search here but avoids having to keep a reference to 
				 * the b2bua here. Keeping a reference can lead to reference management
				 * problems ( leaks ) and hence this quick search is worthwhile.
				 */
				for (Dialog sipDialog : dialogs) {
					if (sipDialog.getApplicationData() != null) {
						BackToBackUserAgent btobua = DialogContext
								.getBackToBackUserAgent(sipDialog);
						if (btobua.managesCallId(callId)) {
							logger.debug("found existing mapping for B2BuA");
							b2bua = btobua;
							break;
						}

					}
				}

				/*
				 * Could not find an existing call so go ahead and create one.
				 */
				if (b2bua == null) {
					b2bua = new BackToBackUserAgent(provider, request, dialog,
							accountInfo);
				}
				
				dialogContext = DialogContext.attach(b2bua, dialog, serverTransaction, request);
				dialogContext.setItspInfo(accountInfo);
				dialogContext.setBackToBackUserAgent(b2bua);
			}

		} catch (Exception ex) {
			logger.error("unexpected exception ", ex);
			throw new SipXbridgeException(
					"Initialization exception while processing request", ex);
		} finally {
			logger.debug("returning " + b2bua);
		}
		return b2bua;

	}

}
