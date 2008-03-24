/*
 *  Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 *  Contributors retain copyright to elements licensed under a Contributor Agreement.
 *  Licensed to the User under the LGPL license.
 *
 */
package org.sipfoundry.sipxbridge;

import java.util.TimerTask;

import javax.sip.SipProvider;
import javax.sip.message.Request;

import org.apache.log4j.Logger;

/**
 * A Timer task for re-issuing OPTIONS requests for keep alive.
 * 
 * @author M. Ranganathan
 */

public class OptionsTimerTask extends TimerTask {

    SipProvider provider;

    ItspAccountInfo itspAccount;

    private static Logger logger = Logger.getLogger(OptionsTimerTask.class);

    public OptionsTimerTask(SipProvider provider, ItspAccountInfo itspAccount) {

        if (itspAccount.getOptionsTimerTask() != null) {
            itspAccount.getOptionsTimerTask().cancel();
        }
        itspAccount.setOptionsTimerTask(this);
        this.provider = provider;
        this.itspAccount = itspAccount;

    }

    @Override
    public void run() {
        try {
            Request options = SipUtilities.createOptionsRequest(provider,
                    itspAccount);
            provider.sendRequest(options);

        } catch (Exception ex) {
            logger.error("Unepxected error sending options", ex);
        }
    }

}
