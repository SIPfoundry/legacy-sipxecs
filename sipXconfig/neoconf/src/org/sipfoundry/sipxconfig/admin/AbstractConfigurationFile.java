/*
 *
 *
 * Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 */
package org.sipfoundry.sipxconfig.admin;

import java.io.IOException;
import java.io.StringWriter;

import org.apache.commons.lang.StringUtils;
import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import org.sipfoundry.sipxconfig.admin.commserver.Location;
import org.springframework.beans.factory.annotation.Required;

public abstract class AbstractConfigurationFile implements ConfigurationFile {
    private static final Log LOG = LogFactory.getLog(AbstractConfigurationFile.class);

    private String m_name;

    private String m_directory;

    private boolean m_restartRequired = true;

    public final String getName() {
        return m_name;
    }

    public final String getPath() {
        // forward slash on purpose - this is *not* a local path
        if (m_name == null) {
            throw new IllegalStateException("Name has to be configured for a file.");
        }
        StringBuilder path = new StringBuilder();
        if (StringUtils.isNotBlank(m_directory)) {
            path.append(m_directory);
            path.append('/');
        }
        path.append(m_name);
        return path.toString();
    }

    @Required
    public void setName(String name) {
        m_name = name;
    }

    @Required
    public void setDirectory(String directory) {
        m_directory = directory;
    }

    /**
     * Treat every file with the same path
     */
    @Override
    public boolean equals(Object obj) {
        if (obj instanceof ConfigurationFile) {
            ConfigurationFile file = (ConfigurationFile) obj;
            return getPath().equals(file.getPath());
        }
        return false;
    }

    @Override
    public int hashCode() {
        return getPath().hashCode();
    }

    public boolean isReplicable(Location location) {
        return true;
    }

    public void setRestartRequired(boolean restartRequired) {
        m_restartRequired = restartRequired;
    }

    public boolean isRestartRequired() {
        return m_restartRequired;
    }

    /**
     * Retrieves configuration file content as string
     *
     * Use only for preview, use write function to dump it to the file.
     */
    public static String getFileContent(ConfigurationFile cf, Location location) {
        try {
            StringWriter out = new StringWriter();
            cf.write(out, location);
            return out.toString();
        } catch (IOException e) {
            LOG.error("Rethrowing unexpected: " + e.getMessage());
            throw new RuntimeException(e);
        }
    }
}
