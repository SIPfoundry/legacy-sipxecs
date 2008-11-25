/*
 *  Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 *  Contributors retain copyright to elements licensed under a Contributor Agreement.
 *  Licensed to the User under the LGPL license.
 *
 */
package org.sipfoundry.sipxbridge;

import java.io.IOException;
import java.util.HashSet;

import javax.sdp.SdpFactory;
import javax.sdp.SdpParseException;
import javax.sdp.SessionDescription;
import javax.sip.message.Request;

import org.sipfoundry.sipxbridge.symmitron.BridgeInterface;
import org.sipfoundry.sipxbridge.symmitron.BridgeState;

class RtpBridge  {
    
    SessionDescription sessionDescription;
    private BridgeInterface bridge;
    
    private HashSet<RtpSession> syms = new HashSet<RtpSession> ();
    
    /**
     * Constructor.
     * 
     * @param itspAccountInfo
     * @throws IOException
     */
    RtpBridge(Request request, BridgeInterface bridge) throws IOException {
        
        try {

            this.sessionDescription = SdpFactory.getInstance().createSessionDescription(
                    new String(request.getRawContent()));
            this.bridge = bridge;
        } catch (SdpParseException ex) {
            throw new IOException("Unable to parse SDP ");
        }
    }
    
    RtpBridge(BridgeInterface bridge) {
        this.bridge = bridge;
    }
    
    public HashSet<RtpSession> getSyms() {
        return this.syms;
    }

    public void addSym(RtpSession rtpSession) {
        bridge.addSym(rtpSession.getSym());
        this.syms.add(rtpSession);
    }

    public void resume() {
        bridge.resume();
        
    }

    public void pause() {
       bridge.pause(); 
    }

    public void start() {
      bridge.start();
        
    }

    public void stop() {
       bridge.stop();
    }
    
    public BridgeState getState() {
        return this.bridge.getState();
    }
}
