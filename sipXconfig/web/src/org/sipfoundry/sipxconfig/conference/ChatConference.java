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

import org.sipfoundry.sipxconfig.setting.Setting;

public class ChatConference {

    private final Conference m_conference;

    public ChatConference(Conference conference) {
        m_conference = conference;
    }
    public Setting getSettings() {
        return m_conference.getSettings().getSetting("chat-meeting");
    }

}
