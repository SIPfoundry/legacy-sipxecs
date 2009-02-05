/*
 * 
 * 
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 * 
 * $
 */
package org.sipfoundry.sipxconfig.cdr;

import java.util.Date;

import org.sipfoundry.sipxconfig.common.SipUri;

public class Cdr {
    enum Termination {
        UNKNOWN, REQUESTED, IN_PROGRESS, COMPLETED, FAILED, TRANSFER;

        public static Termination fromString(String t) {
            switch (t.charAt(0)) {
            case 'R':
                return REQUESTED;
            case 'C':
                return COMPLETED;
            case 'F':
                return FAILED;
            case 'I':
                return IN_PROGRESS;
            case 'U':
                return UNKNOWN;
            case 'T':
                return TRANSFER;
            default:
                return UNKNOWN;
            }
        }
    }

    private String m_callerAor;
    private String m_calleeAor;

    private Date m_startTime;
    private Date m_connectTime;
    private Date m_endTime;

    private Termination m_termination;
    private int m_failureStatus;

    private String m_caller;

    private String m_callee;

    public String getCalleeAor() {
        return m_calleeAor;
    }

    public String getCallee() {
        return m_callee;
    }

    public void setCalleeAor(String calleeAor) {
        m_calleeAor = calleeAor;
        m_callee = SipUri.extractUser(calleeAor);
    }

    public String getCallerAor() {
        return m_callerAor;
    }

    public String getCaller() {
        return m_caller;
    }

    public void setCallerAor(String callerAor) {
        m_callerAor = callerAor;
        m_caller = SipUri.extractFullUser(callerAor);
    }

    public Date getConnectTime() {
        return m_connectTime;
    }

    public void setConnectTime(Date connectTime) {
        m_connectTime = connectTime;
    }

    public Date getEndTime() {
        return m_endTime;
    }

    public void setEndTime(Date endTime) {
        m_endTime = endTime;
    }

    public int getFailureStatus() {
        return m_failureStatus;
    }

    public void setFailureStatus(int failureStatus) {
        m_failureStatus = failureStatus;
    }

    public Date getStartTime() {
        return m_startTime;
    }

    public void setStartTime(Date startTime) {
        m_startTime = startTime;
    }

    public Termination getTermination() {
        return m_termination;
    }

    public void setTermination(Termination termination) {
        m_termination = termination;
    }

    public long getDuration() {
        if (m_endTime == null || m_connectTime == null) {
            return 0;
        }
        return m_endTime.getTime() - m_connectTime.getTime();
    }
}
