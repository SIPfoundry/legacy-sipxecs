/*
 * Copyright (C) 2010 Avaya, certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 */
package org.sipfoundry.sipxconfig.service;

import org.sipfoundry.sipxconfig.TestHelper;
import org.sipfoundry.sipxconfig.setting.Setting;


public class SipxPageServiceTest extends SipxServiceTestBase {

    public void testGetSipPort() {
        SipxPageService out = new SipxPageService();
        initCommonAttributes(out);

        out.setModelDir("sipxpage");
        out.setModelName("sipxpage.xml");
        out.setModelFilesContext(TestHelper.getModelFilesContext());

        Setting pageSettings = out.getSettings();
        pageSettings.getSetting("page-config/PAGE_SERVER_SIP_PORT").setValue("9905");

        assertEquals("9905", out.getSipPort());
    }
}
