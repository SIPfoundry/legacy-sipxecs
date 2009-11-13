/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.callcontroller;


import javax.sip.ClientTransaction;
import javax.sip.Dialog;
import javax.sip.message.Request;

import gov.nist.javax.sip.clientauthutils.UserCredentialHash;

import org.apache.log4j.Logger;


public class SipServiceImpl  implements SipService {

    static final Logger LOG = Logger.getLogger(SipServiceImpl.class);

  
    
  
    public SipServiceImpl() {
    	
    }
   

    public Dialog sendRefer(UserCredentialHash agentCredentials, String agentAddrSpec, String displayName, String callingPartyAddrSpec,
            String referTarget, String subject, boolean allowForwarding, DialogContext dialogContext, int timeout) {
        LOG.debug("sendRefer: source = " + agentAddrSpec + " dest = " 
                + callingPartyAddrSpec + " referTarget = " + referTarget
                + " allowForwarding = " + allowForwarding );
        InviteMessage message = new InviteMessage( agentCredentials, displayName,
                agentAddrSpec, callingPartyAddrSpec,
                referTarget, timeout);
        message.setSubject(subject);
        message.setforwardingAllowed(allowForwarding);
        String sdpBody = SipUtils.formatWithIpAddress(InviteMessage.SDP_BODY_FORMAT);
        ClientTransaction clientTransaction = message.createAndSend(dialogContext, Request.INVITE,
                Operator.SEND_3PCC_REFER_CALL_SETUP, sdpBody);
        return clientTransaction.getDialog();
        
    }
    
    public Dialog sendInvite(UserCredentialHash agentCredentials, String agentAddrSpec, String displayName, String callingPartyAddrSpec,
            String calledPartyAddrSpec, String subject, boolean allowForwarding, DialogContext dialogContext, int timeout) {
        LOG.debug("sendRefer: source = " + agentAddrSpec + " dest = " 
                + callingPartyAddrSpec + " referTarget = " + calledPartyAddrSpec
                + " allowForwarding = " + allowForwarding );
        InviteMessage message = new InviteMessage( agentCredentials, displayName,
                agentAddrSpec, callingPartyAddrSpec,
                calledPartyAddrSpec, timeout);
        message.setSubject(subject);
        message.setforwardingAllowed(allowForwarding);
        String sdpBody = SipUtils.formatWithIpAddress(InviteMessage.SDP_BODY_FORMAT);
        
        ClientTransaction clientTransaction = message.createAndSend(dialogContext, Request.INVITE,
                Operator.SEND_3PCC_CALL_SETUP1, sdpBody);
        return clientTransaction.getDialog();
        
    }
   
   
}
