/*
 *
 *
 * Copyright (C) 2009 Nortel, certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 *
 */
package org.sipfoundry.sipxconfig.admin.commserver.imdb;

import java.io.InputStream;
import java.util.ArrayList;
import java.util.List;
import java.util.Map;

import junit.framework.TestCase;

import org.apache.commons.io.IOUtils;
import org.dom4j.Document;
import org.easymock.EasyMock;
import org.sipfoundry.sipxconfig.TestHelper;
import org.sipfoundry.sipxconfig.common.CoreContext;
import org.sipfoundry.sipxconfig.common.DaoUtils;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.test.TestUtil;

public class PresenceRoutingTest extends TestCase {

    public void testDataSetGeneration() throws Exception {
        CoreContext coreContext = EasyMock.createMock(CoreContext.class);
        coreContext.getDomainName();
        EasyMock.expectLastCall().andReturn("host.company.com");
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

        PresenceRouting routing = new PresenceRouting();
        routing.setCoreContext(coreContext);
        List<Map<String, String>> items = routing.generate();
        assertEquals("200@host.company.com", items.get(0).get("identity"));
        assertEquals("false", items.get(0).get("vmOnDnd"));

        assertEquals("201@host.company.com", items.get(1).get("identity"));
        assertEquals("true", items.get(1).get("vmOnDnd"));

        assertEquals("202@host.company.com", items.get(2).get("identity"));
        assertEquals("false", items.get(2).get("vmOnDnd"));

        assertEquals("203@host.company.com", items.get(3).get("identity"));
        assertEquals("true", items.get(3).get("vmOnDnd"));

        assertEquals("204@host.company.com", items.get(4).get("identity"));
        assertEquals("false", items.get(4).get("vmOnDnd"));
    }
}
