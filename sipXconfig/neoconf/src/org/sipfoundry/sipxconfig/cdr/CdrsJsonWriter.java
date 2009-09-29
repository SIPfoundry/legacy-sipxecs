/*
 *
 *
 * Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 *
 */
package org.sipfoundry.sipxconfig.cdr;

import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.io.Writer;
import java.sql.ResultSet;
import java.sql.SQLException;
import java.text.FieldPosition;
import java.text.Format;
import java.text.ParsePosition;
import java.util.Formatter;

import org.apache.commons.io.IOUtils;
import org.apache.commons.lang.StringEscapeUtils;
import org.apache.commons.lang.StringUtils;
import org.sipfoundry.sipxconfig.cdr.CdrManagerImpl.ColumnInfo;
import org.sipfoundry.sipxconfig.cdr.CdrManagerImpl.ColumnInfoFactory;
import org.sipfoundry.sipxconfig.common.SipUri;

class CdrsJsonWriter extends CdrsWriter {
    public static final Format AOR_FORMAT = new AorFormat();
    private boolean m_firstRecord = true;

    public CdrsJsonWriter(Writer out, ColumnInfoFactory ciFactory) {
        super(out, ciFactory);
    }

    protected void writeHeader() throws IOException {
        InputStream headerStream = getClass().getResourceAsStream("cdrs-header.json");
        InputStreamReader headerReader = new InputStreamReader(headerStream, "US-ASCII");
        IOUtils.copy(headerReader, getWriter());
    }

    protected void writeCdr(ResultSet rs, ColumnInfo[] columns) throws IOException, SQLException {
        Writer out = getWriter();
        if (!m_firstRecord) {
            out.write(",\n");
        }
        out.write("\t\t{\n");
        Formatter formatter = new Formatter(out);
        String label = "";
        for (ColumnInfo ci : columns) {
            String value = ci.formatValue(rs);
            if (StringUtils.isNotEmpty(value)) {
                String escapedValue = StringEscapeUtils.escapeJavaScript(value);
                String field = ci.getField();
                if (field.equals(CdrManagerImpl.CALL_ID)) {
                    field = "id";
                }
                formatter.format("\t\t\t\"%s\" : \"%s\",\n", field, escapedValue);
                if (field.equals(CdrManagerImpl.CALLER_AOR)) {
                    label = escapedValue;
                }
            }
        }
        formatter.format("\t\t\t\"label\" : \"%s\",\n", label);
        out.write("\t\t\t\"type\" : \"CDR\"");
        out.write("\n\t\t}");
        m_firstRecord = false;
    }

    protected void writeFooter() throws IOException {
        getWriter().write("\n\t]\n}\n");
    }

    /**
     * User readable version of SIP AORs: "display-name - username"
     */
    public static class AorFormat extends Format {

        public StringBuffer format(Object obj, StringBuffer toAppendTo, FieldPosition pos) {
            String user = SipUri.extractFullUser(obj.toString());
            return toAppendTo.append(user);
        }

        public Object parseObject(String source, ParsePosition pos) {
            throw new UnsupportedOperationException();
        }

    }
}
