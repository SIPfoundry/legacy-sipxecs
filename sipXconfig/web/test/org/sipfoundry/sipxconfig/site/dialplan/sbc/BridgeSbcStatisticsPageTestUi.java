/*
 * Copyright (C) 2010 Avaya, certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 */
package org.sipfoundry.sipxconfig.site.dialplan.sbc;

import junit.framework.Test;
import net.sourceforge.jwebunit.junit.WebTestCase;

import org.sipfoundry.sipxconfig.site.SiteTestHelper;

public class BridgeSbcStatisticsPageTestUi extends WebTestCase{

    public static Test suite() throws Exception {
        return SiteTestHelper.webTestSuite(BridgeSbcStatisticsPageTestUi.class);
    }

    @Override
    public void setUp() {
        getTestContext().setBaseUrl(SiteTestHelper.getBaseUrl());
        SiteTestHelper.home(tester);
        clickLink("link:seedBridgeSbc");
        SiteTestHelper.home(tester);
        clickLink("InternalSbcStats");
    }

    public void testRefresh() {
        SiteTestHelper.assertNoException(tester);
        setWorkingForm("intervalForm");
        assertButtonPresent("refresh");
        assertElementPresent("sbc:stats:list");
        clickButton("refresh");
        SiteTestHelper.assertNoException(tester);
        setWorkingForm("intervalForm");
        assertButtonPresent("refresh");
        assertElementPresent("sbc:stats:list");
    }

    @Override
    public void tearDown() {
        SiteTestHelper.home(tester);
        clickLink("link:deleteBridgeSbc");
    }
}
