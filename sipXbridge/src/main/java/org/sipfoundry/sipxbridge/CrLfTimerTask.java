/*
 *  Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 *  Contributors retain copyright to elements licensed under a Contributor Agreement.
 *  Licensed to the User under the LGPL license.
 *
 */

package org.sipfoundry.sipxbridge;

import gov.nist.javax.sip.ListeningPointExt;

import java.net.InetAddress;
import java.util.TimerTask;

import javax.sip.SipProvider;

import org.apache.log4j.Logger;

/**
 * Sends out a CRLF to keep the NAT pinhole open.
 * 
 * @author mranga
 *
 */
class CrLfTimerTask extends TimerTask {
    private static Logger logger = Logger.getLogger(CrLfTimerTask.class);
    private ItspAccountInfo accountInfo; 
    private SipProvider provider;
    
    CrLfTimerTask(SipProvider provider, ItspAccountInfo accountInfo) {
        this.provider = provider;
        this.accountInfo  = accountInfo;
    }

    @Override
    public void run() {
        try {
            ListeningPointExt listeningPoint = (ListeningPointExt) provider.getListeningPoint("udp");
            if ( logger.isDebugEnabled() ) logger.debug("sending heartbeat to " + accountInfo.getInboundProxy());
            listeningPoint.sendHeartbeat
                (InetAddress.getByName(accountInfo.getInboundProxy()).getHostAddress(),
                    accountInfo.getInboundProxyPort());
        } catch (Exception ex) {
            logger.error("Unexpected exception in CRLF timer task "+ ex.getMessage());
        }
        

    }

}
