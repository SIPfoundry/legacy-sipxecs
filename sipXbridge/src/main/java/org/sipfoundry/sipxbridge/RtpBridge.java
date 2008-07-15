/*
 *  Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 *  Contributors retain copyright to elements licensed under a Contributor Agreement.
 *  Licensed to the User under the LGPL license.
 *
 */
package org.sipfoundry.sipxbridge;

import java.io.IOException;

import javax.sdp.SdpFactory;
import javax.sdp.SdpParseException;
import javax.sdp.SessionDescription;
import javax.sip.message.Request;

import org.sipfoundry.sipxbridge.symmitron.Bridge;

class RtpBridge extends Bridge {
    
    SessionDescription sessionDescription;
    
    /**
     * Constructor.
     * 
     * @param itspAccountInfo
     * @throws IOException
     */
    RtpBridge(Request request) throws IOException {
        super();
        try {

            this.sessionDescription = SdpFactory.getInstance().createSessionDescription(
                    new String(request.getRawContent()));
        } catch (SdpParseException ex) {
            throw new IOException("Unable to parse SDP ");
        }
    }
}
