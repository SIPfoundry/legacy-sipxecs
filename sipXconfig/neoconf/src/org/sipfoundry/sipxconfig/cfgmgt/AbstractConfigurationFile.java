/*
 * Copyright (C) 2011 eZuce Inc., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the AGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.cfgmgt;

import java.io.IOException;
import java.util.Collection;
import java.util.Iterator;

import org.sipfoundry.sipxconfig.setting.AbstractSettingVisitor;
import org.sipfoundry.sipxconfig.setting.Setting;

/**
 * No particular format, but common methods for writing out settings and single key,value pairs
 */
public abstract class AbstractConfigurationFile {
    private IOException m_error;
    private String m_globalPrefix;

    public AbstractConfigurationFile() {
    }

    public AbstractConfigurationFile(String prefix) {
        m_globalPrefix = prefix;
    }

    public void write(String prefix, Collection<Setting> settings) throws IOException {
        Iterator<Setting> i = settings.iterator();
        while (i.hasNext()) {
            write(prefix, i.next());
        }
    }

    public void write(Collection<Setting> settings) throws IOException {
        write(m_globalPrefix, settings);
    }

    public void write(Setting settings) throws IOException {
        settings.acceptVisitor(new SettingsWriter(m_globalPrefix));
        if (m_error != null) {
            throw m_error;
        }
    }

    public void write(String prefix, Setting settings) throws IOException {
        settings.acceptVisitor(new SettingsWriter(prefix));
        if (m_error != null) {
            throw m_error;
        }
    }

    public void write(String key, Object value) throws IOException {
        write(m_globalPrefix, key, value);
    }

    public abstract void write(String prefix, String key, Object value) throws IOException;

    class SettingsWriter extends AbstractSettingVisitor {
        private String m_prefix;

        SettingsWriter() {
        }

        SettingsWriter(String prefix) {
            m_prefix = prefix;
        }

        @Override
        public void visitSetting(Setting setting) {
            try {
                write(m_prefix, setting.getName(), setting.getValue());
            } catch (IOException e) {
                m_error = e;
            }
        }
    }
}
