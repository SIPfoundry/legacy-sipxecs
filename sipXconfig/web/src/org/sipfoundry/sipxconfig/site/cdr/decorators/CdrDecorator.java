/*
 *
 *
 * Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.site.cdr.decorators;

import java.io.Serializable;
import java.util.Date;
import java.util.Locale;

import org.apache.commons.lang.StringUtils;
import org.apache.hivemind.Messages;
import org.sipfoundry.sipxconfig.cdr.Cdr;
import org.sipfoundry.sipxconfig.components.MillisDurationFormat;
import org.sipfoundry.sipxconfig.components.NewEnumFormat;

public class CdrDecorator implements Serializable {
    private Cdr m_cdr;

    private Locale m_locale;

    private Messages m_messages;

    public CdrDecorator(Cdr cdr, Locale locale, Messages messages) {
        m_cdr = cdr;
        m_locale = locale;
        m_messages = messages;
    }

    public String getCalleeAor() {
        return StringUtils.defaultString(m_cdr.getCalleeAor());
    }

    public String getCallee() {
        return StringUtils.defaultString(m_cdr.getCallee());
    }

    public String getCallerAor() {
        return StringUtils.defaultString(m_cdr.getCalleeAor());
    }

    public String getCaller() {
        return StringUtils.defaultString(m_cdr.getCaller());
    }

    public String getRecipient() {
        String recipient = m_cdr.getRecipient();
        String calltypeshortname = m_cdr.getCallTypeShortName();
        if (calltypeshortname == null) {
            return StringUtils.defaultString(recipient);
        } else {
            return StringUtils.defaultString(recipient + "(" + calltypeshortname + ")");
        }
    }

    public Date getConnectTime() {
        return m_cdr.getConnectTime();
    }

    public Date getEndTime() {
        return m_cdr.getEndTime();
    }

    public int getFailureStatus() {
        return m_cdr.getFailureStatus();
    }

    public Date getStartTime() {
        return m_cdr.getStartTime();
    }

    public String getTermination() {
        NewEnumFormat format = new NewEnumFormat();
        format.setMessages(m_messages);
        format.setPrefix("state");
        return format.format(m_cdr.getTermination());
    }

    public String getDuration() {
        MillisDurationFormat format = new MillisDurationFormat();
        format.setMaxField(1);
        format.setShowZero(true);
        format.setSeparator(":");
        return format.format(new Long(m_cdr.getDuration()));
    }

    public String getCallDirection() {
        return m_cdr.getCallDirection();
    }

    public String getCalleeRoute() {
        return m_cdr.getCalleeRoute();
    }

    public String getCallTypeName() {
        return m_cdr.getCallTypeName();
    }
}
