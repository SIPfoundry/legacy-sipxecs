/*
 * 
 * 
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 * 
 * $
 */
package org.sipfoundry.sipxconfig.site.vm;


import java.text.DateFormat;
import java.util.Locale;

import junit.framework.Test;
import net.sourceforge.jwebunit.junit.WebTestCase;

import org.sipfoundry.sipxconfig.components.TapestryUtils;
import org.sipfoundry.sipxconfig.site.SiteTestHelper;
import org.sipfoundry.sipxconfig.site.TestPage;
import org.sipfoundry.sipxconfig.test.TestUtil;


public class ManageVoicemailTestUi extends WebTestCase {
    
    public static Test suite() throws Exception {
        return SiteTestHelper.webTestSuite(ManageVoicemailTestUi.class);
    }
    
    protected void setUp() throws Exception {
        super.setUp();
        getTestContext().setBaseUrl(SiteTestHelper.getBaseUrl());
        SiteTestHelper.home(getTester());
    }
    
    private void gotoManageVoicemail() {
        SiteTestHelper.home(getTester());    
        SiteTestHelper.setScriptingEnabled(tester, true);
        clickLink("seedLocationsManager");
        clickLink("resetVoicemail");              
        clickLink("loginFirstTestUser");              
        clickLink("ManageVoicemail");                      
    }
    
    public void testNoPermissionNotice() {
        SiteTestHelper.home(getTester());
        clickLink("DisableVoicemailPermission");
        clickLink("ManageVoicemail");
        assertElementPresent("noVoicemailPermission");
    }
    
    public void testDisplay() throws Exception {
        gotoManageVoicemail();
        SiteTestHelper.assertNoException(tester);        
        SiteTestHelper.assertNoUserError(tester);        
        assertElementPresent("voicemail:list");
    }    
    
    public void testNavigation() throws Exception {
        gotoManageVoicemail();
        assertTextPresent("Voice Message 00000002");
        clickLink("link:deleted");
        assertTextPresent("Voice Message 00000001");
    }
    
    public void testMove() throws Exception {
        gotoManageVoicemail();
        assertTextPresent("Voice Message 00000002");
        checkCheckbox("checkbox");
        actionSelection(MoveVoicemailAction.class, "deleted");
        assertTextNotPresent("Voice Message 00000002");
        clickLink("link:deleted");
        assertTextPresent("Voice Message 00000001");
        assertTextPresent("Voice Message 00000002");
    }
    
    private void actionSelection(Class groupAction, String action) {
        SiteTestHelper.selectOptionByValue(tester, "actionSelection", groupAction.getName() + action);        
    }
    
    public void testEdit() throws Exception {
        gotoManageVoicemail();
        clickLinkWithText("Voice Message 00000002");
        assertElementPresent("voicemail:edit");
    }   
    
    public void testPageServiceShouldntRedirect() throws Exception {
        SiteTestHelper.home(getTester());    
        clickLink("seedTestUser");
        clickLink("Logout");
        clickLink("ManageVoicemail");
        login(TestPage.TEST_USER_USERNAME, TestPage.TEST_USER_PIN);
        assertElementNotPresent("voicemail:edit");
    }

    public void testExternalServiceShouldRedirect() throws Exception {
        SiteTestHelper.home(getTester());    
        clickLink("seedTestUser");
        clickLink("Logout");        
        gotoPage(String.format("mailbox/%s/inbox", TestPage.TEST_USER_USERNAME));
        login(TestPage.TEST_USER_USERNAME, TestPage.TEST_USER_PIN);
        assertElementPresent("voicemail:list");
    }

    public void testExternalServiceShouldRedirectSecondPasswordAttempt() throws Exception {
        SiteTestHelper.home(getTester());    
        clickLink("seedTestUser");
        clickLink("Logout");
        gotoPage(String.format("mailbox/%s/inbox", TestPage.TEST_USER_USERNAME));
        login(TestPage.TEST_USER_USERNAME, "Bogus");
        login(TestPage.TEST_USER_USERNAME, TestPage.TEST_USER_PIN);
        assertElementPresent("voicemail:list");
    }
    
    public void testForwardedMessages() throws Exception {
        DateFormat fmt = TapestryUtils.getDateFormat(Locale.getDefault());
        gotoManageVoicemail();
        String[][] forwardedMessage = {{
            "unchecked",
            "Fwd:Voice Message 00000014 Voice Message 00000014", 
            "200 Yellowthroat Warbler - 200", 
            fmt.format(TestUtil.localizeDateTime("2/9/07 6:03:00 PM EST")) + " " 
              + fmt.format(TestUtil.localizeDateTime("2/9/07 3:40:00 PM EST")), 
            "6 seconds",
            ""
        }};

        assertTableRowsEqual("voicemail:list", 3, forwardedMessage);
        checkCheckbox("checkbox_0");
        actionSelection(MoveVoicemailAction.class, "saved");
        clickLink("link:saved");
        
        assertTableRowsEqual("voicemail:list", 2, forwardedMessage);
    }
    
    public void testPlayFriendlyUrl() throws Exception {
        gotoManageVoicemail();
        gotoPage(String.format("mailbox/%s/inbox/00000002", TestPage.TEST_USER_USERNAME));
// FIXME: how to check content type?        
//        assertMatchInElement(elementID, regexp) Equals("audio/x-wav", getServeurResponse().getContentType());                
    }
    
    public void testDeleteFriendlyUrl() throws Exception {
        gotoManageVoicemail();
        assertTextPresent("00000002");
        gotoPage(String.format("mailbox/%s/inbox/00000002/delete", TestPage.TEST_USER_USERNAME));
        assertTextNotPresent("00000002");
    }

    
    private void login(String username, String password) {
        assertElementPresent("loginForm");
        setTextField("j_username", username);
        setTextField("j_password", password);
        clickButton("login:submit");        
    }
}


