/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.bulk.csv;

import java.io.File;
import java.io.FileReader;
import java.io.IOException;
import java.io.Reader;

import org.apache.commons.io.IOUtils;
import org.sipfoundry.sipxconfig.bulk.BulkParser;
import org.springframework.orm.hibernate3.support.HibernateDaoSupport;

public class BulkManagerImpl extends HibernateDaoSupport implements BulkManager {
    private BulkParser m_csvParser;

    private CsvRowInserter m_rowInserter;

    public void setCsvParser(BulkParser csvParser) {
        m_csvParser = csvParser;
    }

    public void setRowInserter(CsvRowInserter rowInserter) {
        m_rowInserter = rowInserter;
    }

    public void insertFromCsv(Reader reader) {
        m_rowInserter.beforeInserting();
        m_csvParser.parse(reader, m_rowInserter);
        m_rowInserter.afterInserting();
    }

    public void insertFromCsv(File file, boolean deleteOnImport) {
        Reader reader = null;
        try {
            reader = new FileReader(file);
            insertFromCsv(reader);
            reader.close();
            if (deleteOnImport) {
                file.delete();
            }
        } catch (IOException e) {
            throw new RuntimeException(e);
        } finally {
            IOUtils.closeQuietly(reader);
        }
    }
}
