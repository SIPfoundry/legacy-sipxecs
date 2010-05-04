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

import java.io.IOException;

import junit.framework.Test;
import net.sourceforge.jwebunit.junit.WebTestCase;
import org.sipfoundry.sipxconfig.site.SiteTestHelper;

public class LdapImportPageTestUi extends WebTestCase {
    public static Test suite() throws Exception {
        return SiteTestHelper.webTestSuite(LdapImportPageTestUi.class);
    }

    public void testImportLdapPage() throws Exception {
        getTestContext().setBaseUrl(SiteTestHelper.getBaseUrl());
        SiteTestHelper.setScriptingEnabled(tester, true);
        SiteTestHelper.home(tester);
        clickLink("toggleNavigation");
        clickLink("menu.importLdap");
        SiteTestHelper.assertNoException(tester);
        assertButtonPresent("import:verify");
        assertButtonPresent("import:ok");
        assertCheckboxPresent("schedule.enabled");
        assertElementPresent("type");
        assertSelectOptionPresent("type", "Every Week");
        assertSelectOptionPresent("type", "Every Day");
        assertSelectOptionPresent("type", "Every Hour");
    }

}
