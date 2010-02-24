/*
 *
 *
 * Copyright (C) 2010 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.admin;

import java.io.File;
import java.io.FileInputStream;
import java.io.IOException;
import java.io.Writer;

import org.sipfoundry.sipxconfig.admin.commserver.Location;

import static org.apache.commons.io.IOUtils.copy;

public class AnyFile extends AbstractConfigurationFile {
    private String m_sourceFilePath;

    public final void write(Writer output, Location location) throws IOException {
        File sourceFile = new File(m_sourceFilePath);
        copy(new FileInputStream(sourceFile), output);
    }

    public void setSourceFilePath(String sourceFilePath) {
        m_sourceFilePath = sourceFilePath;
    }
}
