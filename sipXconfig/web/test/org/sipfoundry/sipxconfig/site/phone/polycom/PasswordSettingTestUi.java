/*
 * 
 * 
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 * 
 * $
 */
package org.sipfoundry.sipxconfig.site.phone.polycom;

import junit.framework.Test;
import net.sourceforge.jwebunit.WebTestCase;

import org.sipfoundry.sipxconfig.site.SiteTestHelper;
import org.sipfoundry.sipxconfig.site.phone.PhoneTestHelper;
import org.w3c.dom.Element;

public class PasswordSettingTestUi extends WebTestCase {
 
    private PhoneTestHelper m_helper;
    
    public static Test suite() throws Exception {
        return SiteTestHelper.webTestSuite(PasswordSettingTestUi.class);
    }
    
    protected void setUp() throws Exception {
        super.setUp();
        getTestContext().setBaseUrl(SiteTestHelper.getBaseUrl());        
        m_helper = new PhoneTestHelper(tester);
        m_helper.reset();
    }

    protected void tearDown() throws Exception {
        super.tearDown();
    }
    
    public void testEditSipSetttings() {
        m_helper.seedLine(1);
        SiteTestHelper.setScriptingEnabled(true);
        clickLink("ManagePhones");        
        clickLinkWithText(SiteTestHelper.TEST_USER);
        clickLinkWithText("Credentials");
        clickLink("setting:toggle");
        Element passwordField = getDialog().getElement("setting:password");    
tester.dumpResponse();
        assertEquals("password", passwordField.getAttribute("type"));
    }
}
