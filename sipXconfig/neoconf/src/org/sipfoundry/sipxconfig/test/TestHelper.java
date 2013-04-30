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

import static org.easymock.EasyMock.aryEq;
import static org.easymock.EasyMock.reportMatcher;

import java.io.File;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.StringWriter;
import java.net.URL;
import java.security.CodeSource;
import java.sql.Connection;
import java.sql.SQLException;
import java.text.DateFormat;
import java.text.ParseException;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collection;
import java.util.Date;
import java.util.HashSet;
import java.util.Locale;
import java.util.Properties;
import java.util.Set;

import javax.sql.DataSource;

import junit.framework.Assert;
import junit.framework.TestCase;

import org.apache.commons.io.IOUtils;
import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import org.apache.velocity.app.VelocityEngine;
import org.dbunit.database.DatabaseConfig;
import org.dbunit.database.DatabaseConnection;
import org.dbunit.database.IDatabaseConnection;
import org.dbunit.dataset.IDataSet;
import org.dbunit.dataset.ReplacementDataSet;
import org.dbunit.dataset.xml.FlatXmlDataSet;
import org.dbunit.dataset.xml.XmlDataSet;
import org.dbunit.operation.DatabaseOperation;
import org.dom4j.Document;
import org.dom4j.io.OutputFormat;
import org.dom4j.io.XMLWriter;
import org.easymock.EasyMock;
import org.easymock.IMocksControl;
import org.easymock.internal.matchers.ArrayEquals;
import org.easymock.internal.matchers.Equals;
import org.sipfoundry.sipxconfig.commserver.Location;
import org.sipfoundry.sipxconfig.commserver.LocationsManager;
import org.sipfoundry.sipxconfig.device.Device;
import org.sipfoundry.sipxconfig.device.DeviceTimeZone;
import org.sipfoundry.sipxconfig.device.FileSystemProfileLocation;
import org.sipfoundry.sipxconfig.device.TimeZoneManager;
import org.sipfoundry.sipxconfig.device.VelocityProfileGenerator;
import org.sipfoundry.sipxconfig.domain.Domain;
import org.sipfoundry.sipxconfig.domain.DomainManager;
import org.sipfoundry.sipxconfig.domain.DomainManagerImpl;
import org.sipfoundry.sipxconfig.phonebook.Phonebook;
import org.sipfoundry.sipxconfig.setting.ModelBuilder;
import org.sipfoundry.sipxconfig.setting.ModelFilesContext;
import org.sipfoundry.sipxconfig.setting.ModelFilesContextImpl;
import org.sipfoundry.sipxconfig.setting.Setting;
import org.sipfoundry.sipxconfig.setting.XmlModelBuilder;
import org.springframework.beans.factory.access.BeanFactoryLocator;
import org.springframework.beans.factory.access.BeanFactoryReference;
import org.springframework.context.ApplicationContext;
import org.springframework.context.access.ContextSingletonBeanFactoryLocator;
import org.springframework.dao.DataIntegrityViolationException;

/**
* TestHelper: used for unit tests that need Spring instantiated
*/
public final class TestHelper {
    private static final String ROOT_RES_PATH = "/org/sipfoundry/sipxconfig/";

    private static final Log LOG = LogFactory.getLog(TestHelper.class);

    private static final String EXAMPLE_ORG = "example.org";

    private static final String EOL = System.getProperty("line.separator");

    private static final String FORWARD_SLASH = "/";

    private static Properties s_testProps;

    private static Properties s_configProps;

    private static final DateFormat ENGLISH_DATE = DateFormat.getDateTimeInstance(DateFormat.SHORT, DateFormat.FULL,
            Locale.ENGLISH);

    private static ApplicationContext s_appContext;

    private static DatabaseConnection s_dbunitConnection;

    static {
        ENGLISH_DATE.setLenient(true);

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

    private TestHelper() {
    }

    public static ApplicationContext getApplicationContext() {
        if (s_appContext == null) {
            BeanFactoryLocator bfl = ContextSingletonBeanFactoryLocator.getInstance();
            BeanFactoryReference bfr = bfl.useBeanFactory("servicelayer-context");
            s_appContext = (ApplicationContext) bfr.getFactory();
        }

        return s_appContext;
    }

    public static DomainManager getTestDomainManager(String domain) {
        Domain exampleDomain = new Domain(domain);
        exampleDomain.setNetworkName(domain);
        exampleDomain.setName(domain);
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
        return getModelFilesContext(getSystemEtcDir());
    }

    public static ModelFilesContext getModelFilesContext(String modelDir) {
        ModelFilesContextImpl mfc = new ModelFilesContextImpl();
        mfc.setConfigDirectory(modelDir);
        XmlModelBuilder builder = new XmlModelBuilder(modelDir);
        mfc.setModelBuilder(builder);
        return mfc;
    }

    public static Setting loadSettings(File file) {
        ModelBuilder builder = new XmlModelBuilder(file.getParent());
        return builder.buildModel(file);
    }

    public static Setting loadSettings(String etcDir, String path) {
        ModelFilesContext mfc = getModelFilesContext(etcDir);
        Setting s = mfc.loadModelFile(path);
        return s;
    }

    public static Setting loadSettings(String path) {
        ModelFilesContext mfc = getModelFilesContext();
        Setting s = mfc.loadModelFile(path);
        return s;
    }

    public static String getEtcDir() {
        // requires adding to test.properties in Makefile.am
        return getTestProperties().getProperty("local.etc.dir");
    }

    public static String getSystemEtcDir() {
        return getTestProperties().getProperty("SIPX_CONFDIR");
    }

    public static VelocityEngine getVelocityEngine() {
        return getVelocityEngine(getSystemEtcDir());
    }

    public static VelocityEngine getVelocityEngine(String etcDir) {
        try {
            VelocityEngine engine = new VelocityEngine();
            engine.setProperty("resource.loader", "file");
            engine.setProperty("file.resource.loader.path", etcDir);
            engine.init();

            return engine;
        } catch (Exception e) {
            throw new RuntimeException(e);
        }
    }

    public static VelocityProfileGenerator getProfileGenerator(String etcDir) {
        VelocityProfileGenerator profileGenerator = new VelocityProfileGenerator();
        profileGenerator.setVelocityEngine(getVelocityEngine(etcDir));
        profileGenerator.setTemplateRoot(etcDir);
        return profileGenerator;
    }

    /**
* Sets velocity profile generator that generates profile to memory and can be used during
* testing.
*
* @param device
*/
    public static MemoryProfileLocation setVelocityProfileGenerator(Device device, String etcDir) {
        MemoryProfileLocation location = new MemoryProfileLocation();
        VelocityProfileGenerator profileGenerator = new VelocityProfileGenerator();
        profileGenerator.setVelocityEngine(getVelocityEngine(etcDir));
        profileGenerator.setTemplateRoot(etcDir);
        device.setProfileGenerator(profileGenerator);
        return location;
    }

    public static FileSystemProfileLocation setFsVelocityProfileGenerator(Device device, String etcDir) {
        FileSystemProfileLocation location = new FileSystemProfileLocation();
        VelocityProfileGenerator profileGenerator = new VelocityProfileGenerator();
        profileGenerator.setVelocityEngine(getVelocityEngine(etcDir));
        profileGenerator.setTemplateRoot(etcDir);
        device.setProfileGenerator(profileGenerator);
        return location;
    }

    public static String getTestDirectory() {
        return TestHelper.getTestOutputDirectory();
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
        InputStream datasetStream = TestHelper.class.getResourceAsStream(ROOT_RES_PATH + fileResource);
        return new XmlDataSet(datasetStream);
    }

    public static IDataSet loadDataSetFlat(String resource) throws Exception {
        InputStream datasetStream = TestHelper.class.getResourceAsStream(ROOT_RES_PATH + resource);
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
                LOG.info(next.getMessage());
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

    public static String getSourceDirectory(Class klass) {
        String n = klass.getSimpleName();
        return getResourceAsFile(klass, klass.getSimpleName() + ".java").getParent();
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

    /**
* Retrieves the file corresponding to the class resource
*
* @param klass
* @param resource resource name relative to class
* @return file that can be opened and used to read resource
*/
    public static File getResourceAsFile(String resource) {
        URL url = TestHelper.class.getResource(resource);
        return url == null ? null : new File(url.getFile());
    }

    public static <T> T[] asArrayElems(T... items) {
        reportMatcher(new ArrayElementsEquals(items));
        return null;
    }

    public static <T> T[] asArray(T... items) {
        return aryEq(items);
    }

    public static final boolean isWindows() {
        return File.separatorChar == '\\';
    }

    /**
* If you want to use a date in a unit test that eventually
*
* @param usDate typical US date representation
* @return local date
*/
    public static final Date localizeDateTime(String usDate) {
        try {
            return ENGLISH_DATE.parse(usDate);
        } catch (ParseException e) {
            throw new RuntimeException(e);
        }
    }

    /**
* Change "\n" the the native end-of-line char. On unix, this does nothing on windows, this
* add "\r\n"
*/
    public static final String cleanEndOfLines(String s) {
        String clean = s.replaceAll("\n", EOL);
        return clean;
    }

    public static final String currentDrive() {
        if (!isWindows()) {
            return "";
        }

        String drive = new File(FORWARD_SLASH).getAbsolutePath().substring(0, 2);
        return drive;
    }

    /**
* The directory that is part of the classpath that a class was loaded from
*/
    public static String getClasspathDirectory(Class testClass) {
        // create file on classpath
        CodeSource code = testClass.getProtectionDomain().getCodeSource();
        URL classpathUrl = code.getLocation();
        File classpathDir = new File(classpathUrl.getFile());

        return classpathDir.getAbsolutePath();
    }

    /**
* Where to direct test output, cleaned on 'ant clean' and ignored by subversion
*/
    public static String getTestOutputDirectory() {
        return "test-results";
    }

    /**
* Use ClassLoader.getSystemResource() when you can gets you a stream and can work from jars,
* but when you need a filename use this. Example:
*
* <pre>
* # Test file in same directory as JUnit test source file
* String testFile = TestUtil.getTestSourceDirectory(getClass()) + &quot;/test-file&quot;;
* </pre>
*/
// public static String getTestSourceDirectory(Class testClass) {
// StringBuffer sb = new StringBuffer(TestUtil.getProjectDirectory()).append("/test/").append(
// testClass.getPackage().getName().replace('.', '/'));
// String path = sb.toString();
//
// return path;
// }

    public static Properties getTestProperties() {
        if (s_testProps == null) {
            s_testProps = loadProperties("/test.properties");
        }
        return s_testProps;
    }

    public static Properties getConfigProperties() {
        if (s_configProps == null) {
            s_configProps = loadProperties("/sipxconfig.properties");
        }
        return s_configProps;
    }

    private static Properties loadProperties(String resource) {
        Properties testProps = new Properties();
        File propsFile = TestHelper.getResourceAsFile(resource);
        InputStream propsStream = null;
        try {
            LOG.info("Loading test properties " + propsFile.getPath());
            propsStream = new FileInputStream(propsFile);
            testProps.load(propsStream);
        } catch (IOException e) {
            throw new RuntimeException(e);
        } finally {
            IOUtils.closeQuietly(propsStream);
        }
        return testProps;
    }

    public static File createTempDir(String name) throws IOException {
        File createTempFile = File.createTempFile(name, "dir");
        String tempDirPath = createTempFile.getPath();
        createTempFile.delete();
        File tempDir = new File(tempDirPath);
        tempDir.mkdirs();
        return tempDir;
    }

    /**
* Creates a mock domain manager using EasyMock. Up to the caller to call replay on the mock.
*/
    public static DomainManager getMockDomainManager() {
        return getMockDomainManager(false);
    }

    public static DomainManager getMockDomainManager(boolean replay) {
        Domain domain = new Domain();
        domain.setName(EXAMPLE_ORG);
        domain.setSipRealm(EXAMPLE_ORG);

        DomainManager domainManager = EasyMock.createMock(DomainManager.class);
        domainManager.getDomain();
        EasyMock.expectLastCall().andReturn(domain).anyTimes();
        if (replay) {
            EasyMock.replay(domainManager);
        }
        return domainManager;
    }

    /**
* Creates a default location for use in tests
*/
    public static Location createDefaultLocation() {
        Location location = new Location();
        location.setName("localLocation");
        location.setFqdn("sipx.example.org");
        location.setAddress("192.168.1.1");
        return location;
    }

    /**
* Creates a default location for use in tests
*/
    public static void initDefaultDomain() {
        Domain domain = new Domain();
        domain.setNetworkName(EXAMPLE_ORG);
        domain.setName(EXAMPLE_ORG);
        DomainManagerImpl manager = new DomainManagerImpl();
        manager.setTestDomain(domain);
    }

    /**
* Gets mock user group phonebooks
*/
    public static Collection<Phonebook> getMockPublicPhonebooks() {
        Phonebook phonebook = new Phonebook();
        Collection<Phonebook> phonebooks = new ArrayList<Phonebook>();
        phonebooks.add(phonebook);
        return phonebooks;
    }

    /**
* Gets mock all user phonebooks
*/
    public static Collection<Phonebook> getMockAllPhonebooks() {
        Phonebook privatePhonebook = new Phonebook();
        Collection<Phonebook> phonebooks = getMockPublicPhonebooks();
        Collection<Phonebook> allPhonebooks = new ArrayList<Phonebook>();
        allPhonebooks.addAll(phonebooks);
        allPhonebooks.add(privatePhonebook);
        return phonebooks;
    }

    /**
* Creates a mock LocationsManager with the specified locations. This LocationsManager only
* responds to requests for the primary service.
*
*/
    public static LocationsManager getMockLocationsManager() {
        LocationsManager locationsManager = EasyMock.createMock(LocationsManager.class);
        locationsManager.getPrimaryLocation();
        EasyMock.expectLastCall().andReturn(createDefaultLocation()).anyTimes();
        EasyMock.replay(locationsManager);
        return locationsManager;
    }

    /**
* Dumps DOM4J document to Strings.
*
* @param doc DOM4J document
* @return String containing XML document
*/
    public static String asString(Document doc) {
        try {
            StringWriter writer = new StringWriter();
            OutputFormat format = new OutputFormat();
            format.setNewlines(true);
            format.setIndent(true);
            XMLWriter xmlWriter = new XMLWriter(writer, format);
            xmlWriter.write(doc);
            xmlWriter.close();
            return writer.toString();
        } catch (IOException e) {
            e.printStackTrace(System.err);
            Assert.fail(e.getMessage());
            return null;
        }
    }
}
