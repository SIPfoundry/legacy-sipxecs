/*
 * 
 * 
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 * 
 * $
 */
package org.sipfoundry.sipxconfig.site.line;

import junit.framework.Test;
import net.sourceforge.jwebunit.WebTestCase;

import org.sipfoundry.sipxconfig.site.SiteTestHelper;
import org.sipfoundry.sipxconfig.site.phone.PhoneTestHelper;

public class AddExternalLineTestUi extends WebTestCase {
    
    private PhoneTestHelper m_helper;

    public static Test suite() throws Exception {
        return SiteTestHelper.webTestSuite(AddExternalLineTestUi.class);
    }
    
    protected void setUp() throws Exception {
        super.setUp();
        getTestContext().setBaseUrl(SiteTestHelper.getBaseUrl());        
        m_helper = new PhoneTestHelper(tester);
        m_helper.reset();
        SiteTestHelper.seedUser(tester);
        m_helper.seedPhone(1);
        clickLink("ManagePhones");
        clickLinkWithText(m_helper.endpoint[0].getSerialNumber());
        clickLinkWithText("Lines");
    }

    protected void tearDown() throws Exception {
        super.tearDown();
    }

    public void testDisplay() {
        clickLink("AddExternalLine");        
        SiteTestHelper.assertNoException(tester);
        assertElementPresent("form:externalLine");
    }
    
    public void testAddExternalLine() {
        clickLink("AddExternalLine");        
        setFormElement("displayName", "Dil Bert");
        setFormElement("userId", "dilbert");
        setFormElement("password", "1234");
        setFormElement("confirmPassword", "1234");
        setFormElement("registrationServer", "frakenberry.org");
        setFormElement("voiceMail", "2000");
        clickButton("form:ok");
        SiteTestHelper.assertNoException(tester);
        SiteTestHelper.assertNoUserError(tester);
        assertTablePresent("line:list");
        assertEquals(SiteTestHelper.getRowCount(tester, "line:list"), 2); // 1 plus header
        assertTextPresent("\"Dil Bert\"&lt;sip:dilbert@frakenberry.org&gt;");
    }
    
    public void testAddExternalLineWithError() {
        assertEquals(SiteTestHelper.getRowCount(tester, "line:list"), 1); // header

        clickLink("AddExternalLine");        
        
        // empty
        clickButton("form:ok");
        SiteTestHelper.assertUserError(tester);

        setFormElement("userId", "dilbert");
        clickButton("form:ok");
        SiteTestHelper.assertUserError(tester);

        setFormElement("password", "1");
        setFormElement("confirmPassword", "1");
        setFormElement("registrationServer", "fwd.org");
        clickButton("form:ok");
        SiteTestHelper.assertNoUserError(tester);
        
        assertTablePresent("line:list");
        assertEquals(SiteTestHelper.getRowCount(tester, "line:list"), 2); // 1 plus header
        
    }
}
