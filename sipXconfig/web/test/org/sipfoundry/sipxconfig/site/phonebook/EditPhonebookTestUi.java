/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.site.phonebook;

import junit.framework.Test;
import net.sourceforge.jwebunit.junit.WebTestCase;

import org.sipfoundry.sipxconfig.site.SiteTestHelper;

public class EditPhonebookTestUi extends WebTestCase {

    public static Test suite() throws Exception {
        return SiteTestHelper.webTestSuite(EditPhonebookTestUi.class);
    }

    protected void setUp() throws Exception {
        super.setUp();
        getTestContext().setBaseUrl(SiteTestHelper.getBaseUrl());
        SiteTestHelper.home(getTester());
        clickLink("link:phonebookReset");
        clickLink("link:phonebook");
        setWorkingForm("phonebookForm");
    }

    public void testApplyOkOnNew() {
        SiteTestHelper.home(getTester());
        clickLink("link:managePhonebooks");
        clickLink("addPhonebook");
        setWorkingForm("phonebookForm");
        setTextField("item:name", "test-phonebook");
        clickButton("form:apply");
        SiteTestHelper.assertNoUserError(tester);
        clickButton("form:ok");
        SiteTestHelper.assertNoUserError(tester);
    }

    public void testDisplay() {
        SiteTestHelper.assertNoException(tester);
        assertElementPresent("phonebookForm");
    }

    public void testNewPhonebook() {
        setTextField("item:name", "test-phonebook");
        clickButton("form:apply");
        SiteTestHelper.assertNoUserError(tester);
    }

    public void testFormError() {
        clickButton("form:apply");
        SiteTestHelper.assertUserError(tester);
    }

    public void testImportGmailContacts() {
        assertElementPresent("gmailAddress");
        assertElementPresent("gmailPassword");
        setTextField("item:name", "test-phonebook");
        setTextField("gmailAddress", "test");
        clickButton("form:apply");
        SiteTestHelper.assertUserError(tester);
    }
}
