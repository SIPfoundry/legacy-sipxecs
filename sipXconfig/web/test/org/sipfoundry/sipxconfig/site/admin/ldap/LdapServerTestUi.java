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

import junit.framework.Test;
import net.sourceforge.jwebunit.junit.WebTestCase;

import static org.sipfoundry.sipxconfig.site.SiteTestHelper.assertNoUserError;
import static org.sipfoundry.sipxconfig.site.SiteTestHelper.assertUserError;
import static org.sipfoundry.sipxconfig.site.SiteTestHelper.getBaseUrl;
import static org.sipfoundry.sipxconfig.site.SiteTestHelper.home;
import static org.sipfoundry.sipxconfig.site.SiteTestHelper.webTestSuite;

public class LdapServerTestUi extends WebTestCase {
    public static Test suite() throws Exception {
        return webTestSuite(LdapServerTestUi.class);
    }

    protected void setUp() throws Exception {
        getTestContext().setBaseUrl(getBaseUrl());
        home(tester);
    }

    public void testDisplay() {
        clickLink("LdapServer");
        assertNoUserError(tester);

        // Set port to an unlikely ldap port, so that the test passes on systems running a
        // local ldap server.
        assertElementPresent("port");
        setTextField("port", "3256");

        submit("applyConnectionParams");
        assertUserError(tester);
    }
}
