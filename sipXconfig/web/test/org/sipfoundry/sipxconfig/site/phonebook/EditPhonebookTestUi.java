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
import net.sourceforge.jwebunit.WebTestCase;

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
        SiteTestHelper.initUploadFields(getDialog().getForm(), "EditPhonebook");
    }

    public void testApplyOkOnNew() {
        SiteTestHelper.home(getTester());
        clickLink("link:managePhonebooks");
        clickLink("addPhonebook");
        SiteTestHelper.initUploadFields(getDialog().getForm(), "EditPhonebook");
        setFormElement("name", "test-phonebook");
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
        setFormElement("name", "test-phonebook");
        clickButton("form:apply");
        SiteTestHelper.assertNoException(tester);
        SiteTestHelper.assertNoUserError(tester);
    }

    public void testFormError() {
        clickButton("form:apply");
        SiteTestHelper.assertNoException(tester);
        SiteTestHelper.assertUserError(tester);
    }
}
