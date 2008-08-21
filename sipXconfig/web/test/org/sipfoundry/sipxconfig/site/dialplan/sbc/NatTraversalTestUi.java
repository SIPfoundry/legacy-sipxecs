package org.sipfoundry.sipxconfig.site.dialplan.sbc;

import junit.framework.Test;
import net.sourceforge.jwebunit.junit.WebTestCase;

import org.sipfoundry.sipxconfig.site.SiteTestHelper;

public class NatTraversalTestUi extends WebTestCase {
    public static Test suite() throws Exception {
        return SiteTestHelper.webTestSuite(NatTraversalTestUi.class);
    }

    @Override
    public void setUp() {
        getTestContext().setBaseUrl(SiteTestHelper.getBaseUrl());
        SiteTestHelper.home(getTester());
        clickLink("initNatTraversal");
        clickLink("InternetCalling");
        clickLink("link:internetCalling");
        SiteTestHelper.selectOption(tester, "common_FlexiblePropertySelection", "bridge");
        clickButton("form:apply");
        SiteTestHelper.assertNoUserError(tester);
        clickButton("cancel");
        clickLink("link:natTraversal");
    }

    public void testMaxConcurentRelaysError() {
        SiteTestHelper.assertNoException(tester);
        assertButtonPresent("form:apply");
        setTextField("setting:concurrentrelays", "-1");
        clickButton("form:apply");
        SiteTestHelper.assertUserError(tester);
    }

    public void testRtpRangeErrors() {
        SiteTestHelper.assertNoException(tester);
        assertButtonPresent("form:apply");
        assertLinkPresent("setting:toggle");
        SiteTestHelper.clickSubmitLink(tester, "setting:toggle");
        SiteTestHelper.assertNoException(tester);
        //negative values
        setTextField("setting:port-range-start", "-1");
        clickButton("form:apply");
        SiteTestHelper.assertUserError(tester);
        //start higher than end
        setTextField("setting:port-range-start", "20000");
        setTextField("setting:port-range-end", "19000");
        clickButton("form:apply");
        SiteTestHelper.assertUserError(tester);
    }

    public void testApplySettings() {
        SiteTestHelper.assertNoException(tester);
        assertButtonPresent("form:apply");
        assertLinkPresent("setting:toggle");
        SiteTestHelper.clickSubmitLink(tester, "setting:toggle");
        checkCheckbox("enabled");
        selectOption("setting:relayaggressiveness","Conservative");
        setTextField("setting:concurrentrelays", "20");
        //relay setting:
        setTextField("setting:mediarelayexternaladdress","11.12.13.15");
        clickButton("form:apply");
        SiteTestHelper.assertNoUserError(tester);
        clickButton("cancel");
        tester.assertTextFieldEquals("setting:concurrentrelays", "20");
        //relay setting:
        tester.assertTextFieldEquals("setting:mediarelayexternaladdress","11.12.13.15");
        tester.assertCheckboxSelected("enabled");
        tester.assertSelectedOptionEquals("setting:relayaggressiveness", "Conservative");

    }
}
