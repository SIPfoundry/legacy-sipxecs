/*
 *
 *
 * Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 */
package org.sipfoundry.sipxconfig.phonebook;

import java.io.IOException;
import java.io.Writer;
import java.util.Formatter;

public class VcardWriter implements PhonebookWriter {
    private Writer m_writer;

    public VcardWriter() {
    }

    public VcardWriter(Writer writer) {
        m_writer = writer;
    }

    public void write(PhonebookEntry entry) throws IOException {
        if (entry == null) {
            return;
        }
        if (!entry.isWritable()) {
            return;
        }
        String[] fields = entry.getFields();

        Formatter formatter = new Formatter(m_writer);
        m_writer.write("BEGIN:vCard\n");
        m_writer.write("VERSION:3.0\n");
        formatter.format("FN:%s %s\n", fields[0], fields[1]);
        formatter.format("N:%s;%s;;;\n", fields[1], fields[0]);
        formatter.format("TEL;TYPE=WORK:%s\n", fields[2]);
        if (entry.getAddressBookEntry() != null) {
            formatter.format("TEL;TYPE=HOME:%s\n", fields[8]);
            formatter.format("TEL;TYPE=CELL:%s\n", fields[7]);
            formatter.format("TEL;TYPE=FAX:%s\n", fields[10]);
            formatter.format("ADR;TYPE=WORK:%s;;%s;%s;%s;%s;%s\n", fields[24], fields[22], fields[19], fields[21],
                    fields[23], fields[20]);
            formatter.format("ADR;TYPE=HOME:;;%s;%s;%s;%s;%s\n", fields[17], fields[14], fields[16], fields[18],
                    fields[15]);
            formatter.format("EMAIL;TYPE=PREF:%s\n", fields[25]);
            formatter.format("EMAIL:%s\n", fields[26]);
            formatter.format("ORG:%s;%s\n", fields[5], fields[4]);
            formatter.format("TITLE:%s\n", fields[3]);
        }
        m_writer.write("END:vCard\n\n");
    }

}
