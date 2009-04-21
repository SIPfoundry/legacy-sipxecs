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
            RegistrationTimerTask ttask = new RegistrationTimerTask(itspAccount);
            Gateway.getTimer().schedule(ttask, 60 * 1000);
            try {
                if (!itspAccount.isAlarmSent()) {
                    Gateway.getAlarmClient().raiseAlarm(
                            "SIPX_BRIDGE_OPERATION_TIMED_OUT", 
                            itspAccount.getSipDomain());
                    itspAccount.setAlarmSent(true);
                }
            } catch (Exception e) {
                logger.error("Could not send alarm.", e);
            }
            /*
             * Dont throw runtime exception here! It will kill the Timer.
             */
            logger.error("Unexpected Exception Sending registration in timer task",ex);
        }
    }

  

}
