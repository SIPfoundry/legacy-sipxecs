/*
 *  Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 *  Contributors retain copyright to elements licensed under a Contributor Agreement.
 *  Licensed to the User under the LGPL license.
 *
 */
package org.sipfoundry.sipxbridge;

import java.util.TimerTask;

import org.apache.log4j.Logger;

/**
 * Issues a re-registration after time expires.
 * 
 * @author mranga
 * 
 */
public class RegistrationTimerTask extends TimerTask {
	private static Logger logger = Logger.getLogger(RegistrationTimerTask.class);

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
        	/*
        	 * Dont throw runtime exception here! It will kill the Timer.
        	 */
        	logger.error("Unexpected Exception in timer task",ex);
        }
    }

}
