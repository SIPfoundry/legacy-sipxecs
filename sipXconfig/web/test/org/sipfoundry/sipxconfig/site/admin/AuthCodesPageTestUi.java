/*
 *
 *
 * Copyright (C) 2010 Avaya, certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 *
 */
package org.sipfoundry.sipxconfig.site.admin;

import junit.framework.Test;
import net.sourceforge.jwebunit.junit.WebTestCase;

import org.sipfoundry.sipxconfig.site.SiteTestHelper;

public class AuthCodesPageTestUi extends WebTestCase {

    public static Test suite() throws Exception {
        return SiteTestHelper.webTestSuite(AuthCodesPageTestUi.class);
    }

    @Override
    public void setUp() {
        getTestContext().setBaseUrl(SiteTestHelper.getBaseUrl());
        SiteTestHelper.home(getTester());
        clickLink("toggleNavigation");
        clickLink("menu.authcodes");
        SiteTestHelper.assertNoException(getTester());
    }
    public void testAuthCodesPageUi() throws Exception {
        SiteTestHelper.assertNoException(getTester());
        assertElementPresent("setting:SIP_AUTH_CODE_PREFIX");
        assertElementPresent("setting:SIP_AUTH_CODE_ALIASES");
        assertElementPresent("form:apply");

        assertLinkPresent("code:add");
        assertElementPresent("authcode:list");
        assertElementPresent("authCodes:delete");
    }

    public void testAuthCodeDeleteUi() throws Exception {
        SiteTestHelper.assertNoException(getTester());
        assertLinkPresent("code:add");

        // delete authcode
        setWorkingForm("authCodeTableForm");
        int rowCount = SiteTestHelper.getRowCount(tester, "authcode:list");
        for (int i = 0; i < rowCount - 2; i++) {
            SiteTestHelper.selectRow(tester, i, true);
        }
        clickButton("authCodes:delete");
        assertEquals(2, SiteTestHelper.getRowCount(tester, "authcode:list"));
    }

    public void testAccCodeServiceUi() throws Exception {
        SiteTestHelper.assertNoException(getTester());

        assertElementPresent("setting:SIP_AUTH_CODE_ALIASES");

        //some random unused alias
        setTextField("setting:SIP_AUTH_CODE_ALIASES","23952513");
        clickButton("form:apply");
        SiteTestHelper.assertNoUserError(getTester());

        //remove alias
        setTextField("setting:SIP_AUTH_CODE_ALIASES","");
        clickButton("form:apply");
        SiteTestHelper.assertNoUserError(getTester());
    }

    public void testAuthCodeEditUi() throws Exception {
        SiteTestHelper.assertNoException(getTester());
        assertLinkPresent("code:add");


        // check create auth code page
        clickLink("code:add");
        assertElementPresent("code:name");
        assertElementPresent("code:description");

        assertCheckboxPresent("setting:900Dialing");
        assertCheckboxPresent("setting:InternationalDialing");
        assertCheckboxPresent("setting:LocalDialing");
        assertCheckboxPresent("setting:LongDistanceDialing");
        assertCheckboxPresent("setting:Mobile");
        assertCheckboxPresent("setting:TollFree");

        // have to provide a Auth Code
        clickButton("form:ok");
        SiteTestHelper.assertUserError(getTester());

        // fill in the name and proceed with AuthCode creation
        setTextField("code:name","123456");
        checkCheckbox("setting:900Dialing");
        clickButton("form:ok");
        SiteTestHelper.assertNoUserError(getTester());

        // check if back in main page
        assertLinkPresentWithText("123456");

        // edit 123456
        clickLinkWithText("123456");

        //change the name
        setTextField("code:name","654321");

        clickButton("form:ok");

        assertLinkPresentWithText("654321");

        // create auth code with same name
        clickLink("code:add");

        setTextField("code:name","654321");
        clickButton("form:ok");
        SiteTestHelper.assertUserError(getTester());

        clickButton("form:cancel");

        // check if back in main page
        assertElementPresent("code:add");

        // delete authcode
        setWorkingForm("authCodeTableForm");
        int rowCount = SiteTestHelper.getRowCount(tester, "authcode:list");
        for (int i = 0; i < rowCount - 2; i++) {
            SiteTestHelper.selectRow(tester, i, true);
        }
        clickButton("authCodes:delete");
        assertEquals(2, SiteTestHelper.getRowCount(tester, "authcode:list"));
    }

}
