/**
 *
 *
 * Copyright (c) 2010 / 2011 eZuce, Inc. All rights reserved.
 * Contributed to SIPfoundry under a Contributor Agreement
 *
 * This software is free software; you can redistribute it and/or modify it under
 * the terms of the Affero General Public License (AGPL) as published by the
 * Free Software Foundation; either version 3 of the License, or (at your option)
 * any later version.
 *
 * This software is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more
 * details.
 */

package org.sipfoundry.sipxconfig.site.admin.ldap;

import net.sourceforge.jwebunit.api.IElement;
import net.sourceforge.jwebunit.junit.WebTestCase;

import org.sipfoundry.sipxconfig.site.SiteTestHelper;

public class LdapSettingsTestUi extends WebTestCase {

    @Override
    public void setUp() {
        getTestContext().setBaseUrl(SiteTestHelper.getBaseUrl());
    }

    public void testSettings() {
        clickLink("toggleNavigation");
        clickLink("menu.ldap");
        clickLink("link:settingsTarget");
        assertElementPresent("ldap:authenticationOptions");
        assertElementPresent("ldap:configured");
        //test default value
        IElement element = getElementById("ldap:authenticationOptions");
        assertEquals("noLDAP", element.getTextContent());
        assertCheckboxSelected("ldap:configured");
        //test change LDAP option / unconfigured
        uncheckCheckbox("ldap:configured");
        element.setAttribute("ldap:authenticationOptions", "pinLDAP");
        clickButton("ldap:settings:apply");
        assertCheckboxNotSelected("ldap:configured");
        assertEquals("pinLDAP", element.getTextContent());
    }
}
