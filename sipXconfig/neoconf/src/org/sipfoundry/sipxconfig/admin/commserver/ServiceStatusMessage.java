/*
 *
 *
 * Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */

package org.sipfoundry.sipxconfig.admin.commserver;

import java.util.regex.Pattern;

public class ServiceStatusMessage {
    public static final Pattern PATTERN_MESSAGE_DELIMITER = Pattern.compile("(\\-[0-9]*)?:");

    private String m_prefix;
    private String m_message;

    public ServiceStatusMessage(String line) {
        String[] splitMessage = line.split(PATTERN_MESSAGE_DELIMITER.pattern(), 2);

        if (splitMessage.length != 2) {
            throw new IllegalArgumentException("Invalid status message string passed. Must be \"prefix: message\"");
        }

        m_prefix = splitMessage[0].trim();
        m_message = splitMessage[1].trim();
    }

    public void setPrefix(String prefix) {
        m_prefix = prefix;
    }

    public String getPrefix() {
        return m_prefix;
    }

    public void setMessage(String message) {
        m_message = message;
    }

    public String getMessage() {
        return m_message;
    }
}
