/*
 *
 *
 * Copyright (C) 2008-2009 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 */
package org.sipfoundry.commons.freeswitch;

import java.io.UnsupportedEncodingException;
import java.net.URLDecoder;

public class Collect extends CallCommand {
    private boolean m_done;
    private Break m_breaker;
    private boolean m_stopped;
    private String m_digitMask = "1234567890ABCD#*i";
    private String m_termChars = "#";
    private int m_firstDigitTimer;
    private int m_interDigitTimer;
    private int m_extraDigitTimer;
    private int m_maxDigits;

    private String m_digits = "";
    private int m_currentTimer;

    public Collect(FreeSwitchEventSocketInterface fses, int maxDigits, int firstDigitTimer,
            int interDigitTimer, int extraDigitTimer) {
        super(fses);
        this.m_maxDigits = maxDigits;
        this.m_firstDigitTimer = firstDigitTimer;
        this.m_interDigitTimer = interDigitTimer;
        this.m_extraDigitTimer = extraDigitTimer;
    }

    public void setDigitMask(String digitMask) {
        this.m_digitMask = digitMask;
    }

    public void setTermChars(String termChars) {
        this.m_termChars = termChars;
    }

    String timerCmd(int delay) {
    	if (delay <= 0) {
    		delay = 1;
    	}
        return "playback\nexecute-app-arg: silence_stream://" + delay;
    }

    @Override
    public boolean start() {
        LOG.info(String.format("Collect::start %d %d/%d/%d mask %s", m_maxDigits,
                m_firstDigitTimer, m_interDigitTimer, m_extraDigitTimer, m_digitMask));
        m_stopped = false;
        m_digits = "";
        m_currentTimer = m_firstDigitTimer;

        m_fses.trimDtmfQueue(m_digitMask);
        String digit;
        m_done = false;
        for (; !m_done;) {
            digit = m_fses.getDtmfDigit();
            if (digit == null) {
                break;
            }
            m_done = handleDigit(digit);
        }

        if (m_done) {
            // nothing further to do
            return true;
        }

        // Start the timer
        m_command = timerCmd(m_currentTimer);

        // If timer expires, then
        m_done = true;

        return super.start();
    }

    boolean handleDigit(String digit) {
        boolean terminate = false;

        // Check if a term char was pressed
        if (m_termChars.contains(digit)) {
            terminate = true;
        }

        // Don't add term chars pressed after digits (but not by themselves)
        if (terminate && m_digits.length() > 0) {
            m_done = true;
            return true;
        }

        if (!terminate && m_digits.length() == m_maxDigits) {
	        // put digit back to the DTMF queue
	        m_fses.pushDtmfQueue(digit);
	        m_done = true ;
	        return true;
        }

        // Add digit to list of digits
        m_digits += digit;

        // Are we finished?
        if (terminate || m_digits.length() > m_maxDigits) {
            m_done = true;
            return true;
        }

        // Nope. Start the next timer
        if (m_digits.length() == m_maxDigits) {
            // Have enough digits, wait for a term char
            m_currentTimer = m_extraDigitTimer;
        } else {
            // Wait for the next digit
            m_currentTimer = m_interDigitTimer;
        }

        if (m_currentTimer <= 0) {
        	// Next timer is "immediate", so give up now
        	m_done = true;
        	return true;
        }

        return false;
    }

    public String getDigits() {
        return m_digits;
    }

    @Override
    public boolean handleEvent(FreeSwitchEvent event) {
        if (m_breaker != null) {
            // Feed all events to breaker until it is finished.
            if (m_breaker.handleEvent(event)) {
                m_breaker = null;
            }
            return false;
        }

        if (event.getEventValue("Event-Name", "").contentEquals("DTMF")) {
            String encodedDigit = event.getEventValue("DTMF-Digit");
            assert (encodedDigit != null);
            String digit;
            try {
                digit = URLDecoder.decode(encodedDigit, "UTF-8");
            } catch (UnsupportedEncodingException e) {
                LOG.error("Collect::handleEvent cannot decode encoded DTMF digit "+encodedDigit, e);
                digit = "";
            }
            String duration = event.getEventValue("DTMF-Duration", "(Unknown)");
            LOG.debug(String.format("DTMF event %s %s", m_fses.redact(digit), duration));
            if (m_digitMask.contains(digit)) {
                // Timer did not expire
                m_done = false;

                // Add digit to the list
                handleDigit(digit);

                // Stop the current command.
                m_breaker = new Break(m_fses);
                m_breaker.start();
            }
            return false;
        }

        if (super.handleEvent(event)) {
            // Command is finished.  Is Collection complete?
            if (m_done) {
                return true;
            }

            // Nope.  Start the next timer for more digits
            m_done = true;
            m_command = timerCmd(m_currentTimer);
            super.start();
        }

        return false;
    }

}
