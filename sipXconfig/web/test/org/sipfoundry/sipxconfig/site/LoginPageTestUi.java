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

import junit.framework.Test;
import net.sourceforge.jwebunit.WebTestCase;

import com.meterware.httpunit.WebForm;

public class LoginPageTestUi extends WebTestCase {
    public static Test suite() throws Exception {
        return SiteTestHelper.webTestSuite(LoginPageTestUi.class);
    }

    public void setUp() {
        getTestContext().setBaseUrl(SiteTestHelper.getBaseUrl());
        tester.beginAt(SiteTestHelper.TEST_PAGE_URL);
        clickLink("seedTestUser");
        clickLink("ManageUsers");   // will redirect to login page
    }    
    
    public void testLoginWithUserName() throws Exception {       
        checkLogin(TestPage.TEST_USER_USERNAME);
    }
    
    public void testLoginWithFirstAlias() throws Exception {       
        checkLogin(TestPage.TEST_USER_ALIAS1);
    }
    
    public void testLoginWithSecondAlias() throws Exception {       
        checkLogin(TestPage.TEST_USER_ALIAS2);
    }
    
    private void checkLogin(String userId) {        
        tester.beginAt("/");        
        SiteTestHelper.assertNoException(getTester());
        SiteTestHelper.assertNoUserError(getTester());
        
        WebForm form = tester.getDialog().getForm();
        form.setParameter("userName", userId);
        form.setParameter("password", TestPage.TEST_USER_PIN);
        clickButton("login:submit");
                
        // we are on the home page now - no errors no login form
        SiteTestHelper.assertNoException(getTester());
        assertElementNotPresent("user:error");        
        assertFormNotPresent("login:form");
    }
    
    // successful login is tested by "home" function
    public void testLoginFailed() throws Exception {
        tester.beginAt("/");
        
        SiteTestHelper.assertNoException(getTester());
        SiteTestHelper.assertNoUserError(getTester());
        
        WebForm form = tester.getDialog().getForm();
        form.setParameter("userName", "xyz");
        form.setParameter("password", "abc");
        clickButton("login:submit");
        
        // still on the same page
        SiteTestHelper.assertNoException(getTester());
        assertElementPresent("login:form");
        assertElementPresent("user:error");
    }
    
    public void testLoginBlankPassword() throws Exception {
        WebForm form = tester.getDialog().getForm();
        form.setParameter("userName", TestPage.TEST_USER_USERNAME);
        clickButton("login:submit");        
        assertElementPresent("user:error");
    }
}
