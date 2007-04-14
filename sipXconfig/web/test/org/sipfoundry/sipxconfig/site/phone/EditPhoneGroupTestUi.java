/*
 * 
 * 
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 * 
 * $
 */
package org.sipfoundry.sipxconfig.site.phone;

import junit.framework.Test;
import net.sourceforge.jwebunit.WebTestCase;

import org.sipfoundry.sipxconfig.site.SiteTestHelper;

public class EditPhoneGroupTestUi extends WebTestCase {

    PhoneTestHelper tester;

    public static Test suite() throws Exception {
        return SiteTestHelper.webTestSuite(EditPhoneGroupTestUi.class);
    }

    protected void setUp() throws Exception {
        getTestContext().setBaseUrl(SiteTestHelper.getBaseUrl());
        tester = new PhoneTestHelper(getTester());
    }

    public void testNewGroup() {
        tester.reset();
        clickLink("NewPhoneGroup");
        setFormElement("name", "editPhoneGroupTestUi");
        setFormElement("description", "test description text");
        clickButton("form:ok");
        SiteTestHelper.assertNoException(getTester());
        clickLink("PhoneGroups");
        String[][] table = new String[][] {
            {
                "editPhoneGroupTestUi"
            },
        };
        assertTextInTable("group:list", table[0]);
    }

    public void testDuplicateNameUserError() {
        tester.reset();
        tester.seedGroup(1);
        clickLink("NewPhoneGroup");
        setFormElement("name", "seedGroup0");
        clickButton("form:ok");
        assertTextPresent("A group with name: seedGroup0 already exists");
    }
    
    public void testWhitespaceIllegal() {
        tester.reset();
        clickLink("NewPhoneGroup");
        setFormElement("name", "seed group");
        clickButton("form:ok");
        assertTextPresent("Cannot contain spaces");        
    }
}
