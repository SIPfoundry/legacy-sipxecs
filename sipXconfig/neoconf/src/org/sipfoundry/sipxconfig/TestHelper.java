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
import org.sipfoundry.sipxconfig.admin.commserver.Location;
import org.sipfoundry.sipxconfig.admin.commserver.LocationsManager;
import org.sipfoundry.sipxconfig.device.Device;
import org.sipfoundry.sipxconfig.device.DeviceTimeZone;
import org.sipfoundry.sipxconfig.device.MemoryProfileLocation;
import org.sipfoundry.sipxconfig.device.TimeZoneManager;
import org.sipfoundry.sipxconfig.device.VelocityProfileGenerator;
import org.sipfoundry.sipxconfig.domain.Domain;
import org.sipfoundry.sipxconfig.domain.DomainManager;
import org.sipfoundry.sipxconfig.phonebook.Phonebook;
import org.sipfoundry.sipxconfig.service.SipxService;
import org.sipfoundry.sipxconfig.service.SipxServiceManager;
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

    private static final String EXAMPLE_ORG = "example.org";

    private static final String EOL = System.getProperty("line.separator");

    private static final String FORWARD_SLASH = "/";
    
    private static Properties s_testProps;

    private static final DateFormat ENGLISH_DATE = DateFormat.getDateTimeInstance(DateFormat.SHORT, DateFormat.FULL,
            Locale.ENGLISH);

    private static Properties s_sysDirProps;

    private static ApplicationContext s_appContext;

    private static DatabaseConnection s_dbunitConnection;

    private static String s_classPathDir;

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
        String sysdir = getTestProperties().getProperty("sysdir.etc");
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
        if (s_classPathDir == null) {
            s_classPathDir = System.getProperty("classpath.dir");
            if (s_classPathDir == null) {
                try {
                    Class c = Class.forName("org.sipfoundry.sipxconfig.ClearDb");
                    s_classPathDir = TestHelper.getClasspathDirectory(c);
                } catch (ClassNotFoundException err) {
                    throw new RuntimeException("You must set 'classpath.dir' system property where to find resources");
                }
            }
        }
        return s_classPathDir;
    }

    public static VelocityEngine getVelocityEngine() {
        try {
            String etcDir = getTestProperties().getProperty("sysdir.etc");
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
        profileGenerator.setTemplateRoot(TestHelper.getTestProperties().getProperty("sysdir.etc"));
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
        String cp = System.getProperty("java.class.path");
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
     *   # Test file in same directory as JUnit test source file
     *   String testFile = TestUtil.getTestSourceDirectory(getClass()) + &quot;/test-file&quot;;
     * </pre>
     */
//    public static String getTestSourceDirectory(Class testClass) {
//        StringBuffer sb = new StringBuffer(TestUtil.getProjectDirectory()).append("/test/").append(
//                testClass.getPackage().getName().replace('.', '/'));
//        String path = sb.toString();
//
//        return path;
//    }

    public static Properties getTestProperties() {
        if (s_testProps == null) {
            s_testProps = new Properties();
            File propsFile = TestHelper.getResourceAsFile("/test.properties");
            InputStream propsStream = null;
            try {
                propsStream = new FileInputStream(propsFile);
                s_testProps.load(propsStream);
            } catch (IOException e) {
                throw new RuntimeException(e);
            } finally {
                IOUtils.closeQuietly(propsStream);                
            }
        }
        return s_testProps;
    }
        
    /**
     * Builds directory in which model files for a specified project (neoconf or any of the
     * plug-ins are located)
     *
     */
    public static String getModelDirectory(String project) {
        String etcDir = getTestProperties().getProperty("sysdir.etc");
        return etcDir;
    }

    /**
     * Finds the directory in which sipXconfig source is located
     */
//    private static String getSourceRootDirectory() {
//        String projectDirectory = getProjectDirectory();
//        int lastIndexOf = projectDirectory.lastIndexOf("sipXconfig");
//        if (lastIndexOf < 0) {
//            throw new RuntimeException("sipXconfig sources have to be in sipXconfig dir");
//        }
//        return projectDirectory.substring(0, lastIndexOf) + "/sipXconfig";
//    }

//    public static String getProjectDirectory() {
//        // eclipse
//        String userDir = System.getProperty("user.dir");
//        // ant
//        return System.getProperty("basedir", userDir);
//    }

    /**
     * Get the directory all autoconf and ant build output gets sent
     */
//    public static String getBuildDirectory(String project) {
//        try {
//            String propName = "top.build.dir";
//            File dir = new File(getProjectDirectory());
//            while (dir != null) {
//                File propFile = new File(dir, propName);
//                if (propFile.exists()) {
//                    Properties props = new Properties();
//                    FileInputStream topBuildDirProperties = new FileInputStream(propFile);
//                    props.load(topBuildDirProperties);
//                    return props.getProperty(propName) + '/' + project;
//                }
//                dir = dir.getParentFile();
//            }
//            throw new RuntimeException(String.format("Cannot find %s in any of the parent of %s.", propName,
//                    getProjectDirectory()));
//        } catch (IOException ioe) {
//            throw new RuntimeException("Could not find top build directory", ioe);
//        }
//    }

    /**
     * Create a sysdir.properties file in the classpath. Uses a trick that will only work if unit
     * tests are unjar-ed. We could do this in ant, but this approach avoids setup and works in
     * IDEs like Eclipse where bin.eclipse is the classpath.
     */
//    public static void setSysDirProperties(Properties sysProps, String etcDirectory, String outputDirectory) {
//
//        // HACK: sysdir.bin is not a real directory when testing
//        final String vxmlDir = outputDirectory + "/vxml";
//        final String mailstoreDir = outputDirectory + "/mailstore";
//        final String binDir = outputDirectory + "/bin";
//        final String tmpDir = outputDirectory + "/tmp";
//        final String sslDir = outputDirectory + "/ssl";
//        final String authDir = sslDir + "/authorities";
//
//        sysProps.setProperty("sysdir.bin", binDir);
//        sysProps.setProperty("sysdir.etc", etcDirectory);
//        sysProps.setProperty("sysdir.data", outputDirectory);
//        sysProps.setProperty("sysdir.share", outputDirectory);
//        sysProps.setProperty("sysdir.thirdparty", outputDirectory);
//        sysProps.setProperty("sysdir.var", outputDirectory);
//        sysProps.setProperty("sysdir.phone", outputDirectory);
//        sysProps.setProperty("sysdir.tmp", tmpDir);
//        sysProps.setProperty("sysdir.log", outputDirectory);
//        sysProps.setProperty("sysdir.doc", outputDirectory);
//        sysProps.setProperty("sysdir.mailstore", mailstoreDir);
//        sysProps.setProperty("sysdir.vxml", vxmlDir);
//        sysProps.setProperty("sysdir.vxml.prompts", vxmlDir + "/prompts");
//        sysProps.setProperty("sysdir.vxml.scripts", vxmlDir + "/scripts");
//        sysProps.setProperty("sysdir.vxml.moh", vxmlDir + "/moh");
//        sysProps.setProperty("sysdir.user", "sipxpbxuser");
//        sysProps.setProperty("sysdir.libexec", outputDirectory);
//        sysProps.setProperty("sysdir.default.firmware", outputDirectory + "/devicefiles");
//        sysProps.setProperty("sysdir.alarmsStrings", etcDirectory);
//        sysProps.setProperty("sipxpbx.mibs.dir", etcDirectory);
//
//        sysProps.setProperty("dataSource.jdbcUrl", "jdbc:postgresql://localhost/SIPXCONFIG_TEST");
//        sysProps.setProperty("acdHistoryDataSource.jdbcUrl", "jdbc:postgresql://localhost/SIPXACD_HISTORY_TEST");
//        sysProps.setProperty("acdHistoricalStatsImpl.enabled", Boolean.toString(true));
//        sysProps.setProperty("cdrDataSource.jdbcUrl", "jdbc:postgresql://localhost/SIPXCDR_TEST");
//        sysProps.setProperty("localBackupPlan.backupDirectory", outputDirectory + "/backup");
//        sysProps.setProperty("ftpBackupPlan.backupDirectory", outputDirectory + "/ftpBackup");
//        sysProps.setProperty("ftpRestore.downloadDirectory", outputDirectory + "/downloadFtpBackup");
//        sysProps.setProperty("orbitsGenerator.audioDirectory", outputDirectory + "/parkserver/music");
//        sysProps.setProperty("replicationTrigger.replicateOnStartup", Boolean.toString(false));
//        sysProps.setProperty("acdContextImpl.enabled", Boolean.toString(true));
//        sysProps.setProperty("indexTrigger.enabled", Boolean.toString(false));
//        sysProps.setProperty("upload.uploadRootDirectory", outputDirectory + "/upload");
//        sysProps.setProperty("upload.destinationDirectory", outputDirectory + "/tftproot");
//        sysProps.setProperty("phonebookManagerImpl.externalUsersDirectory", outputDirectory + "/phonebook");
//        sysProps.setProperty("audiocodesGatewayModel.configDirectory", etcDirectory);
//        sysProps.setProperty("audiocodesFxs.configDirectory", etcDirectory);
//
//        sysProps.setProperty("monitoringContextImpl.enabled", Boolean.toString(true));
//        sysProps.setProperty("coreContextImpl.debug", "on");
//        sysProps.setProperty("sipxconfig.db.user", "postgres");
//        sysProps.setProperty("acdServer.agentPort", "8120");
//        sysProps.setProperty("mrtgTemplateConfig.filename", etcDirectory + "/mrtg-t.cfg");
//        sysProps.setProperty("mrtgConfig.filename", outputDirectory + "/mrtg.cfg");
//        sysProps.setProperty("jasperReportContextImpl.reportsDirectory", etcDirectory + "/reports");
//
//        File vmDir = createDirectory(mailstoreDir, "Could not create voicemail store");
//        createDirectory(tmpDir, "Could not create tmp directory");
//        File ssl = createDirectory(sslDir, "Could not create ssl directory");
//        File auth = createDirectory(authDir, "Could not create auth directory");
//
//        sysProps.setProperty("mailboxManagerImpl.mailstoreDirectory", vmDir.getAbsolutePath());
//        sysProps.setProperty("certificateManagerImpl.sslDirectory", ssl.getAbsolutePath());
//        sysProps.setProperty("certificateManagerImpl.sslAuthDirectory", auth.getAbsolutePath());
//    }
//
//    private static File createDirectory(String directoryName, String errorMessage) {
//        File dir = new File(directoryName);
//        if (!dir.exists()) {
//            if (!dir.mkdirs()) {
//                throw new RuntimeException(errorMessage + dir.getAbsolutePath());
//            }
//        }
//        return dir;
//    }
//
//    public static void saveSysDirProperties(Properties sysProps, String classpathDirectory) {
//        File sysdirPropsFile = new File(classpathDirectory, "sipxconfig.properties");
//        FileOutputStream sysdirPropsStream;
//        try {
//            sysdirPropsStream = new FileOutputStream(sysdirPropsFile);
//            // store them so spring's application context file find it
//            // in classpath
//            sysProps.store(sysdirPropsStream, null);
//        } catch (FileNotFoundException e) {
//            throw new RuntimeException("could not create system dir properties file", e);
//        } catch (IOException e) {
//            throw new RuntimeException("could not store system dir properties", e);
//        }
//    }

    public static File createTempDir(String name) throws IOException {
        File createTempFile = File.createTempFile(name, "dir");
        String tempDirPath = createTempFile.getPath();
        createTempFile.delete();
        File tempDir = new File(tempDirPath);
        tempDir.mkdirs();
        return tempDir;
    }

    /**
     * Creates a mock SipxServiceManager using EasyMock.
     *
     * If replay is set to false, it is up to the user of this mock to call EasyMock.replay. This
     * allows the user to add more functionality to the mock. By default the service manager will
     * do lookups for all of the provides SipxService objects by beanId. If processName is set up,
     * look ups by process names will work as well.
     *
     */
    public static SipxServiceManager getMockSipxServiceManager(boolean replay, SipxService... sipxServices) {
        SipxServiceManager sipxServiceManager = EasyMock.createMock(SipxServiceManager.class);
        for (SipxService sipxService : sipxServices) {
            String beanId = sipxService.getBeanId();
            if (beanId != null) {
                sipxServiceManager.getServiceByBeanId(beanId);
                EasyMock.expectLastCall().andReturn(sipxService).anyTimes();
                sipxServiceManager.isServiceInstalled(beanId);
                EasyMock.expectLastCall().andReturn(true).anyTimes();
            }
            String processName = sipxService.getProcessName();
            if (processName != null) {
                sipxServiceManager.getServiceByName(processName);
                EasyMock.expectLastCall().andReturn(sipxService).anyTimes();
            }
        }

        if (replay) {
            EasyMock.replay(sipxServiceManager);
        }

        return sipxServiceManager;
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
