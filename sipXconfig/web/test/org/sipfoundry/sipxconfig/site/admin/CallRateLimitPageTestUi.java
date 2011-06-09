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
package org.sipfoundry.sipxconfig.site.admin;

import net.sourceforge.jwebunit.junit.WebTestCase;

import org.sipfoundry.sipxconfig.site.SiteTestHelper;

public class CallRateLimitPageTestUi extends WebTestCase {

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
