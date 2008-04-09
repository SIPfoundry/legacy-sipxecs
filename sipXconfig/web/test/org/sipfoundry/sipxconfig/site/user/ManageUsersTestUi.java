/*
 * 
 * 
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 * 
 * $
 */
package org.sipfoundry.sipxconfig.site.user;

import junit.framework.Test;
import net.sourceforge.jwebunit.junit.WebTestCase;

import org.sipfoundry.sipxconfig.site.SiteTestHelper;
import org.sipfoundry.sipxconfig.site.phone.PhoneTestHelper;


public class ManageUsersTestUi extends WebTestCase {

    private PhoneTestHelper m_helper;
    
    public static Test suite() throws Exception {
        return SiteTestHelper.webTestSuite(ManageUsersTestUi.class);
    }

    protected void setUp() throws Exception {
        super.setUp();
        getTestContext().setBaseUrl(SiteTestHelper.getBaseUrl());        
        m_helper = new PhoneTestHelper(tester);
        m_helper.reset();
    }
    
    public void testDisplay() throws Exception {
        clickLink("ManageUsers");              
        SiteTestHelper.assertNoException(tester);        
    }
    
    public void testAddUser() throws Exception {
        SiteTestHelper.seedUser(tester);
        clickLink("ManageUsers");
        clickLink("AddUser");
        clickButton("form:cancel");
        SiteTestHelper.assertNoException(tester);
    }
    
    public void testGroupFilter() throws Exception {
        SiteTestHelper.seedUser(tester);
        SiteTestHelper.seedGroup(tester, "NewUserGroup", 1);
        clickLink("ManageUsers");
        
        // all users
        int allTableCount = SiteTestHelper.getRowCount(tester, "user:list");
        
        // empty group, no users
        selectOption("groupFilter", "seedGroup0");
        SiteTestHelper.submitNoButton(tester);
        SiteTestHelper.assertNoException(tester);
        int emptyTableCount = SiteTestHelper.getRowCount(tester, "user:list");
        assertTrue(allTableCount > emptyTableCount);

        // back to all users
        selectOption("groupFilter", "- all -");
        SiteTestHelper.submitNoButton(tester);
        int allTableCountAgain = SiteTestHelper.getRowCount(tester, "user:list");
        assertEquals(allTableCount, allTableCountAgain);
    }
    
}
