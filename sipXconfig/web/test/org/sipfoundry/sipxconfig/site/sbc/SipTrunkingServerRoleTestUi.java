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
		uncheckCheckbox("MultiplePropertySelection", "0");
		checkCheckbox("MultiplePropertySelection", "0");
		clickButton("form:ok");
		SiteTestHelper.assertNoUserError(tester);

		clickLink("menu.sbcs");
		SiteTestHelper.assertNoException(tester);
		assertFormPresent();
		assertEquals(2, SiteTestHelper.getRowCount(tester, "list:sbc"));
	}

	public void testDisableSipTrunkingServerRole() {
		uncheckCheckbox("MultiplePropertySelection", "0");
		clickButton("form:ok");
		SiteTestHelper.assertNoUserError(tester);

		clickLink("menu.sbcs");
		SiteTestHelper.assertNoException(tester);
		assertFormPresent();
		assertEquals(1, SiteTestHelper.getRowCount(tester, "list:sbc"));
	}
}
