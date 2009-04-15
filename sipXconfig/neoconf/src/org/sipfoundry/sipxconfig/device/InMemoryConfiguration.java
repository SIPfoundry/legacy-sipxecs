/*
 *
 *
 * Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 */
package org.sipfoundry.sipxconfig.device;

import java.io.IOException;
import java.io.Writer;

import org.sipfoundry.sipxconfig.admin.AbstractConfigurationFile;
import org.sipfoundry.sipxconfig.admin.commserver.Location;

public class InMemoryConfiguration extends AbstractConfigurationFile {
    private final String m_content;

    private Location m_location;

    public InMemoryConfiguration(String directory, String name, String content) {
        setDirectory(directory);
        setName(name);
        m_content = content;
    }

    public void setLocation(Location location) {
        m_location = location;
    }

    public void write(Writer writer, Location location) throws IOException {
        writer.write(m_content);
    }

    @Override
    public boolean isReplicable(Location location) {
        if (m_location == null) {
            return true;
        }
        return m_location.equals(location);
    }

    @Override
    public boolean equals(Object obj) {
        InMemoryConfiguration o = (InMemoryConfiguration) obj;
        return m_content.equals(o.m_content);
    }

    @Override
    public int hashCode() {
        return m_content.hashCode();
    }
}
