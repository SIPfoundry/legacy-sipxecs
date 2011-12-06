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
package org.sipfoundry.sipxconfig.service;

import org.sipfoundry.sipxconfig.common.UserException;
import org.sipfoundry.sipxconfig.setting.Setting;
import org.sipfoundry.sipxconfig.test.TestHelper;


public class SipxProxyServiceTest extends SipxServiceTestBase {

    public void testValidate() throws Exception {

        SipxProxyService proxyService = new SipxProxyService();
        initCommonAttributes(proxyService);
        Setting settings = TestHelper.loadSettings("sipxproxy/sipxproxy.xml");
        proxyService.setSettings(settings);

        Setting callRateLimit = proxyService.getSettings().getSetting("call-rate-limit");
        callRateLimit.getSetting("SIPX_PROXY_WHITE_LIST").setValue("10.1.1.1, 10.1.1.2");
        callRateLimit.getSetting("SIPX_PROXY_BLACK_LIST").setValue("10.1.1.3, 10.1.1.4");
        try {
            proxyService.validate();
        } catch (UserException ex) {
            fail();
        }

        callRateLimit.getSetting("SIPX_PROXY_WHITE_LIST").setValue("10.1.1.1, 10.1.1.2/24");
        callRateLimit.getSetting("SIPX_PROXY_BLACK_LIST").setValue("10.1.1.3/18, 10.1.1.4/9");
        try {
            proxyService.validate();
        } catch (UserException ex) {
            fail();
        }

        callRateLimit.getSetting("SIPX_PROXY_WHITE_LIST").setValue("10.1.1.1/999");
        try {
            proxyService.validate();
            fail();
        } catch (UserException ex) {
            assertEquals("&msg.invalidWhiteList", ex.getMessage());
            assertEquals("10.1.1.1/999", ex.getRawParams()[0]);
        }

        callRateLimit.getSetting("SIPX_PROXY_WHITE_LIST").setValue("10.1.1.999, 10.1.1.2");
        try {
            proxyService.validate();
            fail();
        } catch (UserException ex) {
            assertEquals("&msg.invalidWhiteList", ex.getMessage());
            assertEquals("10.1.1.999", ex.getRawParams()[0]);
        }

        callRateLimit.getSetting("SIPX_PROXY_WHITE_LIST").setValue("10.1.1.1, 10.1.1.2");
        callRateLimit.getSetting("SIPX_PROXY_BLACK_LIST").setValue("10.1.1.555, 10.1.1.2");
        try {
            proxyService.validate();
            fail();
        } catch (UserException ex) {
            assertEquals("&msg.invalidBlackList", ex.getMessage());
            assertEquals("10.1.1.555", ex.getRawParams()[0]);
        }

    }
}
