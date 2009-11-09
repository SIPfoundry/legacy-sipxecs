/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 *
 */
package org.sipfoundry.sipxconfig.admin.dialplan.config;

import java.io.IOException;
import java.io.Writer;

import org.dom4j.Document;
import org.dom4j.DocumentFactory;
import org.dom4j.io.OutputFormat;
import org.dom4j.io.XMLWriter;
import org.sipfoundry.sipxconfig.admin.AbstractConfigurationFile;
import org.sipfoundry.sipxconfig.admin.commserver.Location;

public abstract class XmlFile extends AbstractConfigurationFile {
    protected static final DocumentFactory FACTORY = DocumentFactory.getInstance();

    @Deprecated
    public Document getDocument() {
        // getDocument(Location location) should be used instead
        return null;
    }

    public Document getDocument(Location location) {
        localizeDocument(location);
        return getDocument();
    }

    public void write(Writer writer, Location location) throws IOException {
        Document document = getDocument(location);
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
}
