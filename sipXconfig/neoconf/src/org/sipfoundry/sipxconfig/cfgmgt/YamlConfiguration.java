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

/**
 * Primitive YAML writer.
 *
 * Limitations/Notes:
 *  Does not support associative lists, but you can use write(key, "[ a, b ]")
 *  Does not escape text
 *  Does not support multi-line
 *  Consider introducing a library before expanding on this too, too much.
 */
public class YamlConfiguration extends AbstractConfigurationFile {
    private static final String INDEX = "                                                      ";
    private Writer m_out;
    private int m_nestLevel;
    private boolean m_newStruct;

    public YamlConfiguration(Writer w) {
        m_out = w;
    }

    @Override
    public void write(String prefix, String key, Object value) throws IOException {
        m_out.write(indent());
        m_out.write(key);
        m_out.write(": ");
        m_out.write(value.toString());
        m_out.write("\n");
    }

    private String indent() {
        String indent;
        if (m_newStruct) {
            // if m_nestLevel <= 0 something else is wrong
            indent = INDEX.substring(0, 3 * (m_nestLevel - 1)) + " - ";
        } else {
            indent = INDEX.substring(0, 3 * m_nestLevel);
        }
        m_newStruct = false;
        return indent;
    }

    private String up() {
        String s = indent();
        m_nestLevel++;
        return s;
    }

    private void down() {
        m_nestLevel--;
    }

    public void startStruct(String key) throws IOException {
        m_out.write(up());
        m_out.write(format("%s:\n", key));
    }

    public void startArray(String key) throws IOException {
        startStruct(key);
        m_newStruct = true;
    }

    public void nextElement() throws IOException {
        m_newStruct = true;
    }

    public void endStruct() throws IOException {
        endArray();
    }

    public void endArray() throws IOException {
        down();
        m_newStruct = false;
    }
}
