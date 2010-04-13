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

import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.net.URL;
import java.sql.Connection;
import java.sql.SQLException;
import java.util.Arrays;
import java.util.Collection;
import java.util.HashSet;
import java.util.Properties;
import java.util.Set;

import javax.sql.DataSource;

import junit.framework.TestCase;
import org.apache.commons.io.IOUtils;
import org.apache.velocity.app.VelocityEngine;
import org.dbunit.database.DatabaseConfig;
import org.dbunit.database.DatabaseConnection;
import org.dbunit.database.IDatabaseConnection;
import org.dbunit.dataset.IDataSet;
import org.dbunit.dataset.ReplacementDataSet;
import org.dbunit.dataset.xml.FlatXmlDataSet;
import org.dbunit.dataset.xml.XmlDataSet;
import org.dbunit.operation.DatabaseOperation;
import org.easymock.EasyMock;
import org.easymock.IMocksControl;
import org.easymock.internal.matchers.ArrayEquals;
import org.easymock.internal.matchers.Equals;
import org.sipfoundry.sipxconfig.device.Device;
import org.sipfoundry.sipxconfig.device.DeviceTimeZone;
import org.sipfoundry.sipxconfig.device.MemoryProfileLocation;
import org.sipfoundry.sipxconfig.device.TimeZoneManager;
import org.sipfoundry.sipxconfig.device.VelocityProfileGenerator;
import org.sipfoundry.sipxconfig.domain.Domain;
import org.sipfoundry.sipxconfig.domain.DomainManager;
import org.sipfoundry.sipxconfig.setting.ModelBuilder;
import org.sipfoundry.sipxconfig.setting.ModelFilesContext;
import org.sipfoundry.sipxconfig.setting.ModelFilesContextImpl;
import org.sipfoundry.sipxconfig.setting.Setting;
import org.sipfoundry.sipxconfig.setting.XmlModelBuilder;
import org.sipfoundry.sipxconfig.test.TestUtil;
import org.springframework.beans.factory.access.BeanFactoryLocator;
import org.springframework.beans.factory.access.BeanFactoryReference;
import org.springframework.context.ApplicationContext;
import org.springframework.context.access.ContextSingletonBeanFactoryLocator;
import org.springframework.dao.DataIntegrityViolationException;

import static org.easymock.EasyMock.aryEq;
import static org.easymock.EasyMock.reportMatcher;

/**
 * TestHelper: used for unit tests that need Spring instantiated
 */
public final class TestHelper {

    private static Properties s_sysDirProps;

    private static ApplicationContext s_appContext;

    private static DatabaseConnection s_dbunitConnection;

    static {
        // The default XML parser (Apache Crimson) cannot resolve relative DTDs, google to find
        // the bug.
        System.setProperty("org.xml.sax.driver", "org.apache.xerces.parsers.SAXParser");

        // The method SAXParserFactory.newInstance is using this system property to find the
        // parser factory.
        // So we have to set this property instead of, or in addition to, the one above.
        // Fixes XCF-537: jdk dependency in DB unit tests, fails with jdk 1.4.2, works with jdk
        // 1.5.
        System.setProperty("javax.xml.parsers.SAXParserFactory", "org.apache.xerces.jaxp.SAXParserFactoryImpl");

        // Activate log configuration on test/log4j.properties
        // System.setProperty("org.apache.commons.logging.Log",
        // "org.apache.commons.logging.impl.Log4JLogger");
    }

    public static ApplicationContext getApplicationContext() {
        if (s_appContext == null) {
            getSysDirProperties();
            BeanFactoryLocator bfl = ContextSingletonBeanFactoryLocator.getInstance();
            BeanFactoryReference bfr = bfl.useBeanFactory("servicelayer-context");
            s_appContext = (ApplicationContext) bfr.getFactory();
        }

        return s_appContext;
    }

    public static DomainManager getTestDomainManager(String domain) {
        Domain exampleDomain = new Domain(domain);
        IMocksControl domainManagerControl = EasyMock.createControl();
        DomainManager domainManager = domainManagerControl.createMock(DomainManager.class);
        domainManager.getDomain();
        domainManagerControl.andReturn(exampleDomain).anyTimes();
        domainManagerControl.replay();
        return domainManager;
    }

    public static TimeZoneManager getTimeZoneManager(DeviceTimeZone tz) {
        IMocksControl timeZoneManagerControl = EasyMock.createControl();
        TimeZoneManager tzm = timeZoneManagerControl.createMock(TimeZoneManager.class);
        tzm.getDeviceTimeZone();
        timeZoneManagerControl.andReturn(tz).anyTimes();
        timeZoneManagerControl.replay();
        return tzm;
    }

    public static ModelFilesContext getModelFilesContext() {
        String sysdir = getSettingModelContextRoot();
        return getModelFilesContext(sysdir);
    }

    public static ModelFilesContext getModelFilesContext(String modelDir) {
        ModelFilesContextImpl mfc = new ModelFilesContextImpl();
        mfc.setConfigDirectory(modelDir);
        XmlModelBuilder builder = new XmlModelBuilder(modelDir);
        mfc.setModelBuilder(builder);
        return mfc;
    }

    public static XmlModelBuilder getModelBuilder() {
        ModelFilesContextImpl mfc = new ModelFilesContextImpl();
        String sysdir = getSettingModelContextRoot();
        mfc.setConfigDirectory(sysdir);
        XmlModelBuilder builder = new XmlModelBuilder(sysdir);
        mfc.setModelBuilder(builder);
        return builder;
    }

    public static String getSettingModelContextRoot() {
        String sysdir = getSysDirProperties().getProperty("sysdir.etc");
        return sysdir;
    }

    public static Setting loadSettings(String path) {
        Setting settings = getModelFilesContext().loadModelFile(path);
        return settings;
    }

    public static Setting loadSettings(Class klass, String resource) {
        ModelBuilder builder = new XmlModelBuilder("etc");
        File in = getResourceAsFile(klass, resource);
        return builder.buildModel(in);
    }

    public static String getClasspathDirectory() {
        return TestUtil.getClasspathDirectory(TestHelper.class);
    }

    public static VelocityEngine getVelocityEngine() {

        try {
            Properties sysdir = getSysDirProperties();

            String etcDir = sysdir.getProperty("sysdir.etc");

            VelocityEngine engine = new VelocityEngine();
            engine.setProperty("resource.loader", "file");
            engine.setProperty("file.resource.loader.path", etcDir);
            engine.init();

            return engine;
        } catch (Exception e) {
            throw new RuntimeException(e);
        }
    }

    public static VelocityProfileGenerator getProfileGenerator() {
        VelocityProfileGenerator profileGenerator = new VelocityProfileGenerator();
        profileGenerator.setVelocityEngine(getVelocityEngine());
        profileGenerator.setTemplateRoot(getSettingModelContextRoot());
        return profileGenerator;
    }

    /**
     * Sets velocity profile generator that generates profile to memory and can be used during
     * testing.
     *
     * @param device
     */
    public static MemoryProfileLocation setVelocityProfileGenerator(Device device) {
        MemoryProfileLocation location = new MemoryProfileLocation();
        VelocityProfileGenerator profileGenerator = new VelocityProfileGenerator();
        profileGenerator.setVelocityEngine(getVelocityEngine());
        profileGenerator.setTemplateRoot(getSysDirProperties().getProperty("sysdir.etc"));
        device.setProfileGenerator(profileGenerator);

        return location;
    }

    public static String getTestDirectory() {
        return TestUtil.getTestOutputDirectory("neoconf");
    }

    public static Properties getSysDirProperties() {
        if (s_sysDirProps == null) {
            String classpathDirectory = getClasspathDirectory();
            initSysDirProperties(classpathDirectory);
        }
        return s_sysDirProps;
    }

    public static void initSysDirProperties(String dir) {
        String etcDir = TestUtil.getProjectDirectory() + "/etc";
        String outDir = getTestDirectory();
        s_sysDirProps = new Properties();
        TestUtil.setSysDirProperties(s_sysDirProps, etcDir, outDir);
        TestUtil.saveSysDirProperties(s_sysDirProps, dir);
    }

    public static IDatabaseConnection getConnection() throws Exception {
        if (s_dbunitConnection != null) {
            return s_dbunitConnection;
        }
        Class.forName("com.p6spy.engine.spy.P6SpyDriver");

        DataSource ds = (DataSource) getApplicationContext().getBean("dataSource");
        Connection jdbcConnection = ds.getConnection();
        s_dbunitConnection = new DatabaseConnection(jdbcConnection);
        DatabaseConfig config = s_dbunitConnection.getConfig();
        config.setFeature("http://www.dbunit.org/features/batchedStatements", true);

        return s_dbunitConnection;
    }

    public static void closeConnection() throws SQLException {
        if (s_dbunitConnection != null && !s_dbunitConnection.getConnection().isClosed()) {
            s_dbunitConnection.close();
            s_dbunitConnection = null;
        }
    }

    public static IDataSet loadDataSet(String fileResource) throws Exception {
        InputStream datasetStream = TestHelper.class.getResourceAsStream(fileResource);
        return new XmlDataSet(datasetStream);
    }

    public static IDataSet loadDataSetFlat(String resource) throws Exception {
        InputStream datasetStream = TestHelper.class.getResourceAsStream(resource);
        // we do not want to use metadata since DBTestUnit resolves relative DTDs incorrectly
        // we are checking XML validity in separate Ant tasks (test-dataset)
        return new FlatXmlDataSet(datasetStream, false);
    }

    public static ReplacementDataSet loadReplaceableDataSetFlat(String fileResource) throws Exception {
        IDataSet ds = loadDataSetFlat(fileResource);
        ReplacementDataSet relaceable = new ReplacementDataSet(ds);
        relaceable.addReplacementObject("[null]", null);
        return relaceable;
    }

    public static void cleanInsert(String resource) throws Exception {
        try {
            DatabaseOperation.CLEAN_INSERT.execute(getConnection(), loadDataSet(resource));
        } catch (SQLException e) {
            throw e.getNextException();
        }
    }

    public static void cleanInsertFlat(String resource) throws Exception {
        DatabaseOperation.CLEAN_INSERT.execute(getConnection(), loadDataSetFlat(resource));
    }

    public static void insertFlat(String resource) throws Exception {
        DatabaseOperation.INSERT.execute(getConnection(), loadDataSetFlat(resource));
    }

    public static void update(String resource) throws Exception {
        DatabaseOperation.UPDATE.execute(getConnection(), loadDataSet(resource));
    }

    /**
     * Special TestCase class that catches prints additional info for SQL Exceptions errors that
     * may happed during setUp, testXXX and tearDown.
     *
     * Alternatively we could just throw e.getNextException, but we may want to preserve the
     * original exception.
     */
    public static class TestCaseDb extends TestCase {
        @Override
        public void runBare() throws Throwable {
            try {
                super.runBare();
            } catch (SQLException e) {
                dumpSqlExceptionMessages(e);
                throw e;
            } catch (DataIntegrityViolationException e) {
                if (e.getCause() instanceof SQLException) {
                    dumpSqlExceptionMessages((SQLException) e.getCause());
                }
                throw e;
            }
        }

        private void dumpSqlExceptionMessages(SQLException e) {
            for (SQLException next = e; next != null; next = next.getNextException()) {
                System.err.println(next.getMessage());
            }
        }
    }

    public static class ArrayElementsEquals extends ArrayEquals {
        public ArrayElementsEquals(Object expected) {
            super(expected);
        }

        @Override
        public boolean matches(Object actual) {
            Object expected = getExpected();

            if (expected instanceof Object[] && (actual == null || actual instanceof Object[])) {
                Set expectedSet = new HashSet(Arrays.asList((Object[]) expected));
                Set actualSet = new HashSet(Arrays.asList((Object[]) actual));

                return expectedSet.equals(actualSet);
            }
            return super.matches(actual);

        }
    }

    public static class CollectionEquals extends Equals {
        public CollectionEquals(Object expected) {
            super(expected);
        }

        @Override
        public boolean matches(Object actual) {
            Object expected = getExpected();

            if (expected instanceof Collection && (actual == null || actual instanceof Collection)) {
                Set expectedSet = new HashSet(((Collection) expected));
                Set actualSet = new HashSet(((Collection) actual));

                return expectedSet.equals(actualSet);
            }
            return super.matches(actual);
        }
    }

    /**
     * Use in test to create copy of example files to be changed by test methods.
     *
     * @param from input stream
     * @param dir destination directory
     * @param filename destination file name
     * @throws IOException
     */
    public static final void copyStreamToDirectory(InputStream from, String dir, String filename) throws IOException {
        FileOutputStream to = new FileOutputStream(new File(dir, filename));
        IOUtils.copy(from, to);
        IOUtils.closeQuietly(to);
        IOUtils.closeQuietly(from);
    }

    /**
     * Retrieves the file corresponding to the class resource
     *
     * @param klass
     * @param resource resource name relative to class
     * @return file that can be opened and used to read resource
     */
    public static File getResourceAsFile(Class klass, String resource) {
        URL url = klass.getResource(resource);
        return new File(url.getFile());
    }

    public static <T> T[] asArrayElems(T... items) {
        reportMatcher(new ArrayElementsEquals(items));
        return null;
    }

    public static <T> T[] asArray(T... items) {
        return aryEq(items);
    }
}
