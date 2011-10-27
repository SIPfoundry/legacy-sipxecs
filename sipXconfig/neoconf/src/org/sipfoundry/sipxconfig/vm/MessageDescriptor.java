/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.vm;

import java.util.Date;
import java.util.List;

import org.sipfoundry.sipxconfig.common.SipUri;

import com.thoughtworks.xstream.annotations.XStreamAlias;
import com.thoughtworks.xstream.annotations.XStreamImplicit;

@XStreamAlias("messagedescriptor")
public class MessageDescriptor {
    static final String TIMESTAMP_FORMAT = "EEE, d-MMM-yyyy hh:mm:ss aaa z";
    // see XCF-1519
    static final String TIMESTAMP_FORMAT_NO_ZONE = "EEE, d-MMM-yyyy hh:mm:ss aaa";

    private Date m_timestamp;
    private int m_durationsecs;
    private String m_subject;
    private String m_from;
    private String m_priority;
    private String m_id;

    @XStreamImplicit(itemFieldName = "otherrecipient")
    private List<String> m_otherRecipients;

    public int getDurationsecs() {
        return m_durationsecs;
    }

    public int getDurationMillis() {
        return getDurationsecs() * 1000;
    }

    public List<String> getOtherRecipients() {
        return m_otherRecipients;
    }

    public String getFrom() {
        return m_from;
    }

    public String getFromBrief() {
        return SipUri.extractFullUser(getFrom().replace('+', ' '));
    }

    public String getId() {
        return m_id;
    }

    public String getPriority() {
        return m_priority;
    }

    public String getSubject() {
        return m_subject;
    }

    void setSubject(String subject) {
        m_subject = subject;
    }

    public Date getTimestamp() {
        return m_timestamp;
    }
}
