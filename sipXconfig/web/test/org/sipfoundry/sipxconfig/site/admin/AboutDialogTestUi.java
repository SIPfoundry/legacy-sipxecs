/*
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
package org.sipfoundry.sipxconfig.site.admin;

import junit.framework.Test;

import net.sourceforge.jwebunit.junit.WebTestCase;

import org.sipfoundry.sipxconfig.site.SiteTestHelper;

public class AboutDialogTestUi extends WebTestCase {
    public static Test suite() throws Exception {
        return SiteTestHelper.webTestSuite(AboutDialogTestUi.class);
    }

    @Override
    public void setUp() {
        getTestContext().setBaseUrl(SiteTestHelper.getBaseUrl());
        SiteTestHelper.setScriptingEnabled(tester, true);
    }

    private void clickAboutDialog() {
        SiteTestHelper.home(tester);
        SiteTestHelper.setScriptingEnabled(tester, true);
        clickLink("toggleNavigation");
        clickLink("about");
    }

    public void testShowHideDialog() {
	clickAboutDialog();
	assertElementPresent("aboutDialog");
	assertElementPresent("close");
	clickButton("close");
	//the dialog does not dissapear immediately (some sort of race condition
	//and I cannot test: assertElementNotPresent("aboutDialog");
    }
}
