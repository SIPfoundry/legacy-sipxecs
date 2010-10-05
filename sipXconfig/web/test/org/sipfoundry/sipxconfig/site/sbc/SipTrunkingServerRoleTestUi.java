/*
 * Copyright (C) 2010 Avaya, certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 */
package org.sipfoundry.sipxconfig.site.sbc;

import junit.framework.Test;
import net.sourceforge.jwebunit.junit.WebTestCase;

import org.sipfoundry.sipxconfig.site.SiteTestHelper;

public class SipTrunkingServerRoleTestUi extends WebTestCase {
	public static Test suite() throws Exception {
		return SiteTestHelper.webTestSuite(SipTrunkingServerRoleTestUi.class);
	}

	public void setUp() {
		getTestContext().setBaseUrl(SiteTestHelper.getBaseUrl());
		SiteTestHelper.home(getTester());

		// create one and only one location (primary)
		clickLink("seedLocationsManager");
		clickLink("toggleNavigation");
		clickLink("menu.locations");
		SiteTestHelper.assertNoUserError(tester);

		clickLink("editLocationLink");
		clickLink("link:configureBundle");
	}

	public void testEnableSipTrunkingServerRole() {
		uncheckCheckbox("MultiplePropertySelection", "1");
		checkCheckbox("MultiplePropertySelection", "1");
		clickButton("form:ok");
		SiteTestHelper.assertNoUserError(tester);
	}

	public void testDisableSipTrunkingServerRole() {
		uncheckCheckbox("MultiplePropertySelection", "1");
		clickButton("form:ok");
		SiteTestHelper.assertNoUserError(tester);
	}
}
