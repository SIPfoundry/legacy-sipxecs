/*
 *  Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 *  Contributors retain copyright to elements licensed under a Contributor Agreement.
 *  Licensed to the User under the LGPL license.
 *
 */

package org.sipfoundry.sipxbridge;

import java.util.Map;

public interface RtpSessionInterface {
	
	public String getId();
	
	public RtpEndpointInterface getTransmitter();
	
	public RtpEndpointInterface getReceiver();
	
	public Map<String,Object> toMap();

}
