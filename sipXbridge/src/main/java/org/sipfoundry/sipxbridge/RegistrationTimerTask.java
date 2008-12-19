/*
 *  Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 *  Contributors retain copyright to elements licensed under a Contributor Agreement.
 *  Licensed to the User under the LGPL license.
 *
 */
package org.sipfoundry.sipxbridge;

import java.util.TimerTask;

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
        this.itspAccount.registrationTimerTask = this;

    }

    @Override
    public void run() {

        try {
            Gateway.getRegistrationManager().sendRegistrer(itspAccount);
            this.itspAccount.registrationTimerTask = null;
        } catch (Exception ex) {
            throw new RuntimeException("Unexpected exception", ex);
        }
    }

}
