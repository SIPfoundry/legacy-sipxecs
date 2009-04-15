/*
 * 
 * 
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 * 
 * $
 */
package org.sipfoundry.sipxconfig.admin.commserver.imdb;

import java.util.ArrayList;
import java.util.Collections;
import java.util.List;
import java.util.Map;

import junit.framework.TestCase;

import org.apache.commons.lang.StringUtils;
import org.sipfoundry.sipxconfig.common.CoreContext;
import org.sipfoundry.sipxconfig.common.User;

import static org.easymock.EasyMock.createMock;
import static org.easymock.EasyMock.expectLastCall;
import static org.easymock.EasyMock.replay;
import static org.easymock.EasyMock.verify;

public class ExtensionsTest extends TestCase {
    private String[][] DATA = {
        {
            "first", "last", "userName", "1234"
        }, {
            null, null, "kuku", "4321"
        }, {
            "user", "without", "extension", null
        }, {
            "abbb", "cccc", "1234", "32"
        },
    };

    private final List<User> m_users;

    public ExtensionsTest() {
        List<User> users = new ArrayList<User>();
        for (int i = 0; i < DATA.length; i++) {
            String[] userData = DATA[i];
            User user = new User();
            user.setFirstName(userData[0]);
            user.setLastName(userData[1]);
            user.setUserName(userData[2]);
            String extension = userData[3];
            if (!StringUtils.isBlank(extension)) {
                user.addAlias(extension);
            }
            if (i == 0) {
                // long extension should be ignored
                user.addAlias("555555555555");
            }
            users.add(user);
        }
        m_users = Collections.unmodifiableList(users);
    }

    public void testGenerateEmpty() throws Exception {
        CoreContext coreContext = createMock(CoreContext.class);
        coreContext.getDomainName();
        expectLastCall().andReturn("company.com");
        coreContext.loadUsers();
        expectLastCall().andReturn(Collections.EMPTY_LIST);
        replay(coreContext);

        Extensions extensions = new Extensions();
        extensions.setCoreContext(coreContext);

        List<Map<String, String>> document = extensions.generate();
        assertEquals(0, document.size());

        verify(coreContext);
    }

    public void testGenerate() throws Exception {
        CoreContext coreContext = createMock(CoreContext.class);
        coreContext.getDomainName();
        expectLastCall().andReturn("company.com");
        coreContext.loadUsers();
        expectLastCall().andReturn(m_users);
        replay(coreContext);

        Extensions extensions = new Extensions();
        extensions.setCoreContext(coreContext);

        List<Map<String, String>> items = extensions.generate();
        assertEquals(2, items.size());

        assertEquals("1234", items.get(0).get("extension"));
        assertEquals("\"first last\"<sip:userName@company.com>", items.get(0).get("uri"));
        assertEquals("4321", items.get(1).get("extension"));
        assertEquals("sip:kuku@company.com", items.get(1).get("uri"));

        verify(coreContext);
    }
}
