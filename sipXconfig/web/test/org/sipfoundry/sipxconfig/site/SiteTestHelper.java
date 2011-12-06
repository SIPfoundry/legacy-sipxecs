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
import java.io.IOException;
import java.io.InputStream;
import java.util.ArrayList;
import java.util.Locale;
import java.util.Properties;

import junit.extensions.TestDecorator;
import junit.framework.AssertionFailedError;
import junit.framework.Test;
import junit.framework.TestResult;
import junit.framework.TestSuite;
import net.sourceforge.jwebunit.html.Cell;
import net.sourceforge.jwebunit.html.Row;
import net.sourceforge.jwebunit.html.Table;
import net.sourceforge.jwebunit.junit.WebTestCase;
import net.sourceforge.jwebunit.junit.WebTester;

import org.apache.commons.io.FileUtils;
import org.apache.commons.io.IOUtils;
import org.apache.commons.lang.StringUtils;
import org.apache.tapestry.form.FormConstants;
import org.junit.Assert;
import org.sipfoundry.sipxconfig.test.TestHelper;

public class SiteTestHelper {

    private static final String UPLOAD_PARAM = "promptUpload";

    /**
     * Same userName that TestPage creates. thought of referencing static variable but may pull in
     * unnec. dependencies
     */
    public static final String TEST_USER = "testuser";

    public static final String TEST_PAGE_URL = "/TestPage.html";

    /**
     * The name of the checkbox used in standard tables
     */
    public static final String ROW_CHECKBOX = "checkbox";

    private static String s_baseUrl = "http://localhost:12000/sipxconfig";
    
    private static Properties s_testProps;

    static class SipxWebTestSuite extends TestSuite {
        SipxWebTestSuite(Class test) {
            super(test);
        }

        @Override
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

        @Override
        public void run(TestResult result) {
            int e = totalFailures(result);
            super.run(result);
            if (e < totalFailures(result)) {
                WebTester tester = ((WebTestCase) getTest()).getTester();
                if (tester != null) {
                    System.err.println(tester.getPageSource());
                }
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
     * Go to TestPage.html. Log in if the login arg is true. And disables javascript be default
     * too
     */
    public static void home(WebTester tester, boolean login) {
        // default is to disable javascript, re-enable at will
        setScriptingEnabled(tester, false);

        tester.beginAt(TEST_PAGE_URL);

        // So tests can check for fixed text. tests can reset this if they wish
        // back to Locale.getDefault()
        tester.getTestContext().setLocale(Locale.ENGLISH);

        if (login) {
            tester.clickLink("login");
        }
        tester.clickLink("hideNavigation");
    }

    /**
     * Looks for exception stack on tapestry error page. Dumps response if there was an exception.
     */
    public static void assertNoException(WebTester tester) {
        try {
            tester.assertElementNotPresent("exceptionDisplay");
        } catch (AssertionFailedError e) {
            dumpPage(tester);
            throw e;
        }
    }

    /**
     * Works only for pages that use "user:error:text" id to display user errors. All pages with
     * ErrorMsg component belong to this category. It does check for tapestry errors as well: no
     * need to call assertNoException.
     *
     * Handling errors is kind of strange: there is no easy way to get text by id, so we do not
     * bother unless we discover that there is "user:error" in the page.
     */
    public static void assertNoUserError(WebTester tester) {
        assertNoException(tester);
        try {
            tester.assertElementNotPresent("user:error");
        } catch (AssertionFailedError e) {
            String text = tester.getElementTextByXPath("//span[@id='user:error']");
            Assert.fail("User error: <" + text + ">");
        }
    }

    public static void assertUserError(WebTester tester) {
        assertNoException(tester);
        tester.assertElementPresent("user:error");
    }

    /**
     * Returns the row count in a table. Don't forget to include +1 in assert count if you have a
     * table header.
     */
    public static int getRowCount(WebTester tester, String table) {
        tester.assertTablePresent(table);
        return tester.getTable(table).getRowCount();
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

    public static void assertTextFieldEmpty(WebTester tester, String field) {
        tester.assertTextFieldEquals(field, StringUtils.EMPTY);
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

    public static String getBaseUrl() {
        return s_baseUrl;
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

    public static Properties getTestProperties() {
        if (s_testProps == null) {
            s_testProps = (Properties) TestHelper.getTestProperties().clone();
            InputStream propsStream = null;
            try {
                File propsFile = new File(s_testProps.getProperty("SIPX_CONFDIR") + "/sipxconfig.properties");
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
            tester.setWorkingForm("groupForm");
            tester.setTextField("item:name", "seedGroup" + i);
            tester.clickButton("form:ok");
            SiteTestHelper.home(tester);
        }
    }

    /**
     * Initializes upload elements on the form using ad hoc created temporary file.
     *
     * @param form form for which upload fields will be initialized
     * @param fileNamePrefix at least 3 chatracters - use test name
     */
    public static void initUploadFields(WebTester tester, String fileNamePrefix) {
        try {
            File file = File.createTempFile(fileNamePrefix, null);
            tester.setTextField(UPLOAD_PARAM, file.getAbsolutePath());
        } catch (IOException e) {
            throw new RuntimeException(e);
        }
    }

    /**
     * Turn on/off javascript, make sure to restore state to true after you're done
     */
    public static boolean setScriptingEnabled(WebTester tester, boolean enabled) {
        tester.setScriptingEnabled(enabled);
        return false;
    }

    public static String getCellAsText(Table table, int i, int j) {
        ArrayList rows = table.getRows();
        Row row = (Row) rows.get(i);
        Cell cell = (Cell) row.getCells().get(j);
        return cell.getValue();
    }

    public static int getColumnCount(Table table) {
        ArrayList rows = table.getRows();
        Row row = (Row) rows.get(0);
        return row.getCellCount();
    }

    public static void clickSubmitLink(WebTester tester, String linkName) {
        tester.setTextField(FormConstants.SUBMIT_NAME_PARAMETER, linkName);
        tester.submit();
    }

    public static void dumpPage(WebTester webTester) {
        System.err.println(webTester.getPageSource());
    }

    public static void selectOption(WebTester tester, String selectName, String label) {
        tester.selectOption(selectName, label);
        tester.submit();
    }

    public static void selectOptionByValue(WebTester tester, String selectName, String value) {
        tester.selectOptionByValue(selectName, value);
        tester.submit();
    }

    public static void createAdminUserAndAdminGroup(WebTester tester) {
        home(tester);
        tester.clickLink("createAdminUserAndAdminGroup");
    }
}
