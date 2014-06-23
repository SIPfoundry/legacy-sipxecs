/**
 * Copyright (c) 2014 eZuce, Inc. All rights reserved.
 * Contributed to SIPfoundry under a Contributor Agreement
 *
 * This software is free software; you can redistribute it and/or modify it under
 * the terms of the Affero General Public License (AGPL) as published by the
 * Free Software Foundation; either version 3 of the License, or (at your option)
 * any later version.
 *
 * This software is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more
 * details.
 */
package org.sipfoundry.sipxconfig.api.model;

import java.text.DateFormat;
import java.util.Date;

import javax.xml.bind.annotation.XmlRootElement;
import javax.xml.bind.annotation.XmlTransient;

import org.joda.time.Period;
import org.sipfoundry.sipxconfig.cdr.Cdr;

@XmlRootElement(name = "Cdr")
public class CdrBean {
    private String m_callId;
    private String m_caller;
    private String m_callerAor;
    private String m_callee;
    private String m_calleeAor;
    private String m_recipient;
    private String m_termination;
    private String m_duration;
    @XmlTransient
    private Date m_startTime;
    private String m_startAt;
    private String m_connectAt;

    public String getCaller() {
        return m_caller;
    }

    public void setCaller(String caller) {
        m_caller = caller;
    }

    public String getCallerAor() {
        return m_callerAor;
    }

    public void setCallerAor(String callerAor) {
        m_callerAor = callerAor;
    }

    public String getCallee() {
        return m_callee;
    }

    public void setCallee(String callee) {
        m_callee = callee;
    }

    public String getCalleeAor() {
        return m_calleeAor;
    }

    public void setCalleeAor(String calleeAor) {
        m_calleeAor = calleeAor;
    }

    public String getTermination() {
        return m_termination;
    }

    public void setTermination(String termination) {
        m_termination = termination;
    }

    public String getRecipient() {
        return m_recipient;
    }

    public void setRecipient(String recipient) {
        m_recipient = recipient;
    }

    public String getCallId() {
        return m_callId;
    }

    public void setCallId(String callId) {
        m_callId = callId;
    }

    public String getStartAt() {
        return m_startAt;
    }

    public void setStartAt(String startTime) {
        m_startAt = startTime;
    }

    public String getConnectAt() {
        return m_connectAt;
    }

    public void setConnectAt(String connectTime) {
        m_connectAt = connectTime;
    }

    public String getDuration() {
        return m_duration;
    }

    public void setDuration(String duration) {
        m_duration = duration;
    }

    @XmlTransient
    public Date getStartTime() {
        return m_startTime;
    }

    public void setStartTime(Date time) {
        m_startTime = time;
    }

    public static CdrBean convertCdr(Cdr cdr, DateFormat dateFormat) {
        CdrBean bean = new CdrBean();
        bean.setCallId(cdr.getCallId());
        bean.setCaller(cdr.getCaller());
        bean.setCallerAor(cdr.getCallerAor());
        bean.setCallee(cdr.getCallee());
        bean.setCalleeAor(cdr.getCalleeAor());
        bean.setTermination(cdr.getTermination().name());
        bean.setRecipient(cdr.getRecipient());

        Date start = cdr.getStartTime();
        bean.setStartTime(start);
        if (start != null) {
            bean.setStartAt(dateFormat.format(start));
        }

        Date connect = cdr.getConnectTime();
        if (connect != null) {
            bean.setConnectAt(dateFormat.format(connect));
        }

        Period period = new Period(cdr.getDuration());
        bean.setDuration(String.format("%02d:%02d:%02d", period.getHours(), period.getMinutes(), period.getSeconds()));
        return bean;
    }

}
