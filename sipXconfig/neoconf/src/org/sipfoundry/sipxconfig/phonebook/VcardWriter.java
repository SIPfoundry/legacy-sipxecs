/*
 *
 *
 * Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 */
package org.sipfoundry.sipxconfig.phonebook;

import java.io.IOException;
import java.io.Writer;
import java.util.Formatter;

import org.apache.commons.lang.StringUtils;

public class VcardWriter {
    private String m_telType;

    public void setTelType(String telType) {
        m_telType = telType.toUpperCase();
    }

    public void write(Writer writer, PhonebookEntry entry) throws IOException {

        if (StringUtils.isEmpty(entry.getFirstName()) && StringUtils.isEmpty(entry.getLastName())) {
            return;
        }
        Formatter formatter = new Formatter(writer);
        writer.write("BEGIN:vCard\n");
        writer.write("VERSION:2.1\n");
        String firstName = StringUtils.defaultString(entry.getFirstName());
        String lastName = StringUtils.defaultString(entry.getLastName());
        formatter.format("N:%s;%s;;;\n", lastName, firstName);
        formatter.format("TEL;%s:%s\n", m_telType, entry.getNumber());
        writer.write("END:vCard\n\n");
    }
}
