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
 * Operation that is applied to an existing RTP session 
 * as a result of an inbound request.
 */
public enum RtpSessionOperation {
	
	PLACE_HOLD,REMOVE_HOLD,PORT_REMAP,NO_OP, CODEC_RENEGOTIATION;

}
