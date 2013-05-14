/**
 *
 *
 * Copyright (c) 2012 eZuce, Inc. All rights reserved.
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
package org.sipfoundry.sipxconfig.vm;

import java.util.Date;
import java.util.TimeZone;

import org.joda.time.DateTimeZone;
import org.joda.time.LocalDateTime;
import org.sipfoundry.commons.util.TimeZoneUtils;
import org.w3c.dom.Element;

public class RemoteVoicemail implements Voicemail, Comparable {
    private String m_folderId;
    private String m_userId;
    private String m_messageId;
    private boolean m_heard;
    private String m_subject;
    private String m_from;
    private String m_fromBrief;
    private String m_forwardedFromBrief;
    private Date m_timestamp;
    private Date m_forwardedTimestamp;
    private int m_durationSecs;
    private boolean m_forwarded;
    private String m_forwardedSubject;

    public RemoteVoicemail(Element node, String userId, String folder, TimeZone tz) {
        m_userId = userId;
        m_folderId = folder;
        m_messageId = node.getAttribute("id");
        m_heard = Boolean.valueOf(node.getAttribute("heard"));
        m_durationSecs = Integer.valueOf(node.getAttribute("duration"));
        m_timestamp = TimeZoneUtils.convertJodaTimezone(new LocalDateTime(new Long(node.getAttribute("received"))),
                DateTimeZone.getDefault().getID(), tz.getID());
        m_from = node.getAttribute("fromUri");
        m_fromBrief = node.getAttribute("author");
        m_subject = node.getAttribute("subject");
    }

    public int compareTo(Object o) {
        if (o == null || o instanceof RemoteVoicemail) {
            return -1;
        }
        return getMessageId().compareTo(((RemoteVoicemail) o).getMessageId());
    }

    @Override
    public String getFolderId() {
        return m_folderId;
    }

    @Override
    public String getUserId() {
        return m_userId;
    }

    @Override
    public String getMessageId() {
        return m_messageId;
    }

    @Override
    public boolean isHeard() {
        return m_heard;
    }

    public void setSubject(String subject) {
        m_subject = subject;
    }

    @Override
    public String getSubject() {
        return m_subject;
    }

    @Override
    public String getFromBrief() {
        return m_fromBrief;
    }

    @Override
    public Date getTimestamp() {
        return m_timestamp;
    }

    @Override
    public int getDurationsecs() {
        return m_durationSecs;
    }

    @Override
    public boolean isForwarded() {
        return m_forwarded;
    }

    @Override
    public String getForwardedSubject() {
        return m_forwardedSubject;
    }

    @Override
    public String getForwardedFromBrief() {
        return m_forwardedFromBrief;
    }

    @Override
    public Date getForwardedTimestamp() {
        return m_forwardedTimestamp;
    }

    @Override
    public String getFrom() {
        return m_from;
    }

    @Override
    public int getForwardedDurationsecs() {
        return 0;
    }
}
