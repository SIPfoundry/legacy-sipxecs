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

    public void writeSettings(String prefix, Collection<Setting> settings) throws IOException {
        Iterator<Setting> i = settings.iterator();
        while (i.hasNext()) {
            writeSettings(prefix, i.next());
        }
    }

    public void writeSettings(Collection<Setting> settings) throws IOException {
        writeSettings(m_globalPrefix, settings);
    }

    public void writeSettings(Setting settings) throws IOException {
        settings.acceptVisitor(new SettingsWriter(m_globalPrefix));
        if (m_error != null) {
            throw m_error;
        }
    }

    public void writeSettings(String prefix, Setting settings) throws IOException {
        settings.acceptVisitor(new SettingsWriter(prefix));
        if (m_error != null) {
            throw m_error;
        }
    }

    public void write(String key, Object value) throws IOException {
        write(m_globalPrefix, key, value);
    }

    public void writeSetting(String prefix, Setting setting) throws IOException {
        write(prefix, setting.getName(), setting.getValue());
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
                writeSetting(m_prefix, setting);
            } catch (IOException e) {
                m_error = e;
            }
        }
    }
}
