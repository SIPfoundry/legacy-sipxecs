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

import java.io.IOException;
import java.util.Collection;
import java.util.Iterator;
import java.util.concurrent.ConcurrentHashMap;

import javax.sip.Dialog;
import javax.sip.ServerTransaction;
import javax.sip.SipProvider;
import javax.sip.header.ViaHeader;
import javax.sip.message.Request;

import org.apache.log4j.Logger;
import org.sipfoundry.sipxbridge.symmitron.SymmitronException;


/**
 * Manages a set of back to back user agents indexed by call id.
 * 
 * @author M. Ranganathan
 * 
 * 
 */
public class BackToBackUserAgentFactory {

	private static final Logger logger = Logger.getLogger(BackToBackUserAgentFactory.class);
	
	/*
	 * An association of callid to back to back user agent.
	 */
	private ConcurrentHashMap<String, BackToBackUserAgent> backToBackUserAgentTable = new ConcurrentHashMap<String, BackToBackUserAgent>();

	
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
		String callId = SipUtilities.getCallId(request);
		// BackToBackUserAgent b2bua = callTable.get(callId);
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
			 *Do we have a B2BUA for this call ID? If so, point at it.
			 */
			if (this.backToBackUserAgentTable.containsKey(callId)) {
				b2bua = this.backToBackUserAgentTable.get(callId);
				if ( dialog.getApplicationData() == null ) {
					DialogApplicationData.attach(b2bua, dialog, serverTransaction, request);
					DialogApplicationData.get(dialog).setItspInfo(accountInfo);
				}

			} else {

				b2bua = new BackToBackUserAgent(provider, request, dialog,
						accountInfo);
				DialogApplicationData.attach(b2bua, dialog, serverTransaction,
						request);
				DialogApplicationData.get(dialog).setItspInfo(accountInfo);
				this.backToBackUserAgentTable.put(callId, b2bua);
			}

	     } catch (IOException ex) {
			logger.error("unepxected exception", ex);
			throw new RuntimeException("IOException -- check SipXrelay", ex);
		 } catch (Exception ex) {
			logger.error("unexpected exception ", ex);
			throw new RuntimeException(
					"Unepxected exception processing request", ex);
		} finally {
		    logger.debug("returning " + b2bua);
		}
		return b2bua;

	}
	
	
	/**
	 * Remove all the records in the back to back user agent table corresponsing
	 * to a given B2BUA.
	 * 
	 * @param backToBackUserAgent
	 */
	void removeBackToBackUserAgent(BackToBackUserAgent backToBackUserAgent) {
		for (Iterator<String> keyIterator = this.backToBackUserAgentTable
				.keySet().iterator(); keyIterator.hasNext();) {
			String key = keyIterator.next();
			if (this.backToBackUserAgentTable.get(key) == backToBackUserAgent) {
				keyIterator.remove();
			}
		}

		logger
				.debug("CallControlManager: removeBackToBackUserAgent() after removal "
						+ this.backToBackUserAgentTable);

	}

	/**
	 * Dump the B2BUA table for memory debugging.
	 */

	void dumpBackToBackUATable() {

		logger.debug("B2BUATable = " + this.backToBackUserAgentTable);

	}

	/**
	 * Get the Back to back user agent set.
	 */
	Collection<BackToBackUserAgent> getBackToBackUserAgents() {
		return this.backToBackUserAgentTable.values();
	}

	
	/**
	 * Get the B2BUA for a given callId. This method is used by the XML RPC
	 * interface to cancel a call hence needs to be public.
	 * 
	 * @param callId
	 * @return
	 */
	BackToBackUserAgent getBackToBackUserAgent(String callId) {

		return this.backToBackUserAgentTable.get(callId);

	}

	/**
	 * Set the back to back ua for a given call id.
	 * 
	 * @param callId
	 * @param backToBackUserAgent
	 */
	void setBackToBackUserAgent(String callId,
			BackToBackUserAgent backToBackUserAgent) {
		this.backToBackUserAgentTable.put(callId, backToBackUserAgent);

	}
	
	
}
