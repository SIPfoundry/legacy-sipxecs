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
import java.io.FileWriter;
import java.io.IOException;
import java.io.StringWriter;
import java.io.Writer;

import org.apache.commons.io.FileUtils;
import org.apache.commons.io.IOUtils;
import org.apache.commons.logging.LogFactory;
import org.dom4j.Document;
import org.dom4j.DocumentFactory;
import org.dom4j.io.OutputFormat;
import org.dom4j.io.XMLWriter;
import org.sipfoundry.sipxconfig.admin.ConfigurationFile;

/**
 * ConfigFile
 */
public abstract class XmlFile implements ConfigurationFile {
    protected static final DocumentFactory FACTORY = DocumentFactory.getInstance();

    public abstract Document getDocument();

    public abstract ConfigFileType getType();

    /**
     * Writes document to specified writer
     * 
     * @param writer
     * @throws IOException
     */
    public void write(Writer writer) throws IOException {
        Document document = getDocument();
        OutputFormat format = new OutputFormat();
        format.setNewlines(true);
        format.setIndent(true);
        XMLWriter xmlWriter = new XMLWriter(writer, format);
        xmlWriter.write(document);
    }

    /**
     * Retrieves configuration file content as string
     * 
     * Use only for preview, use write function to dump it to the file.
     * 
     */
    public String getFileContent() {
        try {
            StringWriter writer = new StringWriter();
            write(writer);
            writer.close();
            return writer.toString();
        } catch (IOException e) {
            // ignore when writing to string
            LogFactory.getLog(XmlFile.class).error("Ignoring exception", e);
            return "";
        }
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
    public void writeToFile(File configDir, String filename) throws IOException {
        FileUtils.forceMkdir(configDir);
        // write content to temporary file
        File tmpFile = File.createTempFile(filename, "tmp", configDir);
        FileWriter writer = new FileWriter(tmpFile);
        try {
            write(writer);
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

    /**
     * Treat every file of the same type as equal
     */
    public boolean equals(Object obj) {
        if (obj instanceof XmlFile) {
            XmlFile file = (XmlFile) obj;
            return getType().equals(file.getType());
        }
        return false;
    }

    public int hashCode() {
        return getType().hashCode();
    }

    public String getFileBaseName() {
        return getType().getName();
    }
}
