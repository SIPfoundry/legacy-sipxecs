/*
 *  Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 *  Contributors retain copyright to elements licensed under a Contributor Agreement.
 *  Licensed to the User under the LGPL license.
 *
 */
package org.sipfoundry.sipxbridge;

import java.util.TimerTask;

import javax.sip.ClientTransaction;
import javax.sip.SipProvider;
import javax.sip.message.Request;

/**
 * Issues a re-registration after time expires.
 * 
 * @author mranga
 * 
 */
public class RegistrationTimerTask extends TimerTask {

    ItspAccountInfo itspAccount;

    public RegistrationTimerTask(ItspAccountInfo itspAccount) {
        this.itspAccount = itspAccount;

    }

    @Override
    public void run() {

        try {
            Gateway.getRegistrationManager().sendRegistrer(itspAccount);
        } catch (Exception ex) {
            throw new RuntimeException("Unexpected exception", ex);
        }
    }

}
