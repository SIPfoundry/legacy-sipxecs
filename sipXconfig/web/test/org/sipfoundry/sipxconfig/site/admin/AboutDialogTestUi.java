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
