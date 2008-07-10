package org.sipfoundry.sipxbridge;

import gov.nist.javax.sip.ListeningPointExt;

import java.net.InetAddress;
import java.text.ParseException;
import java.util.TimerTask;

import javax.sip.SipProvider;
import javax.sip.message.Request;

import org.apache.log4j.Logger;

/**
 * Sends out a CRLF to keep the NAT pinhole open.
 * 
 * @author mranga
 *
 */
public class CrLfTimerTask extends TimerTask {
    private static Logger logger = Logger.getLogger(CrLfTimerTask.class);
    private ItspAccountInfo accountInfo;
    
    SipProvider provider;
    
    public CrLfTimerTask(SipProvider provider, ItspAccountInfo accountInfo) {
        this.provider = provider;
        this.accountInfo  = accountInfo;
    }

    @Override
    public void run() {
        try {
            ListeningPointExt listeningPoint = (ListeningPointExt) provider.getListeningPoint("udp");
            listeningPoint.sendHeartbeat
                (InetAddress.getByName(accountInfo.getInboundProxy()).getHostAddress(),
                    Gateway.getSipKeepaliveSeconds());
        } catch (Exception ex) {
            logger.error("Unexpected parse exception",ex);
        }
        

    }

}
