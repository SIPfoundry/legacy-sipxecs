/*
 *
 *
 * Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 */
package org.sipfoundry.sipxconfig.device;

import java.io.IOException;
import java.io.Writer;

import org.sipfoundry.sipxconfig.admin.ConfigurationFile;
import org.sipfoundry.sipxconfig.admin.dialplan.config.ConfigFileType;

public class InMemoryConfiguration implements ConfigurationFile {
    private final String m_content;
    private final ConfigFileType m_type;

    public InMemoryConfiguration(ConfigFileType type, String content) {
        m_type = type;
        m_content = content;
    }

    public String getFileContent() {
        return new String(m_content);
    }

    public ConfigFileType getType() {
        return m_type;
    }

    public void write(Writer writer) throws IOException {
        writer.write(m_content);
    }

    public boolean equals(Object obj) {
        InMemoryConfiguration o = (InMemoryConfiguration) obj;
        return m_content.equals(o.m_content) && m_type.equals(o.m_type);
    }

    public int hashCode() {
        return m_content.hashCode();
    }
}
