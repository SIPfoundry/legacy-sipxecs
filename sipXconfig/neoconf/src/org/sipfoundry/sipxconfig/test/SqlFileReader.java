/*
 * Copyright (C) 2011 eZuce Inc., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the AGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.test;

import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.io.Reader;
import java.util.ArrayList;
import java.util.List;

import org.apache.commons.lang.StringUtils;

/**
 * Read a text file that has multi-line SQL in it and return an
 * array of sql statements.  Format support is fairly limited but fairly
 * standard
 *
 * Format:
 *  o comments lines need to *start* with "--" as first character string
 *  o all SQL must end with ";"
 *
 */
public class SqlFileReader {
    private BufferedReader m_in;

    public SqlFileReader(InputStream in) {
        this(new InputStreamReader(in));
    }

    public SqlFileReader(Reader rdr) {
        m_in = new BufferedReader(rdr);
    }

    public List<String> parse() throws IOException {
        List<String> sql = new ArrayList<String>();
        StringBuilder sqlLine = new StringBuilder();
        boolean eof = false;
        do {
            boolean eol = false;
            String line = m_in.readLine();
            if (line == null) {
                eol = true;
                eof = true;
            } else {
                line = line.trim();
                if (line.startsWith("--")) {
                    continue;
                }
                if (line.endsWith(";")) {
                    eol = true;
                    line = StringUtils.chop(line);
                }
                if (sqlLine.length() > 0) {
                    sqlLine.append(' ');
                }
                sqlLine.append(line);
            }
            if (eol && StringUtils.isNotBlank(line)) {
                sql.add(sqlLine.toString());
                sqlLine.setLength(0);
            }
        } while (!eof);
        return sql;
    }
}
