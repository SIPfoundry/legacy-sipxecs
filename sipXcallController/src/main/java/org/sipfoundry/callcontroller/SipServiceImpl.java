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

import gov.nist.javax.sip.clientauthutils.UserCredentialHash;

import org.apache.log4j.Logger;


public class SipServiceImpl  implements SipService {

    static final Logger LOG = Logger.getLogger(SipServiceImpl.class);

    private SipStackBean sipStackBean;
    
  
    public SipServiceImpl(SipStackBean stackBean) {
    	this.sipStackBean = stackBean;
    }
   

    public Dialog sendRefer(UserCredentialHash agentCredentials, String agentAddrSpec, String displayName, String callingPartyAddrSpec,
            String referTarget, String subject, boolean allowForwarding) {
        LOG.debug("sendRefer: source = " + agentAddrSpec + " dest = " 
                + callingPartyAddrSpec + " referTarget = " + referTarget
                + " allowForwarding = " + allowForwarding );
        InviteMessage message = new InviteMessage(sipStackBean, agentCredentials, displayName, callingPartyAddrSpec,
                agentAddrSpec, referTarget, Operator.SEND_3PCC_REFER_CALL_SETUP);
        message.setSubject(subject);
        message.setforwardingAllowed(allowForwarding);
        ClientTransaction clientTransaction = message.createAndSend();
        return clientTransaction.getDialog();
        
    }

   
}
