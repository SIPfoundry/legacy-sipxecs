/*
 * 
 * 
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 * 
 * $
 */
package org.sipfoundry.sipxconfig.setting;

import java.io.BufferedReader;
import java.io.File;
import java.io.FileReader;
import java.io.IOException;
import java.io.PrintWriter;
import java.io.StringWriter;
import java.util.HashSet;
import java.util.Iterator;
import java.util.Properties;
import java.util.Set;

import org.apache.commons.io.FileUtils;
import org.apache.commons.io.IOUtils;
import org.apache.commons.lang.StringUtils;

/**
 * Special properties writer Preserves the order and comments in the destination file. Updates
 * properties values only and removes undefied properties.
 */
public class ConfigFileWriter {
    private static final String ENCODING = "8859_1";
    private static final Format IN_FILE_FORMAT = new Format(":", " ");
    private static final Format DEFS_FILE_FORMAT = new Format("=", StringUtils.EMPTY);

    private final File m_file;
    private final Format m_format;

    public ConfigFileWriter(File file) {
        m_file = file;
        // HACK: there should be a better way of detecting which format is used
        if (file.getName().endsWith(".in")) {
            m_format = IN_FILE_FORMAT;
        } else {
            m_format = DEFS_FILE_FORMAT;
        }
    }

    void store(Properties properties) throws IOException {
        StringWriter buffer = new StringWriter();
        PrintWriter writer = new PrintWriter(buffer);
        Set remainingProps = writeConfigured(m_file, properties, writer);
        writeRemaining(remainingProps, properties, writer);
        String generatedProps = buffer.toString();
        FileUtils.writeStringToFile(m_file, generatedProps, ENCODING);
    }

    /**
     * Writes properties and thei values to the write trying to preserve the existing properties,
     * the order and formatting of the template file
     * 
     * @param file template file
     * @param props new properties
     * @param writer
     * @return set of properties names that has not been writtent to the file yes (because they
     *         were not in template file)
     * @throws IOException
     */
    private Set writeConfigured(File file, Properties props, PrintWriter writer)
        throws IOException {
        BufferedReader reader = null;
        try {
            Set configuredProperies = new HashSet(props.keySet());
            reader = new BufferedReader(new FileReader(file));
            String line;
            while ((line = reader.readLine()) != null) {
                String propertyName = m_format.getPropertyName(line);
                if (configuredProperies.remove(propertyName)) {
                    writeProperty(writer, propertyName, props);
                    writer.println();
                } else {
                    writer.println(line);
                }
            }
            return configuredProperies;
        } finally {
            IOUtils.closeQuietly(reader);
        }
    }

    /**
     * Appends remaining properties to the configuration file
     * 
     * @param remainingProps set of the properties' names
     * @param properties
     * @param writer
     */
    private void writeRemaining(Set remainingProps, Properties properties, PrintWriter writer) {
        for (Iterator i = remainingProps.iterator(); i.hasNext();) {
            String propertyName = (String) i.next();
            writeProperty(writer, propertyName, properties);
            writer.println();
        }
    }

    private void writeProperty(PrintWriter wr, String name, Properties properties) {
        String value = properties.getProperty(name);
        if (null != value) {
            m_format.writeProperty(wr, name, value);
        }
    }

    /**
     * Details of the file formating. We have 2 Format
     */
    private static class Format {
        private static final String CHAR_COMMENT = "#";
        private final String m_separator;
        private final String m_padding;

        Format(String separator, String padding) {
            m_separator = separator;
            m_padding = padding;
        }

        void writeProperty(PrintWriter wr, String name, String value) {
            wr.print(name);
            wr.print(m_padding + m_separator + m_padding);
            wr.print(value);
        }

        private String getPropertyName(String line) {
            if (isComment(line)) {
                return StringUtils.EMPTY;
            }
            return StringUtils.substringBefore(line, m_separator).trim();
        }

        /**
         * Each property is written in the following format <code>PROP_NAME : PROP_VALUE</code>
         * or <code>PROP_NAME=PROP_VALUE</code>. Newline is not added.
         */
        private boolean isComment(String line) {
            return line.startsWith(CHAR_COMMENT);
        }
    }
}
