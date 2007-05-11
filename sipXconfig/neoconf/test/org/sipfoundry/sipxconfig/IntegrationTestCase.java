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
import org.dbunit.operation.DatabaseOperation;
import org.hibernate.SessionFactory;
import org.springframework.dao.DataIntegrityViolationException;
import org.springframework.test.AbstractTransactionalDataSourceSpringContextTests;

public abstract class IntegrationTestCase extends AbstractTransactionalDataSourceSpringContextTests {    
    private SessionFactory m_sessionFactory;
    
    static {
        // triggers creating sipxconfig.properties in classpath so spring app context
        // is properly configured with test settings
        TestHelper.initSysDirProperties(TestHelper.getClasspathDirectory());
    }

    public IntegrationTestCase() {
        setAutowireMode(AUTOWIRE_BY_NAME);        
    }

    protected String[] getConfigLocations() {
        // There are many interdependencies between spring files so in general you need
        // to load them all. However, if you do have isolated spring file, this is definitely
        // overrideable
        return new String[] { 
          "classpath:**/*.beans.xml", 
        };
    }

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
            DatabaseOperation.CLEAN_INSERT.execute(connection, TestHelper.loadDataSetFlat(resource));
        } catch (RuntimeException e) {
            throw e;
        } catch (Exception e) {
            throw new RuntimeException(e);
        }
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
        
    public void setSessionFactory(SessionFactory sessionFactory) {
        m_sessionFactory = sessionFactory;
    }
}
