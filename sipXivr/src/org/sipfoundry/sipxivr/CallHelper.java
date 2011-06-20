/*
 *
 *
 * Copyright (C) 2009 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 */

package org.sipfoundry.sipxivr;

import java.io.InputStream;
import java.net.HttpURLConnection;
import java.util.LinkedList;
import java.util.StringTokenizer;

import javax.xml.parsers.DocumentBuilder;
import javax.xml.parsers.DocumentBuilderFactory;

import org.apache.log4j.Logger;
import org.w3c.dom.Document;
import org.w3c.dom.Node;
import org.w3c.dom.NodeList;

// Ideally we should import those SIP stats code definitions from
// import org.apache.http.contrib.sip;

public class CallHelper {
	static final Logger LOG = Logger.getLogger("org.sipfoundry.sipxivr");

	public enum CallHelperReturnCode {
		NO_ANSWER, ANSWER, BUSY, FORBIDDEN, INVALID, FAILURE
	}

	// check for immediate failures x milliseconds later
	private static final int m_initialCallProgressDelay = 1000;

	// check for periodic call progress every x milliseconds later
	private static final int m_periodicCallProgressDelay = 5000;

	// longest SIP protocol boundary for call no answer
	private static final int m_maxNoAnswerTimeoutValue = 90;

	// Status codes taken from: http://www.ietf.org/rfc/rfc3261.txt
	// --- 1xx Informational ---
	// --- 2xx Successful ---
	// --- 3xx Redirection ---
	// --- 4xx Request Failure ---
	// --- 5xx Server Failure ---
	// --- 6xx Global Failures ---

	// The following SIP failure status codes need to be handled separately
	public static final int SC_FORBIDDEN = 403;
	public static final int SC_PROXY_AUTHENTICATION_REQUIRED = 407;
	public static final int SC_BUSY_HERE = 486;
	public static final int SC_REQUEST_TERMINATED = 487;

	// User provided call timeout before we indicate no answer
	private int m_callTimeout;

	// Keep track of how much time we have been monitoring the call
	private int m_timeElapsed;

	// This is to keep track of the previous call context query response
	// length to avoid parsing the same record if it has not changed
	private int m_previousResponseLength;

	// Constructor
	public CallHelper() {
	}

	/**
	 * This method makes a call using the third party call controller.
	 * 
	 * @param callingNumber
	 * @param calledNumber
	 * @param userId
	 * @param timeout
	 * @return call status from the calling side perspective (first call leg)
	 * @throws Exception
	 */
	public CallHelperReturnCode call(String callingNumber, String calledNumber,
			String userId, int timeout) throws Exception {

		// Make sure that call no answer timeout value is reasonable
		if (timeout <= m_maxNoAnswerTimeoutValue) {
			m_callTimeout = timeout;
		} else {
			m_callTimeout = m_maxNoAnswerTimeoutValue;
		}

		String parametersUrl = callingNumber + "/" + calledNumber + "?agent="
				+ userId + "&timeout=" + m_callTimeout;

		// Convert seconds to milliseconds
		m_callTimeout = m_callTimeout * 1000;

		// Initialize member attributes to support a new call
		m_previousResponseLength = 0;
		m_timeElapsed = 0;

		try {
			// Make the call first
			RestfulRequest rr = new RestfulRequest(IvrConfiguration.get()
					.get3pccSecureUrl());

			// Check if the call failed immediately because of a basic error
			// such as malformed URL or unauthorized user. The basic error
			// is logged down in the called method
			if (rr.post(parametersUrl) == false) {
				return CallHelperReturnCode.FAILURE;
			}

			// Monitor the progress of the call until the calling party has
			// reached a deterministic state e.g. answer, busy, denied or
			// until the call has timed out.
			while (m_timeElapsed <= m_callTimeout) {

				if (m_timeElapsed == 0) {
					Thread.sleep(m_initialCallProgressDelay);
					m_timeElapsed += m_periodicCallProgressDelay;
				} else {
					Thread.sleep(m_periodicCallProgressDelay);
					m_timeElapsed += m_periodicCallProgressDelay;
				}

				// Query the status of the call. Check if the query failed
				// immediately because of a basic error such as malformed
				// URL or unauthorized user. The basic error is logged down
				// in the called method
				HttpURLConnection urlConn = rr.getConnection(parametersUrl);
				if (rr.get(urlConn) == false) {
					return CallHelperReturnCode.FAILURE;
				}

				// We want to optimize parsing the call context.
				// If the size of previous call context response is the
				// same then skip the parsing until the next query is done.
				// The size of the call context response is expected to grow
				// as the call progresses. If it turns out it has decreases
				// then this highlights an issue with the 3pcc REST server.
				if (urlConn.getContentLength() > m_previousResponseLength) {
					m_previousResponseLength = urlConn.getContentLength();

					InputStream in = urlConn.getInputStream();
					DocumentBuilderFactory factory = DocumentBuilderFactory
							.newInstance();
					DocumentBuilder builder = factory.newDocumentBuilder();
					Document doc = builder.parse(in);

					// parseCallEvents(doc)
					if (doc == null) {
						// LOG an error - this is unexpected
						return CallHelperReturnCode.FAILURE;
					}

					LinkedList<String> inviteEvents = new LinkedList<String>();
					LinkedList<String> notifyEvents = new LinkedList<String>();

					if (!parseCallEvents(doc, inviteEvents, notifyEvents)) {
						// LOG an error - this is unexpected
						return CallHelperReturnCode.FAILURE;
					}

					if (!inviteEvents.isEmpty()) {
						// Next, extract the SIP status code from the last
						// INVITE event
						int sipStatusCode = 0;

						String lastInviteString = inviteEvents.getLast();
						StringTokenizer st = new StringTokenizer(
								lastInviteString);

						LOG.info("Last INVITE message: " + lastInviteString);

						if (st.countTokens() >= 2) {
                            // The second token is the SIP status code
							st.nextToken();
							sipStatusCode = Integer.parseInt(st.nextToken());
						} else {
							LOG.error("INVITE message is too short: " + lastInviteString);
							
							return CallHelperReturnCode.INVALID;
						}

						// Check if the first call leg answered the call
						if (sipStatusCode >= 200 && sipStatusCode <= 299) {
							return CallHelperReturnCode.ANSWER;
						}

						// Check for an error condition
						if (sipStatusCode >= 400 && sipStatusCode <= 699) {
							// Check if the calling side is busy
							if (sipStatusCode == SC_BUSY_HERE) {
								return CallHelperReturnCode.BUSY;
							}

							// Check if the calling has permissions to dial
							// the calling number
							if (sipStatusCode == SC_FORBIDDEN) {
								return CallHelperReturnCode.FORBIDDEN;
							}

							// Check if the calling has not answered the call
							if (sipStatusCode == SC_REQUEST_TERMINATED) {
								return CallHelperReturnCode.NO_ANSWER;
							}

							// The only exception we have to let through as
							// not being an error is proxy authentication.
							// All other errors are considered failures
							if (sipStatusCode != SC_PROXY_AUTHENTICATION_REQUIRED) {
								return CallHelperReturnCode.FAILURE;
							}
						}

						// Let's monitor the progress of the call in the next
						// iteration
					}
				} else if (urlConn.getContentLength() < m_previousResponseLength) {
					LOG.error("Unexpected smaller response content: previous " + 
							m_previousResponseLength + " latest " + urlConn.getContentLength());
					
					return CallHelperReturnCode.FAILURE;
				}
				
				// Flush the last response by disconnecting
				urlConn.disconnect();
			}
		} catch (Exception e) {
			LOG.error("Unexpected error invoking the REST server: ", e);

			return CallHelperReturnCode.INVALID;
		}

		return CallHelperReturnCode.NO_ANSWER;
	}

	private boolean parseCallEvents(Document callEvents,
			LinkedList<String> inviteEvents, LinkedList<String> notifyEvents)
			throws Exception {
		String prop = null;
		try {
			prop = "status";
			// Walk validUsers, building up m_users as we go
			NodeList events = callEvents.getElementsByTagName(prop);
			for (int eventNum = 0; eventNum < events.getLength(); eventNum++) {
				Node eventRecord = events.item(eventNum);
				Node next = eventRecord.getFirstChild();

				boolean isInvite = true;
				while (next != null) {
					if (next.getNodeType() == Node.ELEMENT_NODE) {
						String name = next.getNodeName();
						String text = next.getTextContent().trim();

						if (name.contentEquals("method")) {
							isInvite = text.contentEquals("INVITE");
						} else if (name.contentEquals("status-line")) {
							if (isInvite) {
								inviteEvents.add(text);
							} else {
								notifyEvents.add(text);
							}
						}
					}
					next = next.getNextSibling();
				}
			}

		} catch (Exception e) {
			LOG.error("Problem understanding document " + prop, e);
			
			return false;
		}

		return true;
	}
}
