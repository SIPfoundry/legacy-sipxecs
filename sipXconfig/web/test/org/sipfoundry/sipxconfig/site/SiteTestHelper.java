/*
 * 
 * 
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 * 
 * $
 */
package org.sipfoundry.sipxconfig.site;

import java.io.File;
import java.io.FileInputStream;
import java.io.FileWriter;
import java.io.IOException;
import java.io.InputStream;
import java.lang.reflect.Field;
import java.util.Locale;
import java.util.Properties;

import junit.extensions.TestDecorator;
import junit.framework.Assert;
import junit.framework.AssertionFailedError;
import junit.framework.Test;
import junit.framework.TestResult;
import junit.framework.TestSuite;

import com.meterware.httpunit.HttpUnitOptions;
import com.meterware.httpunit.WebForm;
import com.meterware.httpunit.WebResponse;

import net.sourceforge.jwebunit.HttpUnitDialog;
import net.sourceforge.jwebunit.WebTestCase;
import net.sourceforge.jwebunit.WebTester;

import org.apache.commons.io.FileUtils;
import org.apache.commons.io.IOUtils;
import org.sipfoundry.sipxconfig.setting.Setting;
import org.sipfoundry.sipxconfig.setting.XmlModelBuilder;
import org.sipfoundry.sipxconfig.test.TestUtil;
import org.w3c.dom.Element;

public class SiteTestHelper {

    /**
     * Same userName that TestPage creates. thought of referencing static variable but may pull in
     * unnec. dependencies
     */
    public static final String TEST_USER = "testuser";

    public static final String TEST_PAGE_URL = "/app?page=TestPage&service=page";

    /**
     * The name of the checkbox used in standard tables
     */
    public static final String ROW_CHECKBOX = "checkbox";

    private static String s_buildDir;

    private static String s_baseUrl;

    private static String s_artificialSystemRoot;

    public static Test webTestSuite(Class webTestClass) {
        TestSuite suite = new SipxWebTestSuite(webTestClass);

        JettyTestSetup jetty = new JettyTestSetup(suite);
        s_baseUrl = jetty.getUrl();

        return jetty;
    }
    
    static class SipxWebTestSuite extends TestSuite {
        SipxWebTestSuite(Class test) {
            super(test);
        }
        
        public void addTest(Test test) {
            if (test instanceof WebTestCase) {
                super.addTest(new DumpResponseOnFailure((WebTestCase) test));                
            } else {
                super.addTest(test);
            }
        }
    }
    
    static class DumpResponseOnFailure extends TestDecorator {
        DumpResponseOnFailure(WebTestCase test) {
            super(test);
        }

        public void run(TestResult result) {
            int e = totalFailures(result);
            super.run(result);
            if (e < totalFailures(result)) {
                ((WebTestCase) getTest()).getDialog().dumpResponse();
            }
        }
        
        private int totalFailures(TestResult result) {
            return result.errorCount() + result.failureCount();
        }
    }


    /**
     * Go to TestPage.html and log in. Includes hack for slow machines.
     */
    public static void home(WebTester tester) {
        home(tester, true);

    }

    /**
     * Go to TestPage.html. Log in if the login arg is true. Includes hack for slow machines. And
     * disables javascript be default too
     */
    public static void home(WebTester tester, boolean login) {

        // default is to disable javascript, re-enable at will
        setScriptingEnabled(false);

        tester.beginAt(TEST_PAGE_URL);
        
        // So tests can check for fixed text. tests can reset this if they wish 
        // back to Locale.getDefault()
        tester.getTestContext().setLocale(Locale.ENGLISH);
        
        if (login) {
            tester.clickLink("login");
        }
        tester.clickLink("hideNavigation");
        // HACK: Webunit doesn't appear to fully load page, especially
        // when the machine you're running it on is slow and you're
        // running a batch of tests, calling beginAt("/") twice seems
        // to get webunit to catch up.
        tester.beginAt(TEST_PAGE_URL);
        assertNoException(tester);
    }

    /**
     * Looks for exception stack on tapestry error page. Dumps response if there was an exception.
     */
    public static void assertNoException(WebTester tester) {
        try {
            tester.assertElementNotPresent("exceptionDisplay");
            Assert.assertFalse("Exception".equals(tester.getDialog().getResponsePageTitle()));
        } catch (AssertionFailedError e) {
            tester.dumpResponse(System.err);
            throw e;
        }
    }

    /**
     * Works only for pages that use "user:error:text" id to display user errors. All pages with
     * ErrorMsg component belong to this category.
     */
    public static void assertNoUserError(WebTester tester) {
        assertNoException(tester);
        Element element = tester.getDialog().getElement("user:error");
        if (null != element) {
            tester.dumpResponse(System.err);
            Assert.fail("User error on page: " + element.getFirstChild().getNodeValue());
        }
    }

    public static void assertUserError(WebTester tester) {
        assertNoException(tester);
        Element element = tester.getDialog().getElement("user:error");
        if (null == element) {
            tester.dumpResponse(System.err);
            Assert.fail("Expected user error on the page.");
        }
    }

    /**
     * Returns the row count in a table. Don't forget to include +1 in assert count if you have a
     * table header.
     */
    public static int getRowCount(WebTester tester, String table) {
        return tester.getDialog().getWebTableBySummaryOrId(table).getRowCount();
    }

    /**
     * Translates between Tapestry index and normal index
     * 
     * @param id
     * @param index
     */
    public static void enableCheckbox(WebTester tester, String id, int index, boolean enable) {
        String field = getIndexedId(id, index);
        if (enable) {
            tester.checkCheckbox(field);
        } else {
            tester.uncheckCheckbox(field);
        }
    }

    /**
     * Select/unselect rows in the table Only works if there is a single table on the screen.
     * 
     * @param tester
     * @param index row number starting from 0
     * @param enable True to select checkbox, false otherwise
     */
    public static void selectRow(WebTester tester, int index, boolean enable) {
        String field = getIndexedId(ROW_CHECKBOX, index);
        if (enable) {
            tester.checkCheckbox(field);
        } else {
            tester.uncheckCheckbox(field);
        }
    }

    public static void assertRowSelected(WebTester tester, int index) {
        String field = getIndexedId(ROW_CHECKBOX, index);
        tester.assertCheckboxSelected(field);
    }

    public static void assertRowNotSelected(WebTester tester, int index) {
        String field = getIndexedId(ROW_CHECKBOX, index);
        tester.assertCheckboxNotSelected(field);
    }

    /**
     * Translates between Tapestry index and normal index
     * 
     * @param id HTML element id
     * @param index tapestry index
     */
    public static String getIndexedId(String id, int index) {
        String suffix = "";
        if (index > 0) {
            suffix = "_" + (index - 1);
        }
        return id + suffix;
    }

    public static void assertOptionSelected(WebTester tester, String formElement, String expected) {
        String value = tester.getDialog().getSelectedOption(formElement);
        Assert.assertEquals(expected, value);
    }

    public static String getBaseUrl() {
        return s_baseUrl;
    }

    public static String getClasspathDirectory() {
        return TestUtil.getClasspathDirectory(SiteTestHelper.class);
    }

    public static String getBuildDirectory() {
        if (s_buildDir == null) {
            s_buildDir = TestUtil.getBuildDirectory("web");
        }

        return s_buildDir;
    }

    /**
     * Get the root directory mimicking an installed sipx system. Useful when web pages need to
     * reference files from other sipx projects. Unittest should copy in seed test files.
     */
    public static String getArtificialSystemRootDirectory() {
        if (null == s_artificialSystemRoot) {
            s_artificialSystemRoot = TestUtil.getTestOutputDirectory("web")
                    + "/artificial-system-root";
        }
        return s_artificialSystemRoot;
    }

    /**
     * Create a dir if it doesn't exists and deletes all contents if it does exist
     */
    public static String cleanDirectory(String directory) {
        File dir = new File(directory);
        if (!dir.exists()) {
            dir.mkdirs();
        } else {
            try {
                FileUtils.cleanDirectory(dir);
            } catch (IOException ioe) {
                throw new RuntimeException("Could not clean directory " + directory, ioe);
            }
        }

        return directory;
    }

    /**
     * Get the full path and copy file from etc incase there are modiifcations to it
     * 
     * @param path relative to etc dir e.g. "kphone/phone.xml"
     * @return full path to config file
     */
    public static String getFreshFileInArtificialSystemRoot(String path) {
        String neopath = TestUtil.getProjectDirectory() + "/../neoconf/etc" + path;
        String webpath = getArtificialSystemRootDirectory() + "/" + path;
        try {
            FileWriter out = new FileWriter(webpath);
            IOUtils.write(neopath, out);
            IOUtils.closeQuietly(out);
            return webpath;
        } catch (IOException ioe) {
            throw new RuntimeException(ioe);
        }
    }

    public static Setting loadSettings(String path) {
        String sysdir = TestUtil.getProjectDirectory() + "/../neoconf/etc";
        XmlModelBuilder builder = new XmlModelBuilder(sysdir);
        Setting settings = builder.buildModel(new File(sysdir + "/" + path));

        return settings;
    }

    /**
     * Write out sipxconfig.properties for testing arg 0 - any path in the testing classpath arg 1 -
     * path to artificial root directory arg 2 - where output is generated
     */
    public static void main(String[] args) {
        Properties sysProps = new Properties();
        s_artificialSystemRoot = args[0];
        String systemDirectory = cleanDirectory(args[1]);
        String etcDirectory = systemDirectory + "/etc";

        // generates sipxconfig.properties in classpath (arg 0)
        TestUtil.setSysDirProperties(sysProps, etcDirectory, args[2]);

        // overwrite several properties that have to have "real" values
        sysProps.setProperty("localTftp.uploadDirectory", systemDirectory + "/tftproot");
        sysProps.setProperty("vxml.promptsDirectory", systemDirectory + "/prompts");
        sysProps.setProperty("vxml.scriptsDirectory", systemDirectory + "/aa_vxml");
        sysProps.setProperty("orbitsGenerator.audioDirectory", systemDirectory
                + "/parkserver/music");
        sysProps.setProperty("acdQueue.audioDirectory", systemDirectory + "/acd/audio");
        TestUtil.saveSysDirProperties(sysProps, args[0]);
    }

    private static Properties s_sysProps;

    public static String getTftpDirectory() {
        return getSystemProperties().getProperty("localTftp.uploadDirectory");
    }

    private static Properties getSystemProperties() {
        if (s_sysProps == null) {
            s_sysProps = new Properties();
            File sipxconfig = new File(getBuildDirectory()
                    + "/tests/war/WEB-INF/classes/sipxconfig.properties");
            try {
                InputStream sipxconfigSteam = new FileInputStream(sipxconfig);
                s_sysProps.load(sipxconfigSteam);
            } catch (IOException e) {
                throw new RuntimeException(e);
            }
        }
        return s_sysProps;
    }

    /**
     * Utility function to click on Tapestry submit forms without a button
     * 
     * Make JWebUnit happy: it does not know we submitted form request independently, we need to
     * reinject the response back into the dialog. I could not find any reasonable way of doing
     * that, so I used reflection to set private field. I guess being able to test more pages is
     * the most important factor here.
     * 
     * There is no guarantee that it will work with new version of Tapestry or JWebUnit
     * 
     */
    public static void submitNoButton(WebTester tester) throws Exception {
        // submit the form after setting hidden field
        HttpUnitDialog dialog = tester.getDialog();
        WebForm form = dialog.getForm();

        WebResponse response = form.submitNoButton();

        // set response directly in current JWebUnit object
        Class klass = dialog.getClass();
        Field respField = klass.getDeclaredField("resp");
        respField.setAccessible(true);
        respField.set(dialog, response);

        Assert.assertSame(tester.getDialog().getResponse(), response);
    }

    public static void seedUser(WebTester tester) {
        home(tester);
        tester.clickLink("resetCoreContext");
        home(tester);
        tester.clickLink("seedTestUser");
    }

    /**
     * Create a new group, user or phone
     * 
     * @param pageLinkId From the TestPage, what link to click to get to new group page
     */
    public static void seedGroup(WebTester tester, String pageLinkId, int count) {
        SiteTestHelper.home(tester);
        for (int i = 0; i < count; i++) {
            tester.clickLink(pageLinkId);
            tester.setFormElement("name", "seedGroup" + i);
            tester.clickButton("form:ok");
            SiteTestHelper.home(tester);
        }
    }

    /**
     * Initializes upload elements on the form using ad hoc created temporary file.
     * 
     * Tapestry forms that contain Upload elements cannot be tested by HTTPUnit unless upload
     * elements are initialized. Looks like HTTPUnit never bother to submit this fields if they
     * are empty which does not stop tapestry from trying to parse them. NullPointerException in
     * Upload.rewindFormComponent is the usual sign that this is a problem.
     * 
     * @param form form for which upload fields will be initialized
     * @param fileNamePrefix at least 3 chatracters - use test name
     */
    public static void initUploadFields(WebForm form, String fileNamePrefix) {
        try {
            File file = File.createTempFile(fileNamePrefix, null);
            String[] parameterNames = form.getParameterNames();
            for (int i = 0; i < parameterNames.length; i++) {
                String paramName = parameterNames[i];
                if (paramName.startsWith("promptUpload")) {
                    form.setParameter(paramName, file);
                }
            }
        } catch (IOException e) {
            throw new RuntimeException(e);
        }
    }

    /**
     * Turn on/off javascript, make sure to restore state to true after you're done
     */
    public static boolean setScriptingEnabled(boolean enabled) {
        boolean old = HttpUnitOptions.isScriptingEnabled();
        HttpUnitOptions.setScriptingEnabled(enabled);
        return old;
    }
}
