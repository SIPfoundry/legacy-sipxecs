/*
 *
 *
 * Copyright (C) 2009 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 *
 */

package org.sipfoundry.sipxconfig.conference;

import java.io.UnsupportedEncodingException;
import java.net.URLEncoder;
import java.util.ArrayList;
import java.util.List;
import java.util.Map;

import org.apache.commons.lang.StringUtils;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.setting.Setting;

/**
 * Adaptor for conference object that allows interacting with DimDim APIs
 */
public class DimDimConference {

    private static final String INTERTOLL = "internToll";

    private final Conference m_conference;

    public DimDimConference(Conference conference) {
        m_conference = conference;
    }

    public boolean isConfigured() {
        return getUser() != null && StringUtils.isNotBlank(getPassword());
    }

    public String getSignupUrl() {
        return "http://www.dimdim.com";
    }

    public Setting getSettings() {
        return m_conference.getSettings().getSetting("web-meeting");
    }

    public String getDimDimHost() {
        return m_conference.getSettingValue("web-meeting/dimdim-host").toString();
    }

    public String getUser() {
        String user = m_conference.getSettingValue("web-meeting/user");
        if (StringUtils.isNotBlank(user)) {
            return user;
        }
        User owner = m_conference.getOwner();
        if (owner != null) {
            return owner.getUserName();
        }
        return null;
    }

    String getDisplayName() {
        User owner = m_conference.getOwner();
        if (owner == null) {
            return null;
        }
        return owner.getDisplayName();
    }

    public String getPassword() {
        return m_conference.getSettingValue("web-meeting/password");
    }

    public String getDid() {
        return m_conference.getSettingValue("web-meeting/did");
    }

    public String getAttendeePwd() {
        return m_conference.getParticipantAccessCode();
    }

    public String getAttendeePasscode() {
        return m_conference.getParticipantAccessCode();
    }

    public String getConfName() {
        return m_conference.getName();
    }

    public String getAgenda() {
        return m_conference.getDescription();
    }
    // attendeePwd, Attendee Password, which Attendees need to know this password
    // to join the Dimdim Meeting.
    private void addAttendeePwd(Map<String, String> params) {
        final String accessCode = m_conference.getParticipantAccessCode();
        if (StringUtils.isNotBlank(accessCode)) {
            params.put("attendeePwd", accessCode);
        }
    }

    // attendeePasscode, Attendee pass code, which Attendees use to access audio bridge.
    private void addAttendeePasscode(Map<String, String> params) {
        final String accessCode = m_conference.getParticipantAccessCode();
        if (StringUtils.isNotBlank(accessCode)) {
            params.put("attendeePasscode", accessCode);
        }
    }

    private String paramsToQuery(Map<String, String> params) {
        try {
            List<String> items = new ArrayList<String>(params.size());
            for (Map.Entry<String, String> pe : params.entrySet()) {
                final String value = pe.getValue();
                if (StringUtils.isBlank(value)) {
                    items.add(pe.getKey());
                } else {
                    final String encodedValue = URLEncoder.encode(value, "UTF-8");
                    items.add(String.format("%s=%s", pe.getKey(), encodedValue));
                }
            }
            return StringUtils.join(items, '&');
        } catch (UnsupportedEncodingException e) {
            // UTF-8 is a default supported encoding...
            throw new RuntimeException(e);
        }
    }
}
