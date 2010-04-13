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

import gov.nist.javax.sip.SipStackExt;
import gov.nist.javax.sip.SipStackImpl;

import java.util.Collection;
import java.util.HashSet;
import java.util.Iterator;
import java.util.TimerTask;
import java.util.concurrent.ConcurrentSkipListSet;

import javax.sip.Dialog;
import javax.sip.DialogState;
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
	
	private ConcurrentSkipListSet<BackToBackUserAgent> backToBackUserAgentTable = new ConcurrentSkipListSet<BackToBackUserAgent> ();
	
	class Scanner extends TimerTask {

	    @Override
	    public void run() {
	        try {
	            Iterator<BackToBackUserAgent> iter = backToBackUserAgentTable.iterator();
	            boolean removed = false;
	            while( iter.hasNext() ) {
	                BackToBackUserAgent b2bua = iter.next();
	                if ( b2bua.isPendingTermination() ) {
	                    b2bua.cleanUp();
	                    logger.debug("Removing BackToBackUserAgent");
	                    iter.remove();
	                    removed = true;
	                }
	            }
	            /*
	             * Check for dialog leaks. This us useful for unit testing.
	             */
	            if ( logger.isDebugEnabled() && removed ) {
	                logger.debug("Dialog Table : ");
	                for (Dialog dialog : ( (SipStackExt) ProtocolObjects.getSipStack()).getDialogs() ) {
	                    logger.debug("Dialog " + dialog + " dialogState = " + dialog.getState() ); 
	                    if ( dialog.getState() != DialogState.TERMINATED ) {
	                    	if ( DialogContext.get(dialog) != null ) {
	                    		logger.debug("Dialog was allocated at " + DialogContext.get(dialog).getCreationPointStackTrace());
	                    	} else {
	                    		logger.debug("Null dialog context!");
	                    	}
	                    }
	                }
	            }
	        } catch (Exception ex) {
	            logger.error("Exception caught in Timer task",ex);
	        }
	    }


	}
	
	public BackToBackUserAgentFactory() {
	    Gateway.getTimer().schedule(new Scanner(), 10*1000, 10*1000);
	}

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
				String callId = SipUtilities.getCallId(request);

				/*
				 * Linear search here but avoids having to keep a reference to 
				 * the b2bua here. Keeping a reference can lead to reference management
				 * problems ( leaks ) and hence this quick search is worthwhile.
				 */
				for (BackToBackUserAgent backtobackua : backToBackUserAgentTable) {
				    if (backtobackua.managesCallId(callId)) {
				        b2bua = backtobackua;
				    }
				}

				/*
				 * Could not find an existing call so go ahead and create one.
				 * If we exceed the call limit then fail to add one. This will
				 * result in throwing an exception which will, in turn be reported
				 * as a Server Failure to the calling party.
				 */
				if (b2bua == null && 
				        (Gateway.getBridgeConfiguration().getCallLimit() == -1 ||
                                        backToBackUserAgentTable.size() < Gateway.getBridgeConfiguration().getCallLimit())) {
					b2bua = new BackToBackUserAgent(provider, request, dialog,
							accountInfo);
					this.backToBackUserAgentTable.add(b2bua);
				}
                                /*
                                 * Could not allocate a back to back user agent. So fail the call.
                                 */
				if ( b2bua == null ) {
				    throw new SipXbridgeException ("Call limit exceeded");
				}
				
				dialogContext = DialogContext.attach(b2bua, dialog, serverTransaction, request);
				dialogContext.setItspInfo(accountInfo);
				dialogContext.setBackToBackUserAgent(b2bua);
				b2bua.addDialog(dialogContext);
			}
		} catch (SipXbridgeException ex) {
		    logger.error("Exception while trying to create B2BUA",ex);
		    throw ex;
		} catch (Exception ex) {
			logger.error("unexpected exception ", ex);
			throw new SipXbridgeException(
					"Initialization exception while processing request", ex);
		} finally {
			logger.debug("returning " + b2bua);
		}
		return b2bua;

	}
	
	

        /**
        * Get the structure corresponding to a given callId
        */
	public BackToBackUserAgent getBackToBackUserAgent(String callId) {
	 /*
         * Linear search here but avoids having to keep a reference to 
         * the b2bua here. Keeping a reference can lead to reference management
         * problems ( leaks ) and hence this quick search is worthwhile.
         */
	    for(BackToBackUserAgent b2bua : this.backToBackUserAgentTable ) {
	        if ( b2bua.managesCallId(callId)) return b2bua;
	    }
	    
        return null;
	}

        /**
 	* Provide the current active call count.
 	*/
        public int getBackToBackUserAgentCount() {
            return  this.backToBackUserAgentTable.size();
        }

	/**
	 * Remove the B2BUA from the table of B2BUA.
	 * 
	 * @param backToBackUserAgent
	 */
        public void removeBackToBackUserAgent(BackToBackUserAgent backToBackUserAgent) {
             this.backToBackUserAgentTable.remove(backToBackUserAgent);
        }

}
