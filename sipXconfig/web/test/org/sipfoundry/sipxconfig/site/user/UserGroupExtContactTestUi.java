/*
 *
 *
 * Copyright (C) 2010 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.site.user;

import static org.sipfoundry.sipxconfig.site.SiteTestHelper.getBaseUrl;
import junit.framework.Test;
import net.sourceforge.jwebunit.junit.WebTestCase;

import org.sipfoundry.sipxconfig.site.SiteTestHelper;
import org.sipfoundry.sipxconfig.site.TestPage;

public class UserGroupExtContactTestUi extends WebTestCase {

    public static Test suite() throws Exception {
        return SiteTestHelper.webTestSuite(UserGroupExtContactTestUi.class);
    }

    @Override
    protected void setUp() {
        getTestContext().setBaseUrl(getBaseUrl());
        SiteTestHelper.home(tester);
        SiteTestHelper.setScriptingEnabled(tester, true);
        clickLink("resetCoreContext");
    }

    /**
     * Tests to make sure the external contact creation UI requires a prefix
     * to be selected when external contact creation is enabled, and does not
     * require a prefix when creation is disabled.
     */
    public void testValidation() {
        SiteTestHelper.seedGroup(getTester(), "NewUserGroup", 1);
        clickLink("UserGroups");
        clickLinkWithText("seedGroup0");
        clickLink("link:extcontact");
        checkCheckbox("extcontacts:enable");
        setTextField("extcontacts:prefix", "");
        submit("submit:apply");
        SiteTestHelper.assertUserError(tester);

        setTextField("extcontacts:prefix", "prefix");
        submit("submit:apply");
        SiteTestHelper.assertNoUserError(tester);

        setTextField("extcontacts:prefix", "");
        uncheckCheckbox("extcontacts:enable");
        submit("submit:apply");
        SiteTestHelper.assertNoUserError(tester);
    }

}
