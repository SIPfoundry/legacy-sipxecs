/*
 *
 *
 * Copyright (C) 2008-2009 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 */
package org.sipfoundry.sipxivr;

public class Sleep extends CallCommand {

    private String m_digitMask = "";
    private Break m_breaker;

    public Sleep(FreeSwitchEventSocketInterface fses, int mS) {
        super(fses);
        m_command = timerCmd(mS);
    }

    public void setDigitMask(String digitMask) {
        this.m_digitMask = digitMask;
    }

    String timerCmd(int delay) {
        return "playback\nexecute-app-arg: silence_stream://" + delay;
    }

    @Override
    public boolean start() {
        // Found a bargable digit in the DTMF queue
        if (m_fses.trimDtmfQueue(m_digitMask)) {
            return true;
        }

        return super.start();
    }

    @Override
    public boolean handleEvent(FreeSwitchEvent event) {
        if (m_breaker != null) {
            if (m_breaker.handleEvent(event)) {
                m_breaker = null;
                return m_finished;
            }
        }
        if (event.getEventValue("Event-Name", "").contentEquals("DTMF")) {
            String digit = event.getEventValue("DTMF-Digit");
            assert (digit != null);
            String duration = event.getEventValue("DTMF-Duration", "(Unknown)");
            LOG.debug(String.format("DTMF event %s %s", digit, duration));
            if (m_digitMask.contains(digit)) {
                // Add digit to the Dtmf queue
                m_fses.appendDtmfQueue(digit);

                // End the sleep.
                m_breaker = new Break(m_fses);
                m_breaker.start();
                return m_finished;
            }
        }
        return super.handleEvent(event);
    }

}
