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

import junit.framework.TestCase;

import org.easymock.EasyMock;
import org.easymock.IMocksControl;
import org.sipfoundry.sipxconfig.domain.Domain;
import org.sipfoundry.sipxconfig.domain.DomainManager;
import org.sipfoundry.sipxconfig.setting.Setting;
import org.sipfoundry.sipxconfig.test.TestHelper;

public class SipxImbotServiceTest extends TestCase {

    public void testGenerateImPassword() {
        SipxImbotService imBotService = new SipxImbotService();
        imBotService.setModelDir("sipximbot");
        imBotService.setModelName("sipximbot.xml");
        imBotService.setModelFilesContext(TestHelper.getModelFilesContext());
        IMocksControl domainManagerControl = EasyMock.createControl();
        DomainManager domainManager = domainManagerControl.createMock(DomainManager.class);
        domainManager.getDomain();
        EasyMock.expectLastCall().andReturn(new Domain()).anyTimes();
        EasyMock.replay(domainManager);
        imBotService.setDomainManager(domainManager);

        Setting imBotSettings = imBotService.getSettings();
        imBotSettings.getSetting("imbot/imPassword").setValue("[P2\\sw0\\rd]");
        assertEquals("[P2\\\\sw0\\\\rd]", imBotService.getPersonalAssistantImPassword());
        imBotSettings.getSetting("imbot/imPassword").setValue(null);
        assertEquals("MyBuddy", imBotService.getPersonalAssistantImPassword());
    }

}
