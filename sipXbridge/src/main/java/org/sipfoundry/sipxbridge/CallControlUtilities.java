/*
 * 
 * 
 * Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 * 
 *
 */

package org.sipfoundry.sipxbridge;

import javax.sip.ServerTransaction;
import javax.sip.message.Request;
import javax.sip.message.Response;

/**
 * Utility functions to send various error responses.
 */
public class CallControlUtilities {

	static void sendInternalError(ServerTransaction st, Exception ex) {
		try {
			Request request = st.getRequest();
			Response response = ProtocolObjects.messageFactory.createResponse(
					Response.SERVER_INTERNAL_ERROR, request);
			if (CallControlManager.logger.isDebugEnabled()) {
				String message = "Exception occured at " + ex.getMessage()
						+ " at " + ex.getStackTrace()[0].getFileName() + ":"
						+ ex.getStackTrace()[0].getLineNumber();
	
				response.setReasonPhrase(message);
			} else {
				response.setReasonPhrase(ex.getCause().getMessage());
			}
			st.sendResponse(response);
	
		} catch (Exception e) {
			throw new RuntimeException("Check gateway configuration", e);
		}
	}

	static void sendBadRequestError(ServerTransaction st, Exception ex) {
		try {
			Request request = st.getRequest();
			Response response = ProtocolObjects.messageFactory.createResponse(
					Response.BAD_REQUEST, request);
			if (CallControlManager.logger.isDebugEnabled()) {
				String message = "Exception occured at " + ex.getMessage()
						+ " at " + ex.getStackTrace()[0].getFileName() + ":"
						+ ex.getStackTrace()[0].getLineNumber();
	
				response.setReasonPhrase(message);
			}
			st.sendResponse(response);
	
		} catch (Exception e) {
			throw new RuntimeException("Check gateway configuration", e);
		}
	}

}
