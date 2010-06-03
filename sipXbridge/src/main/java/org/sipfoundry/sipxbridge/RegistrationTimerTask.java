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
    long cseq;
    String callId;

    public RegistrationTimerTask(ItspAccountInfo itspAccount, String callId, long cseq) {
        this.itspAccount = itspAccount;
        this.itspAccount.registrationTimerTask = this;
        this.callId = callId;
        this.cseq = cseq;

    }

    @Override
    public void run() {

        try {
            this.itspAccount.registrationTimerTask = null;
            Gateway.getRegistrationManager().sendRegister(itspAccount,callId,cseq+1);
        } catch (Exception ex) {
            RegistrationTimerTask ttask = new RegistrationTimerTask(itspAccount, null, 1L);
            Gateway.getTimer().schedule(ttask, 60 * 1000);
            try {
                if (!itspAccount.isAlarmSent()) {
                    Gateway.getAlarmClient().raiseAlarm(
                            Gateway.SIPXBRIDGE_ACCOUNT_CONFIGURATION_ERROR_ALARM_ID,
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
