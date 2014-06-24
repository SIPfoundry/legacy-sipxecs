/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.test;

import static org.easymock.EasyMock.createNiceMock;

import java.io.IOException;
import java.io.InputStream;
import java.lang.reflect.InvocationTargetException;
import java.sql.Connection;
import java.sql.SQLException;
import java.util.HashMap;
import java.util.Map;

import javax.sql.DataSource;

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
import org.sipfoundry.commons.userdb.profile.UserProfileService;
import org.sipfoundry.sipxconfig.common.SpringHibernateInstantiator;
import org.sipfoundry.sipxconfig.common.event.DaoEventListener;
import org.sipfoundry.sipxconfig.common.event.DaoEventPublisherImpl;
import org.springframework.dao.DataIntegrityViolationException;
import org.springframework.data.mongodb.core.MongoTemplate;
import org.springframework.jdbc.core.JdbcTemplate;
import org.springframework.orm.hibernate3.HibernateTemplate;
import org.springframework.orm.hibernate3.HibernateTransactionManager;
import org.springframework.test.annotation.AbstractAnnotationAwareTransactionalTests;

public abstract class IntegrationTestCase extends AbstractAnnotationAwareTransactionalTests {
    private static final String ROOT_RES_PATH = "/org/sipfoundry/sipxconfig/";
    private static final Log LOG = LogFactory.getLog(IntegrationTestCase.class);
    private static final String CANNOT_SET_PROP_MSG = "Unable to set property %s on target %s";

    private SessionFactory m_sessionFactory;
    private HibernateTemplate m_hibernateTemplate;
    private JdbcTemplate m_db;
    private Map<Object, Map<String, Object>> m_modifiedContextObjectMap;
    private DaoEventPublisherImpl m_daoEventPublisher;
    private SpringHibernateInstantiator m_entityInterceptor;
    private UserProfileService m_userProfileService;
    private MongoTemplate m_profilesDb;

    public IntegrationTestCase() {
        setAutowireMode(AUTOWIRE_BY_NAME);
    }

    @Override
    protected void onSetUpBeforeTransaction() throws Exception {
        // w/o this beans loaded from hibernate are not created from spring, therefore not dependency injected
        ((HibernateTransactionManager) transactionManager).setEntityInterceptor(m_entityInterceptor);
    }

    protected void sql(String resource) throws IOException {
        SqlFileReader sql = new SqlFileReader(getClass().getResourceAsStream(ROOT_RES_PATH + resource));
        m_db.batchUpdate(sql.parse().toArray(new String[0]));
    }

    protected void sql(InputStream in) throws IOException {
        SqlFileReader sql = new SqlFileReader(in);
        m_db.batchUpdate(sql.parse().toArray(new String[0]));
    }

    protected JdbcTemplate db() {
        return m_db;
    }

    protected void divertDaoEvents(DaoEventListener listener) {
        m_daoEventPublisher.divertEvents(listener);
    }

    protected void disableDaoEventPublishing() {
        DaoEventListener stub = createNiceMock(DaoEventListener.class);
        m_daoEventPublisher.divertEvents(stub);
    }

    @Override
    protected void onSetUpInTransaction() throws Exception {
        super.onSetUpInTransaction();
        m_modifiedContextObjectMap = new HashMap<Object, Map<String, Object>>();
    }
    @Override
    protected void onTearDownInTransaction() throws Exception {
        super.onTearDownInTransaction();
        if (m_modifiedContextObjectMap != null) {
            resetContext();
        }
        m_profilesDb.dropCollection("userProfile");
    }

    @Override
    public void setDataSource(DataSource dataSource) {
        this.jdbcTemplate = m_db;
    }

    public void setConfigJdbcTemplate(JdbcTemplate db) {
        m_db = db;
    }

    @Override
    protected String[] getConfigLocations() {
        // There are many interdependencies between spring files so in general you need
        // to load them all. However, if you do have isolated spring file, this is definitely
        // overrideable
        return new String[] {
            "classpath:/org/sipfoundry/sipxconfig/system.beans.xml",
            "classpath*:/org/sipfoundry/sipxconfig/*/**/*.beans.xml",
            "classpath*:/sipxplugin.beans.xml"
        };
    }

    @Override
    public void runBare() throws Throwable {
        try {
            super.runBare();
        } catch (SQLException e) {
            dumpSqlExceptionMessages(e);
            throw e;
        } catch (DataIntegrityViolationException e) {
            dumpSqlExceptionMessages(e);
            throw e;
        }
    }

    void dumpSqlExceptionMessages(SQLException e) {
        for (SQLException next = e; next != null; next = next.getNextException()) {
            LOG.info(next.getMessage());
        }
    }

    void dumpSqlExceptionMessages(DataIntegrityViolationException e) {
        if (e.getCause() instanceof SQLException) {
            dumpSqlExceptionMessages((SQLException) e.getCause());
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

    protected void evict(Object o) {
        m_hibernateTemplate.evict(o);
    }

    /**
     * Commit everything to database. useful when debugging and want to run queries from another app. Also
     * useful in some circumstances when subsequent sql requires it.  Note, tests are allowed to leave
     * data in database after execution.  It's up to each test to clear all existing data before execution.
     */
    protected void commit() {
        transactionManager.commit(transactionStatus);
    }

    protected void clear() {
        db().execute("select truncate_all()");
    }

    public void setSessionFactory(SessionFactory sessionFactory) {
        m_sessionFactory = sessionFactory;
        m_hibernateTemplate = new HibernateTemplate();
        m_hibernateTemplate.setSessionFactory(m_sessionFactory);
    }

    public void setSpringInstantiator(SpringHibernateInstantiator entityInterceptor) {
        m_entityInterceptor = entityInterceptor;
    }

    public SpringHibernateInstantiator getEntityInterceptor() {
        return m_entityInterceptor;
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
        if (!m_modifiedContextObjectMap.containsKey(target)) {
            m_modifiedContextObjectMap.put(target, new HashMap<String, Object>());
        }

        Map<String, Object> originalContextObjectValueMap = m_modifiedContextObjectMap.get(target);
        originalContextObjectValueMap.put(propertyName, originalValue);

        try {
            BeanUtils.setProperty(target, propertyName, valueForTest);
        } catch (IllegalAccessException e) {
            LOG.error(format(CANNOT_SET_PROP_MSG, propertyName, target), e);
        } catch (InvocationTargetException e) {
            LOG.error(format(CANNOT_SET_PROP_MSG, propertyName, target), e);
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
                    LOG.error(format(CANNOT_SET_PROP_MSG, propertyName, target), e);
                } catch (InvocationTargetException e) {
                    LOG.error(format(CANNOT_SET_PROP_MSG, propertyName, target), e);
                }
            }

            m_modifiedContextObjectMap.remove(target);
        }

        assertTrue(m_modifiedContextObjectMap.isEmpty());
        m_daoEventPublisher.stopDivertingEvents();
    }

    public void setDaoEventPublisherImpl(DaoEventPublisherImpl daoEventPublisher) {
        m_daoEventPublisher = daoEventPublisher;
    }

    public DaoEventPublisherImpl getDaoEventPublisher() {
        return m_daoEventPublisher;
    }

    public SessionFactory getSessionFactory() {
        return m_sessionFactory;
    }

    public HibernateTemplate getHibernateTemplate() {
        return m_hibernateTemplate;
    }

    public UserProfileService getUserProfileService() {
        return m_userProfileService;
    }

    public void setUserProfileService(UserProfileService service) {
        m_userProfileService = service;
    }

    public MongoTemplate getProfilesDb() {
        return m_profilesDb;
    }

    public void setProfilesDb(MongoTemplate template) {
        m_profilesDb = template;
    }
}
