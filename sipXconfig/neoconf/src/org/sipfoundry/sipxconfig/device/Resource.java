/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 *
 */
package org.sipfoundry.sipxconfig.device;

import org.apache.commons.io.FilenameUtils;

/**
 * Represents file or an utility that should be downloadable from device related pages.
 */
public class Resource {
    private String m_name;
    private String m_mimeType;
    private String m_path;

    public Resource(String name, String path, String mimeType) {
        m_name = name;
        m_path = path;
        m_mimeType = mimeType;
    }

    public Resource(String name, String path) {
        this(name, path, "text/plain");
    }

    public String getName() {
        return m_name;
    }

    public String getMimeType() {
        return m_mimeType;

    }

    public String getPath() {
        return m_path;
    }

    public String getDirName() {
        return FilenameUtils.getFullPathNoEndSeparator(m_path);
    }

    public String getFileName() {
        return FilenameUtils.getName(m_path);
    }
}
