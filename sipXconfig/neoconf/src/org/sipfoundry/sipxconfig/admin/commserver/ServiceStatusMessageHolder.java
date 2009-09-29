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

import java.util.ArrayList;
import java.util.List;
import java.util.regex.Pattern;


public class ServiceStatusMessageHolder {
    public static final Pattern PATTERN_STDERR = Pattern.compile("^stderr\\.msg-(\\d)*:");
    public static final Pattern PATTERN_STDOUT = Pattern.compile("^stdout\\.msg-(\\d)*:");

    private List<ServiceStatusMessage> m_statusMessages = new ArrayList<ServiceStatusMessage>();
    private List<ServiceStatusMessage> m_stdOut = new ArrayList<ServiceStatusMessage>();
    private List<ServiceStatusMessage> m_stdErr = new ArrayList<ServiceStatusMessage>();

    public List<ServiceStatusMessage> getStatusMessages() {
        return m_statusMessages;
    }

    public List<ServiceStatusMessage> getStdOut() {
        return m_stdOut;
    }

    public List<ServiceStatusMessage> getStdErr() {
        return m_stdErr;
    }

    public List<ServiceStatusMessage> getAllMessages() {
        List<ServiceStatusMessage> allMessages = new ArrayList<ServiceStatusMessage>();
        allMessages.addAll(m_statusMessages);
        allMessages.addAll(m_stdOut);
        allMessages.addAll(m_stdErr);

        return allMessages;
    }

    public ServiceStatusMessage getFirstMessage() {
        return getAllMessages().get(0);
    }

    public void addMessage(String message) {
        if (PATTERN_STDERR.matcher(message).find()) {
            m_stdErr.add(new ServiceStatusMessage(message));
        } else if (PATTERN_STDOUT.matcher(message).find()) {
            m_stdOut.add(new ServiceStatusMessage(message));
        } else {
            m_statusMessages.add(new ServiceStatusMessage(message));
        }
    }
}
