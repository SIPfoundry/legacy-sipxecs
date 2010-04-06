/*
 * Copyright (C) 2010 Avaya, certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 */
package org.sipfoundry.sipxconfig;

import java.io.Writer;
import java.sql.SQLException;

import junit.framework.TestCase;

import org.dbunit.database.IDatabaseConnection;
import org.dbunit.dataset.IDataSet;
import org.dbunit.dataset.xml.FlatXmlWriter;
import org.springframework.dao.DataIntegrityViolationException;

/**
 * Special TestCase class that catches and prints additional info for SQL exceptions that may
 * happen during setUp, testXXX and tearDown.
 *
 * Alternatively we could just throw e.getNextException, but we may want to preserve the original
 * exception.
 */
public abstract class SipxDatabaseTestCase extends TestCase {
    public void runBare() throws Throwable {
        try {
            super.runBare();
        } catch (SQLException e) {
            dumpSqlExceptionMessages(e.getNextException());
            throw e.getNextException();
        } catch (DataIntegrityViolationException e) {
            dumpSqlExceptionMessages(e);
            throw e;
        }
    }

    static void dumpSqlExceptionMessages(SQLException e) {
        for (SQLException next = e; next != null; next = next.getNextException()) {
            System.err.println(next.getMessage());
        }
    }

    static void dumpSqlExceptionMessages(DataIntegrityViolationException e) {
        if (e.getCause() instanceof SQLException) {
            dumpSqlExceptionMessages((SQLException) e.getCause());
        }
    }

    protected IDatabaseConnection getConnection() throws Exception {
        return TestHelper.getConnection();
    }

    protected void tearDown() throws Exception {
        super.tearDown();

        TestHelper.closeConnection();
    }

    protected void writeFlatXmlDataSet(Writer out) throws Exception {
        IDataSet set = getConnection().createDataSet();
        FlatXmlWriter writer = new FlatXmlWriter(out);
        writer.write(set);
    }
}
