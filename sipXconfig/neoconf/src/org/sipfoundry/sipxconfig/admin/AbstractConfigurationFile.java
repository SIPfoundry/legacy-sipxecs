/*
 *
 *
 * Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 */
package org.sipfoundry.sipxconfig.admin;

import java.io.File;
import java.io.FileWriter;
import java.io.IOException;

import org.apache.commons.io.FileUtils;
import org.apache.commons.io.IOUtils;
import org.apache.commons.lang.StringUtils;
import org.sipfoundry.sipxconfig.admin.dialplan.config.XmlFile;
import org.springframework.beans.factory.annotation.Required;

public abstract class AbstractConfigurationFile implements ConfigurationFile {
    private String m_name;

    private String m_directory;

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
    public boolean equals(Object obj) {
        if (obj instanceof XmlFile) {
            XmlFile file = (XmlFile) obj;
            return getPath().equals(file.getPath());
        }
        return false;
    }

    public int hashCode() {
        return getPath().hashCode();
    }

    /**
     * Creates a bakup copy of a generated file, and writes a new file. The implementation
     * actually writes to a temporary file first and only if this is successfull it will rename
     * the file.
     * 
     * @param configDir File object representing a directory in which files are created
     * @param filename xml file name
     * @throws IOException
     */
    @Deprecated
    public void writeToFile(File configDir, String filename) throws IOException {
        FileUtils.forceMkdir(configDir);
        // write content to temporary file
        File tmpFile = File.createTempFile(filename, "tmp", configDir);
        FileWriter writer = new FileWriter(tmpFile);

        try {
            write(writer, null);
        } finally {
            IOUtils.closeQuietly(writer);
        }

        File configFile = new File(configDir, filename);

        // make a backup copy of the file if it exist
        if (configFile.exists()) {
            // FIXME: this is a naive generation of backup files - we should not have more than n
            // backups
            File backup = new File(configDir, filename + ".~");
            backup.delete();
            configFile.renameTo(backup);
        }

        // rename tmpFile to configFile
        tmpFile.renameTo(configFile);
    }
}
