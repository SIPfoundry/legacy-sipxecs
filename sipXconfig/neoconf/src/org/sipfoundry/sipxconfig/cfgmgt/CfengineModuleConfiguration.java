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

public class CfengineModuleConfiguration extends AbstractConfigurationFile {
    private static final String EOL = System.getProperty("line.separator");
    private Writer m_out;

    public CfengineModuleConfiguration(Writer w) {
        m_out = w;
    }

    public void writeClass(String property, boolean enabled) throws IOException {
        m_out.write(enabled ? '+' : '-');
        m_out.write(property);
        m_out.write(EOL);
    }

    @Override
    public void write(String prefix, String key, Object value) throws IOException {
        m_out.write('=');
        if (prefix != null) {
            m_out.write(prefix);
        }
        m_out.write(key);
        m_out.write('=');
        m_out.write(value == null ? "" : value.toString());
        m_out.write(EOL);
    }
}
