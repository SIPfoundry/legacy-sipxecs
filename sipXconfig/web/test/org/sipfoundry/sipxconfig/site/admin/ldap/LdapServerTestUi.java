/*
 *
 *
 * Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 *
 */
package org.sipfoundry.sipxconfig.site.admin.ldap;

import static org.sipfoundry.sipxconfig.site.SiteTestHelper.assertNoUserError;
import static org.sipfoundry.sipxconfig.site.SiteTestHelper.getBaseUrl;
import static org.sipfoundry.sipxconfig.site.SiteTestHelper.home;
import net.sourceforge.jwebunit.junit.WebTestCase;

import org.sipfoundry.sipxconfig.site.SiteTestHelper;

public class LdapServerTestUi extends WebTestCase {
    @Override
    public void setUp() {
        getTestContext().setBaseUrl(getBaseUrl());
        SiteTestHelper.setScriptingEnabled(tester, true);
        home(tester);
        clickLink("toggleNavigation");
        clickLink("menu.ldap");
    }

    public void testDisplay() {
        assertNoUserError(tester);

        assertElementPresent("host");
        assertCheckboxPresent("useTls");
        // Set port to an unlikely ldap port, so that the test passes on systems running a
        // local ldap server.
        assertElementPresent("port");
        //test ldap connection error display
        assertElementPresent("applyConnectionParams");
        clickButton("applyConnectionParams");
        SiteTestHelper.assertUserError(getTester());
        assertElementPresent("applyConnectionParams");
    }
}
