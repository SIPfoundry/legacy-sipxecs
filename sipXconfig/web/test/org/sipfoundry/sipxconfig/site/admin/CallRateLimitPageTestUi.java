/*
 *
 *
 * Copyright (C) 2011 eZuce, Inc. All rights reserved.
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

public class CallRateLimitPageTestUi extends WebTestCase {

    public static Test suite() throws Exception {
        return SiteTestHelper.webTestSuite(CallRateLimitPageTestUi.class);
    }

    @Override
    public void setUp() {
        getTestContext().setBaseUrl(SiteTestHelper.getBaseUrl());
        SiteTestHelper.home(getTester());
        clickLink("toggleNavigation");
        clickLink("menu.callRateLimit");
        SiteTestHelper.assertNoException(getTester());
    }

    public void testCallRateLimitUi() throws Exception {
        assertCheckboxPresent("setting:SIPX_PROXY_AUTOBAN_THRESHOLD_VIOLATORS");
        assertElementPresent("setting:SIPX_PROXY_PACKETS_PER_SECOND_THRESHOLD");
        assertElementPresent("setting:SIPX_PROXY_THRESHOLD_VIOLATION_RATE");
        assertElementPresent("setting:SIPX_PROXY_BAN_LIFETIME");
        assertElementPresent("setting:SIPX_PROXY_WHITE_LIST");
        assertElementPresent("setting:SIPX_PROXY_BLACK_LIST");
        assertButtonPresent("form:apply");
    }

}
