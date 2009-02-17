/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig;

import java.sql.Connection;
import java.sql.SQLException;

import org.dbunit.database.DatabaseConfig;
import org.dbunit.database.DatabaseConnection;
import org.dbunit.database.IDatabaseConnection;
import org.dbunit.dataset.IDataSet;
import org.dbunit.dataset.ReplacementDataSet;
import org.dbunit.operation.DatabaseOperation;
import org.hibernate.SessionFactory;
import org.springframework.dao.DataIntegrityViolationException;
import org.springframework.orm.hibernate3.HibernateTemplate;
import org.springframework.test.annotation.AbstractAnnotationAwareTransactionalTests;

public abstract class IntegrationTestCase extends AbstractAnnotationAwareTransactionalTests {
    private SessionFactory m_sessionFactory;

    private HibernateTemplate m_hibernateTemplate;

    static {
        // triggers creating sipxconfig.properties in classpath so spring app context
        // is properly configured with test settings
        TestHelper.initSysDirProperties(TestHelper.getClasspathDirectory());
    }

    public IntegrationTestCase() {
        setAutowireMode(AUTOWIRE_BY_NAME);
    }

    @Override
    protected String[] getConfigLocations() {
        // There are many interdependencies between spring files so in general you need
        // to load them all. However, if you do have isolated spring file, this is definitely
        // overrideable
        return new String[] {
            "classpath*:**/*.beans.xml"
        };
    }

    @Override
    public void runBare() throws Throwable {
        try {
            super.runBare();
        } catch (SQLException e) {
            SipxDatabaseTestCase.dumpSqlExceptionMessages(e);
            throw e;
        } catch (DataIntegrityViolationException e) {
            SipxDatabaseTestCase.dumpSqlExceptionMessages(e);
            throw e;
        }
    }

    protected void loadDataSet(String resource) {
        IDatabaseConnection connection = getConnection();
        try {
            IDataSet dataSet = TestHelper.loadDataSetFlat(resource);
            DatabaseOperation.CLEAN_INSERT.execute(connection, dataSet);
        } catch (RuntimeException e) {
            throw e;
        } catch (Exception e) {
            throw new RuntimeException(e);
        }
    }

    protected void loadDataSetXml(String resource) throws Exception {
        IDatabaseConnection connection = getConnection();
        IDataSet dataSet = TestHelper.loadDataSet(resource);
        DatabaseOperation.CLEAN_INSERT.execute(connection, dataSet);
    }

    protected ReplacementDataSet loadReplaceableDataSetFlat(String fileResource) throws Exception {
        IDataSet ds = TestHelper.loadDataSetFlat(fileResource);
        ReplacementDataSet relaceable = new ReplacementDataSet(ds);
        relaceable.addReplacementObject("[null]", null);
        return relaceable;
    }

    protected IDatabaseConnection getConnection() {
        // by getting the same connection, out dbunit operations happen in same transation
        // subsequently get rolled back automaticaly
        Connection jdbcConnection = m_sessionFactory.getCurrentSession().connection();
        IDatabaseConnection dbunitConnection = new DatabaseConnection(jdbcConnection);
        DatabaseConfig config = dbunitConnection.getConfig();
        config.setFeature("http://www.dbunit.org/features/batchedStatements", true);

        return dbunitConnection;
    }

    /**
     * Flush hibernate session.
     *
     * Flush need to be called after hibernate/spring operations, before testing the content of
     * the database with jdbcTemplate or DBUnit assertions.
     */
    protected void flush() {
        m_hibernateTemplate.flush();
    }

    public void setSessionFactory(SessionFactory sessionFactory) {
        m_sessionFactory = sessionFactory;
        m_hibernateTemplate = new HibernateTemplate();
        m_hibernateTemplate.setSessionFactory(m_sessionFactory);
    }
}
