package org.sipfoundry.sipxconfig.site.nattraversal;

import junit.framework.Test;
import org.sipfoundry.sipxconfig.site.SiteTestHelper;
import net.sourceforge.jwebunit.junit.WebTestCase;

public class NatTraversalTestUi extends WebTestCase {
    public static Test suite() throws Exception {
        return SiteTestHelper.webTestSuite(NatTraversalTestUi.class);
    }

    public NatTraversalTestUi() {
    }

    public void setUp() {
        getTestContext().setBaseUrl(SiteTestHelper.getBaseUrl());
        SiteTestHelper.home(getTester());
        clickLink("resetInternetCalling");
        clickLink("NatTraversal");
    }

    public void testMaxConcurentRelaysError() {
        SiteTestHelper.assertNoException(tester);
        assertButtonPresent("form:apply");
        setTextField("setting:max-concurrent-relays", "-1");
        clickButton("form:apply");
        SiteTestHelper.assertUserError(tester);
    }

    public void testApplySettings() {
        SiteTestHelper.assertNoException(tester);
        assertButtonPresent("form:apply");
        checkCheckbox("enabled");
        selectOption("setting:aggressiveness","Conservative");
        setTextField("setting:max-concurrent-relays", "20");
        clickButton("form:apply");
        SiteTestHelper.assertNoUserError(tester);
        clickButton("cancel");
        tester.assertTextFieldEquals("setting:max-concurrent-relays", "20");
        tester.assertCheckboxSelected("enabled");
        tester.assertSelectedOptionEquals("setting:aggressiveness", "Conservative");

    }
}
