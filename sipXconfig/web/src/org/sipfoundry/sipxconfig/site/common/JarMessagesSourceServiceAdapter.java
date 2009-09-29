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

import java.util.Locale;

import org.apache.hivemind.Messages;
import org.apache.hivemind.impl.AbstractMessages;
import org.apache.tapestry.IComponent;
import org.apache.tapestry.services.ComponentMessagesSource;
import org.sipfoundry.sipxconfig.components.LocalizationUtils;

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
            return new FallbackMessages(messages, m_systemMessagesSource.getMessages(component), component.getPage()
                    .getLocale());
        }
        return m_systemMessagesSource.getMessages(component);
    }

    private static class FallbackMessages extends AbstractMessages {

        private Messages m_fallbackMessages;
        private Messages m_localeMessages;
        private Locale m_locale;

        public FallbackMessages(Messages localeMessages, Messages fallbackMessages, Locale locale) {
            m_localeMessages = localeMessages;
            m_fallbackMessages = fallbackMessages;
            m_locale = locale;
        }

        protected String findMessage(String key) {
            String message = LocalizationUtils.getMessage(m_localeMessages, key, null);
            if (message == null) {
                message = m_fallbackMessages.getMessage(key);
            }
            return message;
        }

        protected Locale getLocale() {
            return m_locale;
        }
    }
}
