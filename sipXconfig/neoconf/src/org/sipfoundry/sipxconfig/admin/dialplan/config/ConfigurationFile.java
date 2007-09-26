/*
 * 
 * 
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 * 
 * $
 */
package org.sipfoundry.sipxconfig.admin.dialplan.config;

import java.io.File;
import java.io.IOException;
import java.io.Writer;

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
     * Creates a backup copy of a generated file, and writes a new file. The implementation
     * actually writes to a temporary file first and only if this is successful it will rename
     * the file.
     * 
     * @param configDir File object representing a directory in which files are created
     * @param filename xml file name
     * @throws IOException
     */
    public void writeToFile(File configDir, String filename) throws IOException;

    /**
     * Retrieves configuration file content as string
     * 
     * Use only for preview, use write function to dump it to the file.
     * 
     */
    public String getFileContent();

}
