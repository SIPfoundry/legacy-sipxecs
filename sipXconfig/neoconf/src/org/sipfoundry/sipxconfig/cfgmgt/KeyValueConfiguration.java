/*
 * Copyright (C) 2011 eZuce Inc., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the AGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.cfgmgt;

import static java.lang.String.format;

import java.io.IOException;
import java.io.Writer;
import java.util.Collection;
import java.util.Iterator;

import org.apache.commons.lang.StringUtils;
import org.sipfoundry.sipxconfig.setting.AbstractSettingVisitor;
import org.sipfoundry.sipxconfig.setting.Setting;

/**
 * Convert settings into key-value files that are common in configuration files and in files used
 * in cfengine scripts to feed into final configurations files.
 */
public class KeyValueConfiguration {
    private IOException m_error;
    private Writer m_out;
    private String m_delimitor;
    private String m_valueFormat;

    public KeyValueConfiguration(Writer w) {
        this(w, " : ");
    }

    public KeyValueConfiguration(Writer w, String delimitor) {
        m_out = w;
        m_delimitor = delimitor;
    }

    public void write(Collection<Setting> settings) throws IOException {
        Iterator<Setting> i = settings.iterator();
        while (i.hasNext()) {
            write(i.next());
        }
    }

    public void write(Setting settings) throws IOException {
        settings.acceptVisitor(new SettingsWriter());
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
        m_out.write(key);
        m_out.write(m_delimitor);
        writeValue(value);
        m_out.write('\n');
    }

    protected void writeValue(Object value) throws IOException {
        if (m_valueFormat != null) {
            m_out.write(format(m_valueFormat, value));
        } else {
            m_out.write(value == null ? "" :  String.valueOf(value));
        }
    }

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
                String key = setting.getName();
                if (StringUtils.isNotBlank(m_prefix)) {
                    key = m_prefix + key;
                }
                write(key, setting.getValue());
            } catch (IOException e) {
                m_error = e;
            }
        }
    }

    public String getValueFormat() {
        return m_valueFormat;
    }

    /**
     * Uses String.format(valueFormat, value) to write value to file
     *
     * Example:
     *  Calling
     *    setValueFormat("AAA %s BBB");
     *    write("KEY", "VALUE");
     *  Result:
     *    KEY : AAA VALUE BBB
     *
     * @param valueFormat
     */
    public void setValueFormat(String valueFormat) {
        m_valueFormat = valueFormat;
    }
}
