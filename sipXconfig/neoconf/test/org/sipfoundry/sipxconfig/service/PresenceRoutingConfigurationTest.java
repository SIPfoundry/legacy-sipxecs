/*
 *
 *
 * Copyright (C) 2009 Nortel, certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 *
 */
package org.sipfoundry.sipxconfig.service;

import java.io.InputStream;
import java.util.ArrayList;
import java.util.List;

import junit.framework.TestCase;

import org.apache.commons.io.IOUtils;
import org.dom4j.Document;
import org.easymock.EasyMock;
import org.sipfoundry.sipxconfig.TestHelper;
import org.sipfoundry.sipxconfig.common.CoreContext;
import org.sipfoundry.sipxconfig.common.DaoUtils;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.test.TestUtil;

public class PresenceRoutingConfigurationTest extends TestCase {

    public void testXmlGeneration() throws Exception {
        CoreContext coreContext = EasyMock.createMock(CoreContext.class);
        List<User> users = new ArrayList<User>();

        for (int userId = 200; userId < 205; userId++) {
            User user = new User();
            user.setSettings(TestHelper.loadSettings("commserver/user-settings.xml"));
            user.setUserName(String.valueOf(userId));

            if (201 == userId || 203 == userId) {
                user.setSettingTypedValue("im/fwd-vm-on-dnd", true);
            }
            users.add(user);
        }

        coreContext.loadUsersByPage(0, DaoUtils.PAGE_SIZE);
        EasyMock.expectLastCall().andReturn(users);
        EasyMock.replay(coreContext);

        PresenceRoutingConfiguration presenceRoutingConfiguration = new PresenceRoutingConfiguration();
        presenceRoutingConfiguration.setCoreContext(coreContext);

        Document document = presenceRoutingConfiguration.getDocument();
        String domDoc = TestUtil.asString(document);

        InputStream referenceXml = PresenceRoutingConfiguration.class.getResourceAsStream("presencerouting-prefs.test.xml");
        assertEquals(IOUtils.toString(referenceXml), domDoc);
    }
}
