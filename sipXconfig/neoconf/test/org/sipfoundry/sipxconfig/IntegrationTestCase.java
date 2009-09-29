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

import java.lang.reflect.InvocationTargetException;
import java.sql.Connection;
import java.sql.SQLException;
import java.util.HashMap;
import java.util.Map;

import org.apache.commons.beanutils.BeanUtils;
import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import org.dbunit.database.DatabaseConfig;
import org.dbunit.database.DatabaseConnection;
import org.dbunit.database.IDatabaseConnection;
import org.dbunit.dataset.IDataSet;
import org.dbunit.dataset.ReplacementDataSet;
import org.dbunit.operation.DatabaseOperation;
import org.hibernate.SessionFactory;
import org.sipfoundry.sipxconfig.admin.commserver.LocationsManagerImplTestIntegration;
import org.springframework.dao.DataIntegrityViolationException;
import org.springframework.orm.hibernate3.HibernateTemplate;
import org.springframework.test.annotation.AbstractAnnotationAwareTransactionalTests;

public abstract class IntegrationTestCase extends AbstractAnnotationAwareTransactionalTests {
    private static final Log LOG = LogFactory.getLog(LocationsManagerImplTestIntegration.class);

    private SessionFactory m_sessionFactory;

    private HibernateTemplate m_hibernateTemplate;

    protected Map<Object, Map<String, Object>> m_modifiedContextObjectMap;

    static {
        // triggers creating sipxconfig.properties in classpath so spring app context
        // is properly configured with test settings
        TestHelper.initSysDirProperties(TestHelper.getClasspathDirectory());
    }

    public IntegrationTestCase() {
        setAutowireMode(AUTOWIRE_BY_NAME);
    }

    @Override
    protected void onSetUpInTransaction() throws Exception {
        super.onSetUpInTransaction();
        m_modifiedContextObjectMap = new HashMap<Object, Map<String,Object>>();
    }
    @Override
    protected void onTearDownInTransaction() throws Exception {
        super.onTearDownInTransaction();
        if (m_modifiedContextObjectMap != null) {
            resetContext();
        }
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

    /**
     * Modifies a concrete object from the spring context.  Any modifications will be be
     * rolled back in the tearDown
     * @param target The context object to modify
     * @param propertyName The property of the the target to modify
     * @param originalValue The original value, used to roll back in the tearDown
     * @param valueForTest The value to be set in the target for this test
     */
    protected void modifyContext(Object target, String propertyName, Object originalValue, Object valueForTest) {
        if (! m_modifiedContextObjectMap.containsKey(target)) {
            m_modifiedContextObjectMap.put(target, new HashMap<String, Object>());
        }

        Map<String, Object> originalContextObjectValueMap = m_modifiedContextObjectMap.get(target);
        originalContextObjectValueMap.put(propertyName, originalValue);

        try {
            BeanUtils.setProperty(target, propertyName, valueForTest);
        } catch (IllegalAccessException e) {
            LOG.error("Unable to set property " + propertyName + " on target " + target, e);
        } catch (InvocationTargetException e) {
            LOG.error("Unable to set property " + propertyName + " on target " + target, e);
        }
    }

    /**
     * Roll back any changes made to context objects via the modifyContext method
     */
    private void resetContext() {
        for (Object target : m_modifiedContextObjectMap.keySet()) {
            Map<String, Object> originalValueMap = m_modifiedContextObjectMap.get(target);
            for (String propertyName : originalValueMap.keySet()) {
                Object originalValue = originalValueMap.get(propertyName);
                try {
                    BeanUtils.setProperty(target, propertyName, originalValue);
                    originalValueMap.remove(propertyName);
                } catch (IllegalAccessException e) {
                    LOG.error("Unable to set property " + propertyName + " on target " + target, e);
                } catch (InvocationTargetException e) {
                    LOG.error("Unable to set property " + propertyName + " on target " + target, e);
                }
            }

            m_modifiedContextObjectMap.remove(target);
        }

        assertTrue(m_modifiedContextObjectMap.isEmpty());
    }
}
