/*
 *
 *
 * Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 * 
 *
 */
package org.sipfoundry.callcontroller;

import gov.nist.javax.sip.DialogExt;

import java.util.TimerTask;

import javax.sip.ClientTransaction;
import javax.sip.Dialog;
import javax.sip.DialogState;
import javax.sip.SipProvider;
import javax.sip.message.Request;

import org.apache.log4j.Logger;

public class ReferTimerTask extends TimerTask {
    private Dialog m_dialog;
    private static Logger logger = Logger.getLogger(ReferTimerTask.class);

    public ReferTimerTask(Dialog dialog) {
        m_dialog = dialog;
    }

    @Override
    public void run() {
        try {
            if (m_dialog.getState() != DialogState.TERMINATED) {
                Request byeRequest = m_dialog.createRequest(Request.BYE);
                SipProvider provider = ((DialogExt) m_dialog).getSipProvider();
                ClientTransaction ctx = provider.getNewClientTransaction(byeRequest);
                m_dialog.sendRequest(ctx);
            }
        } catch (Exception ex) {
            logger.error("Exception caught", ex);
        }
    }

}
