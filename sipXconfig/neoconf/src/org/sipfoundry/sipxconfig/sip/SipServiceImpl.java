/*
 * 
 * 
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 * 
 * $
 */
package org.sipfoundry.sipxconfig.sip;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;

public class SipServiceImpl extends SipStackBean implements SipService {

    static final Log LOG = LogFactory.getLog(SipServiceImpl.class);

  
    public void sendCheckSync(String addrSpec) {
        AbstractMessage message = new NotifyMessage(this, addrSpec, "check-sync");
        message.createAndSend();
    }

    public void sendNotify(String addrSpec, String eventType, String contentType, byte[] payload) {
        AbstractMessage message = new NotifyMessage(this, addrSpec, eventType, contentType,
                payload);
        message.createAndSend();
    }


    public void sendRefer(String sourceAddrSpec, String destinationAddrSpec) {
        LOG.debug("sendRefer: source = " + sourceAddrSpec + " dest = " + destinationAddrSpec);
        AbstractMessage message = new InviteMessage(this, destinationAddrSpec, sourceAddrSpec,
                Operator.SEND_3PCC_REFER_CALL_SETUP);
        message.createAndSend();
    }

}
