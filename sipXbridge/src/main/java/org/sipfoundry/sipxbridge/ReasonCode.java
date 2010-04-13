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

/**
 * Reason codes. These are put into BYE requests for diagnostic purposes.
 */
public class ReasonCode {

	public static final int SIPXBRIDGE_CONFIG_ERROR = 200;
	public static final int SIPXBRIDGE_INTERNAL_ERROR = 201;
	public static final int SESSION_TIMER_ERROR = 202;
	public static final int UNEXPECTED_CONTENT_TYPE = 203;
	public static final int UNCAUGHT_EXCEPTION = 204;
	public static final int PROTOCOL_ERROR = 205;
	public static final int CALL_SETUP_ERROR = 206;
	public static final int BRIDGE_STOPPING = 207;
	public static final int ACCOUNT_NOT_FOUND = 208;
	public static final int AUTHENTICATION_FAILURE = 209;
	public static final int ERROR_SENDING_REINVITE = 210;
	public static final int TIMED_OUT_WAITING_TO_SEND_REINVITE = 211;
	public static final int CALL_TERMINATED = 212;
    public static final int RELAYED_ERROR_RESPONSE = 213;
    public static final int FORK_TIMED_OUT = 214;
    public static final int UNEXPECTED_RESPONSE_CODE = 215;
    public static final int UNEXPECTED_DIALOG_STATE = 216;

}
