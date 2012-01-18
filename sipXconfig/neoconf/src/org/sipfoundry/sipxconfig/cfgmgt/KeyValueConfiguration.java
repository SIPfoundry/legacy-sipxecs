/*
 * Copyright (C) 2011 eZuce Inc., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the AGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.cfgmgt;

import static java.lang.String.format;

import java.io.IOException;
import java.io.Writer;

import org.apache.commons.lang.StringUtils;

/**
 * Convert settings into key-value files that are common in configuration files and in files used
 * in cfengine scripts to feed into final configurations files.
 */
public class KeyValueConfiguration extends AbstractConfigurationFile {
    private static final String COLON = " : ";
    private static final String EQUALS = "=";
    private String m_valueFormat;
    private String m_delimator;
    private Writer m_out;

    public KeyValueConfiguration(Writer w, String delimitor, String prefix) {
        super(prefix);
        m_out = w;
        m_delimator = delimitor;
    }

    public static KeyValueConfiguration colonSeparated(Writer w) {
        return new KeyValueConfiguration(w, COLON, null);
    }

    public static KeyValueConfiguration colonSeparated(Writer w, String globalPrefix) {
        return new KeyValueConfiguration(w, COLON, globalPrefix);
    }

    public static KeyValueConfiguration equalsSeparated(Writer w) {
        return new KeyValueConfiguration(w, EQUALS, null);
    }

    public static KeyValueConfiguration equalsSeparated(Writer w, String globalPrefix) {
        return new KeyValueConfiguration(w, EQUALS, globalPrefix);
    }

    protected void writeValue(Object value) throws IOException {
        if (m_valueFormat != null) {
            m_out.write(format(m_valueFormat, value));
        } else {
            m_out.write(value == null ? "" :  String.valueOf(value));
        }
    }

    public void write(String prefix, String key, Object value) throws IOException {
        String fullKey;
        if (StringUtils.isNotBlank(prefix)) {
            fullKey = prefix + key;
        } else {
            fullKey = key;
        }
        m_out.write(fullKey);
        m_out.write(m_delimator);
        writeValue(value);
        m_out.write('\n');
    }

    public String getValueFormat() {
        return m_valueFormat;
    }

    /**
     * Uses String.format(valueFormat, value) to write value to file
     *
     * Example:
     *  Calling
     *    setValueFormat("AAA %s BBB");
     *    write("KEY", "VALUE");
     *  Result:
     *    KEY : AAA VALUE BBB
     *
     * @param valueFormat
     */
    public void setValueFormat(String valueFormat) {
        m_valueFormat = valueFormat;
    }
}
