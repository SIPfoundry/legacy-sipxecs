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

import java.io.IOException;
import java.io.StringWriter;
import java.io.Writer;

import org.apache.commons.logging.LogFactory;
import org.dom4j.Document;
import org.dom4j.DocumentFactory;
import org.dom4j.io.OutputFormat;
import org.dom4j.io.XMLWriter;
import org.sipfoundry.sipxconfig.admin.AbstractConfigurationFile;

/**
 * ConfigFile
 */
public abstract class XmlFile extends AbstractConfigurationFile {
    protected static final DocumentFactory FACTORY = DocumentFactory.getInstance();

    public abstract Document getDocument();

    /**
     * Writes document to specified writer
     * 
     * @param writer
     * @throws IOException
     */
    public void write(Writer writer) throws IOException {
        Document document = getDocument();
        OutputFormat format = createFormat();
        XMLWriter xmlWriter = new XMLWriter(writer, format);
        xmlWriter.write(document);
    }

    /**
     * Overwrite to prepare XML format for this file.
     */
    public OutputFormat createFormat() {
        OutputFormat format = new OutputFormat();
        format.setNewlines(true);
        format.setIndent(true);
        return format;
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

}
