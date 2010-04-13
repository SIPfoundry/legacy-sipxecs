/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.site.skin;

import java.util.Locale;

import org.apache.hivemind.Messages;
import org.apache.hivemind.impl.AbstractMessages;
import org.apache.tapestry.IComponent;
import org.apache.tapestry.services.ComponentMessagesSource;

/**
 * Skins can load their own resource strings depending on client locale of web browser
 */
public class SkinMessagesSource implements ComponentMessagesSource {

    private ComponentMessagesSource m_systemMessagesSource;
    private SkinControl m_skin;

    public void setSkin(SkinControl skin) {
        m_skin = skin;
    }

    public void setSystemMessagesSource(ComponentMessagesSource systemMessagesSource) {
        m_systemMessagesSource = systemMessagesSource;
    }

    public Messages getMessages(IComponent component) {
        return new SkinMessages(m_systemMessagesSource.getMessages(component), component
                .getPage().getLocale(), m_skin);
    }

    static class SkinMessages extends AbstractMessages {
        private Messages m_systemMessages;
        private SkinControl m_skin;
        private Locale m_locale;

        SkinMessages(Messages systemMessages, Locale locale, SkinControl skin) {
            m_systemMessages = systemMessages;
            m_locale = locale;
            m_skin = skin;
        }

        @Override
        protected String findMessage(String key) {
            String defaultMessage = m_systemMessages.getMessage(key);
            return m_skin.getLocalizeString(key, m_locale, defaultMessage);
        }

        @Override
        protected Locale getLocale() {
            return m_locale;
        }
    }
}
