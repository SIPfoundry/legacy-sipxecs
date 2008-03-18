/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 * 
 * 
 */
package org.sipfoundry.sipxconfig.site.common;

import org.apache.hivemind.Messages;
import org.apache.tapestry.IComponent;
import org.apache.tapestry.services.ComponentMessagesSource;

public class JarMessagesSourceServiceAdapter implements ComponentMessagesSource {

    private JarMessagesSource m_jarMessagesSource;
    private ComponentMessagesSource m_systemMessagesSource;

    public void setJarMessagesSource(JarMessagesSource jarMessagesSource) {
        m_jarMessagesSource = jarMessagesSource;
    }

    public void setSystemMessagesSource(ComponentMessagesSource systemMessagesSource) {
        m_systemMessagesSource = systemMessagesSource;
    }

    public Messages getMessages(IComponent component) {
        Messages messages = m_jarMessagesSource.getMessages(component);
        if (messages != null) {
            return messages;
        }
        return m_systemMessagesSource.getMessages(component);
    }
}
