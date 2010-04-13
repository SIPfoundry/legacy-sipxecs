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
import java.util.ListIterator;

public class Record extends CallCommand {
    private PromptList m_prompts;
    private String m_digitMask = "0123456789#*i";
    private boolean m_stopped;
    private ListIterator<String> m_iter;
    private Break m_breaker;
    private String m_recordFile;
    private int m_recordTime; // in seconds

    public Record(FreeSwitchEventSocketInterface fses) {
        super(fses);
    }

    public Record(FreeSwitchEventSocketInterface fses, PromptList prompts) {
        super(fses);
        setPromptList(prompts);
    }

    public Record(FreeSwitchEventSocketInterface fses, String prompts) {
        super(fses);
        PromptList p = new PromptList();
        p.addPrompts(prompts);
        setPromptList(p);
    }

    public void setPromptList(PromptList prompts) {
        this.m_prompts = prompts;
    }

    public void setDigitMask(String digitMask) {
        this.m_digitMask = digitMask;
    }

    public PromptList getPromptList() {
        return m_prompts;
    }

    public void setRecordFile(String recordFile) {
    	this.m_recordFile = recordFile;
    }

    public String getRecordFile() {
    	return m_recordFile;
    }

    public void setRecordTime(int recordTime) {
    	this.m_recordTime = recordTime;
    }

    public int getRecordTime() {
    	return m_recordTime;
    }

    @Override
    public boolean start() {
        m_finished = false;
        m_stopped = false;

        // dummy loop for break
        for (;;) {
            // Found a bargable digit in the DTMF queue
            if (m_fses.trimDtmfQueue(m_digitMask)) {
                m_finished = true;
                break;
            }

            // Start playing the first prompt
            if (m_prompts != null) {
            	m_iter = m_prompts.getPrompts().listIterator();
            }
            nextPrompt();
            break;
        }
        return m_finished;
    }

    void nextPrompt() {
        if (m_iter != null && m_iter.hasNext()) {
            m_finished = false;
            String prompt = m_iter.next();
            m_command = "playback\nexecute-app-arg: " + prompt;
            super.start();
        } else {
            startRecord();
        }
    }

    void startRecord() {
        new Set(m_fses, "record_rate","8000").go();
        new Set(m_fses, "playback_terminators", m_digitMask).go();
    	m_stopped = true;
    	m_command = String.format("record\nexecute-app-arg: %s %d 200 10", m_recordFile, m_recordTime);
    	super.start();
    }

    @Override
    public boolean handleEvent(FreeSwitchEvent event) {
        if (m_breaker != null) {
            // Feed event to breaker first, to see if it wants to handle it
            if (m_breaker.handleEvent(event)) {
                // Breaker is done.
                m_breaker = null;
                return m_finished;
            }
        }
        if (event.getEventValue("Event-Name", "").contentEquals("DTMF")) {
            String encodedDigit = event.getEventValue("DTMF-Digit");
            assert (encodedDigit != null);
            String digit;
            try {
                digit = URLDecoder.decode(encodedDigit, "UTF-8");
            } catch (UnsupportedEncodingException e) {
                LOG.error("Record::handleEvent cannot decode encoded DTMF digit "+encodedDigit, e);
                digit = "";
            }
            String duration = event.getEventValue("DTMF-Duration", "(Unknown)");
            LOG.debug(String.format("DTMF event %s %s", m_fses.redact(digit), duration));
            if (m_digitMask.contains(digit)) {
                // Add digit to the DTMF queue
                m_fses.appendDtmfQueue(digit);

                // No more prompts to play
                m_stopped = true;

                // Stop the currently playing prompt.
                m_breaker = new Break(m_fses);
                m_breaker.start();
                return m_finished;
            }
        }
        m_finished = super.handleEvent(event);
        if (m_finished) {
            if (!m_stopped) {
                // On to the next prompt (if there is one)
                nextPrompt();
            }
        }
        return m_finished;
    }
}
