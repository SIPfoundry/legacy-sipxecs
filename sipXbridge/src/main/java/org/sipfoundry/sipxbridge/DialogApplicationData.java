/*
 *  Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 *  Contributors retain copyright to elements licensed under a Contributor Agreement.
 *  Licensed to the User under the LGPL license.
 *
 */

package org.sipfoundry.sipxbridge;

import javax.sip.Dialog;
import javax.sip.ServerTransaction;
import javax.sip.message.Response;

import org.apache.log4j.Logger;
import org.sipfoundry.sipxbridge.symmitron.Sym;

/**
 * Store information that is specific to a Dialog. This is a temporary holding
 * place for dialog specific data.
 * 
 * @author M. Ranganathan
 * 
 */
class DialogApplicationData {

    private static Logger logger = Logger.getLogger(DialogApplicationData.class);
    
    /*
     * Whether or not to forward requests to the PeerDialogs of this dialog (
     * assuming the state of the Dialog is not TERMINATED).
     */
    boolean forwarding = true;

    /*
     * The Peer Dialog of this Dialog.
     */
    Dialog peerDialog;

    /*
     * The last response seen by the dialog.
     */
    Response lastResponse;

    /*
     * The B2BUA associated with the dialog.
     */
    private BackToBackUserAgent backToBackUserAgent;

    /*
     * The replaced dialog (for Consultative XFer processing ).
     */
    Dialog replacedDialog;

    /*
     * The MOH server dialog
     */
    Dialog musicOnHoldDialog;
    
    /*
     * Rtp session associated with this call leg.
     */

    private RtpSession rtpSession;
    
    /*
     * The RTCP session.
     */
    private RtpSession rtcpSession;
    
    
    boolean isSdpAnswerPending;

    
    /*
     * Account information for the outbound dialog.
     */
    ItspAccountInfo itspInfo; 
   

    
    private DialogApplicationData() {

    }

    /**
     * Conveniance methods
     */
    public static Dialog getPeerDialog(Dialog dialog) {
        return ((DialogApplicationData) dialog.getApplicationData()).peerDialog;
    }

    public static RtpSession getRtpSession(Dialog dialog) {
        return ((DialogApplicationData) dialog.getApplicationData()).rtpSession;
    }
    
    public static RtpSession getRtcpSession(Dialog dialog) {
        return ((DialogApplicationData) dialog.getApplicationData()).rtcpSession;
    }

    public static DialogApplicationData attach(
            BackToBackUserAgent backToBackUserAgent, Dialog dialog) {
        if ( backToBackUserAgent == null ) 
            throw new NullPointerException("Null back2back ua");
        DialogApplicationData dat = new DialogApplicationData();
        dat.setBackToBackUserAgent(backToBackUserAgent);
        dialog.setApplicationData(dat);
        return dat;
    }

    public static DialogApplicationData get(Dialog dialog) {
        return (DialogApplicationData) dialog.getApplicationData();
    }

    /**
     * @param rtpSession the rtpSession to set
     */
    public void setRtpSession(RtpSession rtpSession) {
        if ( this.rtpSession != null ) {
            logger.warn("Resetting RTP session ", new Exception());
        }
        this.rtpSession = rtpSession;
    }

    /**
     * @return the rtpSession
     */
    public RtpSession getRtpSession() {
        return rtpSession;
    }

    /**
     * @param rtcpSession the rtcpSession to set
     */
    public void setRtcpSession(RtpSession rtcpSession) {
        this.rtcpSession = rtcpSession;
    }

    /**
     * @return the rtcpSession
     */
    public RtpSession getRtcpSession() {
        return rtcpSession;
    }

    /**
     * @param backToBackUserAgent the backToBackUserAgent to set
     */
    public void setBackToBackUserAgent(BackToBackUserAgent backToBackUserAgent) {
        this.backToBackUserAgent = backToBackUserAgent;
    }

    /**
     * @return the backToBackUserAgent
     */
    public BackToBackUserAgent getBackToBackUserAgent() {
        return backToBackUserAgent;
    }
    

   

}
