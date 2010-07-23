/*
 *
 *
 *
 * Copyright (c) 2010 eZuce, Inc. All rights reserved.
 *
 * This library is free software; you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License as published by the Free
 * Software Foundation; either version 2.1 of the License, or (at your option)
 * any later version.
 *
 * This library is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more
 * details.
 */

package org.sipfoundry.sipxconfig.site.admin.ldap;

import junit.framework.Test;
import org.sipfoundry.sipxconfig.site.SiteTestHelper;
import org.sipfoundry.sipxconfig.site.admin.AboutDialogTestUi;

import net.sourceforge.jwebunit.api.IElement;
import net.sourceforge.jwebunit.junit.WebTestCase;

public class LdapSettingsTestUi extends WebTestCase {

    public static Test suite() throws Exception {
        return SiteTestHelper.webTestSuite(AboutDialogTestUi.class);
    }

    @Override
    public void setUp() {
        getTestContext().setBaseUrl(SiteTestHelper.getBaseUrl());
    }

    public void testSettings() {
        clickLink("toggleNavigation");
        clickLink("menu.ldap");
        clickLink("link:settingsTarget");
        assertElementPresent("ldap:authenticationOptions");
        //test default value
        IElement element = getElementById("ldap:authenticationOptions");
        assertEquals("noLDAP", element.getTextContent());
        //test change LDAP option
        element.setAttribute("ldap:authenticationOptions", "pinLDAP");
        clickButton("ldap:settings:apply");
        assertEquals("pinLDAP", element.getTextContent());
    }
}
