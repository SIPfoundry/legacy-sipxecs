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

import org.sipfoundry.sipxconfig.site.SiteTestHelper;

import junit.framework.Test;
import net.sourceforge.jwebunit.junit.WebTestCase;

import static org.sipfoundry.sipxconfig.site.SiteTestHelper.assertNoUserError;
import static org.sipfoundry.sipxconfig.site.SiteTestHelper.getBaseUrl;
import static org.sipfoundry.sipxconfig.site.SiteTestHelper.home;
import static org.sipfoundry.sipxconfig.site.SiteTestHelper.webTestSuite;

public class LdapServerTestUi extends WebTestCase {
    public static Test suite() throws Exception {
        return webTestSuite(LdapServerTestUi.class);
    }

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
        setTextField("port", "3256");
    }
}
