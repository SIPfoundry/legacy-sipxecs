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
import org.sipfoundry.sipxconfig.admin.ConfigurationFile;
import org.sipfoundry.sipxconfig.admin.commserver.Location;

public abstract class XmlFile extends AbstractConfigurationFile implements ConfigurationFile {
    protected static final DocumentFactory FACTORY = DocumentFactory.getInstance();

    public abstract Document getDocument();

    public void write(Writer writer, Location location) throws IOException {
        localizeDocument(location);
        Document document = getDocument();
        OutputFormat format = createFormat();
        XMLWriter xmlWriter = new XMLWriter(writer, format);
        xmlWriter.write(document);
    }

    /**
     * Override this method to use location-specific data as last step to generate the Document
     * object that will be written by the write method
     *
     * @param location server for which we generate this document
     */
    protected void localizeDocument(Location location) {
        // default implementation does nothing
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
        return getFileContent(null);
    }

    /**
     * Retrieves configuration file content as string depending on the location
     *
     * Use only for preview, use write function to dump it to the file.
     *
     */
    public String getFileContent(Location location) {
        try {
            StringWriter writer = new StringWriter();
            write(writer, location);
            writer.close();
            return writer.toString();
        } catch (IOException e) {
            // ignore when writing to string
            LogFactory.getLog(XmlFile.class).error("Ignoring exception", e);
            return "";
        }
    }
}
