/*
 * 
 * 
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 * 
 * $
 */
package org.sipfoundry.sipxconfig.admin;

import java.io.IOException;
import java.io.Writer;

import org.sipfoundry.sipxconfig.admin.dialplan.config.ConfigFileType;

public interface ConfigurationFile {

    public abstract ConfigFileType getType();

    /**
     * Writes document to specified writer
     * 
     * @param writer
     * @throws IOException
     */
    public void write(Writer writer) throws IOException;

    /**
     * Retrieves configuration file content as string
     * 
     * Use only for preview, use write function to dump it to the file.
     * 
     */
    public String getFileContent();
}
