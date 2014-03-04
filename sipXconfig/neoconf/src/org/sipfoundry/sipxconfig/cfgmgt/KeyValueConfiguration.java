/**
 *
 *
 * Copyright (c) 2012 eZuce, Inc. All rights reserved.
 * Contributed to SIPfoundry under a Contributor Agreement
 *
 * This software is free software; you can redistribute it and/or modify it under
 * the terms of the Affero General Public License (AGPL) as published by the
 * Free Software Foundation; either version 3 of the License, or (at your option)
 * any later version.
 *
 * This software is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more
 * details.
 */
package org.sipfoundry.sipxconfig.cfgmgt;

import static java.lang.String.format;

import java.io.IOException;
import java.io.Writer;

import org.apache.commons.lang.StringUtils;
import org.sipfoundry.sipxconfig.setting.Setting;

/**
 * Convert settings into key-value files that are common in configuration files and in files used
 * in cfengine scripts to feed into final configurations files.
 */
public class KeyValueConfiguration extends AbstractConfigurationFile {
    protected static final String COLON = " : ";
    protected static final String EQUALS = "=";
    protected static final String COMMA = ",";
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

    /**
     * This methods uses as key not the settings key, but the one provided as parameter.
     * Will also add the provided suffix to the end of each written line.
     * @param key
     * @param value
     * @param suffix
     * @throws IOException
     */
    public void writeWithKey(String key, Setting setting) throws IOException {
        m_out.write(key);
        m_out.write(m_delimator);
        writeValue(setting.getValue());
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
