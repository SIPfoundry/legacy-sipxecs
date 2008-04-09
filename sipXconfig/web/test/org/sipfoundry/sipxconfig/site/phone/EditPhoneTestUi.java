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
import net.sourceforge.jwebunit.junit.WebTestCase;

import org.sipfoundry.sipxconfig.site.SiteTestHelper;


public class EditPhoneTestUi extends WebTestCase {

    private PhoneTestHelper m_helper;
        
    public static Test suite() throws Exception {
        return SiteTestHelper.webTestSuite(EditPhoneTestUi.class);
    }
    
    protected void setUp() throws Exception {
        super.setUp();
        getTestContext().setBaseUrl(SiteTestHelper.getBaseUrl());        
        m_helper = new PhoneTestHelper(tester);
        m_helper.reset();
        m_helper.seedPhone(1);
        clickLink("ManagePhones");        
        clickLinkWithText(m_helper.endpoint[0].getSerialNumber());
        setTextField("phone:serialNumber", "a00000000000");
    }

    public void testEditPhone() {
        setTextField("phone:serialNumber", "a00000000001");
        clickButton("form:ok");
        String[][] table = new String[][] {
            { "a00000000001", "", "Acme" },                
        };
        assertTextInTable("phone:list", table[0]);        
    }

    public void testAddLine() {
        clickLink("AddLine");
        assertElementPresent("user:list");
    }
    
    public void testGenerateProfiles() {
        clickButton("generateProfile");
        // check for confirm screen
        assertCheckboxSelected("restart:checkbox");
        clickButton("generate:ok");
        assertElementPresent("user:success");
    }
}
