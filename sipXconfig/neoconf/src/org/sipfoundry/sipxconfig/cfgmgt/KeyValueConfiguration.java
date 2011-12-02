/*
 * Copyright (C) 2011 eZuce Inc., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the AGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.cfgmgt;

import java.io.IOException;
import java.io.Writer;

import org.sipfoundry.sipxconfig.setting.AbstractSettingVisitor;
import org.sipfoundry.sipxconfig.setting.Setting;

/**
 * Convert settings into key-value files that are common in configuration files and in files used
 * in cfengine scripts to feed into final configurations files.
 */
public class KeyValueConfiguration extends AbstractSettingVisitor {
    private IOException m_error;
    private Writer m_out;
    private String m_delimitor = " : ";

    public KeyValueConfiguration(Writer w) {
        this(w, null);
    }

    public KeyValueConfiguration(Writer w, String delimitor) {
        m_out = w;
        m_delimitor = delimitor;
    }

    public void write(Setting settings) throws IOException {
        settings.acceptVisitor(this);
        if (m_error != null) {
            throw m_error;
        }
    }

    public void write(String key, Object value) throws IOException {
        m_out.write(key);
        m_out.write(m_delimitor);
        m_out.append(String.valueOf(value));
        m_out.append('\n');
    }

    @Override
    public void visitSetting(Setting setting) {
        try {
            write(setting.getName(), setting.getValue());
        } catch (IOException e) {
            m_error = e;
        }
    }
}
