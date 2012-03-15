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
