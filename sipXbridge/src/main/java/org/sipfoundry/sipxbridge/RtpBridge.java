package org.sipfoundry.sipxbridge;

import java.io.IOException;

import javax.sdp.SdpFactory;
import javax.sdp.SdpParseException;
import javax.sdp.SessionDescription;
import javax.sip.message.Request;

import org.sipfoundry.sipxbridge.symmitron.Bridge;

public class RtpBridge extends Bridge {
    
    SessionDescription sessionDescription;
    
    /**
     * Constructor.
     * 
     * @param itspAccountInfo
     * @throws IOException
     */
    public RtpBridge(Request request) throws IOException {
        super();
        try {

            this.sessionDescription = SdpFactory.getInstance().createSessionDescription(
                    new String(request.getRawContent()));
        } catch (SdpParseException ex) {
            throw new IOException("Unable to parse SDP ");
        }
    }
}
