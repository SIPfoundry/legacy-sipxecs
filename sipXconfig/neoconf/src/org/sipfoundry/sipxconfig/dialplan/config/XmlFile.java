/**
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.dialplan.config;

import java.io.IOException;
import java.io.Writer;

import org.dom4j.Document;
import org.dom4j.DocumentFactory;
import org.dom4j.io.OutputFormat;
import org.dom4j.io.XMLWriter;

public class XmlFile {
    public static final DocumentFactory FACTORY = DocumentFactory.getInstance();
    private OutputFormat m_format;
    private Writer m_writer;

    public XmlFile(Writer writer) {
        m_writer = writer;
    }

    public void write(Document doc) throws IOException {
        XMLWriter xmlWriter = new XMLWriter(m_writer, getFormat());
        xmlWriter.write(doc);
    }

    /**
     * Overwrite to prepare XML format for this file.
     */
    public OutputFormat getFormat() {
        if (m_format != null) {
            return m_format;
        }
        OutputFormat format = new OutputFormat();
        format.setNewlines(true);
        format.setIndent(true);
        return format;
    }

    public void setFormat(OutputFormat format) {
        m_format = format;
    }
}
