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

import org.sipfoundry.sipxconfig.common.DaoUtils;

import junit.framework.TestCase;

import org.sipfoundry.sipxconfig.admin.callgroup.CallGroup;
import org.sipfoundry.sipxconfig.admin.callgroup.CallGroupContext;
import org.sipfoundry.sipxconfig.common.CoreContext;
import org.sipfoundry.sipxconfig.common.Md5Encoder;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.common.SpecialUser.SpecialUserType;

import static org.easymock.EasyMock.createMock;
import static org.easymock.EasyMock.expectLastCall;
import static org.easymock.EasyMock.replay;
import static org.easymock.EasyMock.verify;

public class CredentialsTest extends TestCase {
    public void testGenerateEmpty() throws Exception {
        CoreContext coreContext = createMock(CoreContext.class);
        coreContext.getDomainName();
        expectLastCall().andReturn("host.company.com");
        coreContext.getAuthorizationRealm();
        expectLastCall().andReturn("company.com");
        coreContext.loadUsersByPage(0, DaoUtils.PAGE_SIZE);
        expectLastCall().andReturn(Collections.EMPTY_LIST);
        coreContext.loadInternalUsers();
        expectLastCall().andReturn(Collections.EMPTY_LIST);

        User user = new User();
        user.setSipPassword("test");
        user.setUserName("user");
        for (SpecialUserType u : SpecialUserType.values()) {
            coreContext.getSpecialUser(u);
            expectLastCall().andReturn(user);
        }

        CallGroupContext callGroupContext = createMock(CallGroupContext.class);
        callGroupContext.getCallGroups();
        expectLastCall().andReturn(Collections.EMPTY_LIST);

        replay(coreContext, callGroupContext);

        Credentials credentials = new Credentials();
        credentials.setCoreContext(coreContext);
        credentials.setCallGroupContext(callGroupContext);

        List<Map<String, String>> items = credentials.generate();

        assertEquals(SpecialUserType.values().length, items.size());
        assertEquals("sip:user@host.company.com", items.get(0).get("uri"));
        assertEquals("sip:user@host.company.com", items.get(1).get("uri"));
        assertEquals("sip:user@host.company.com", items.get(2).get("uri"));
        assertEquals("sip:user@host.company.com", items.get(3).get("uri"));

        verify(coreContext, callGroupContext);
    }

    public void testAddCallgroup() throws Exception {
        List<Map<String, String>> items = new ArrayList<Map<String, String>>();

        CallGroup cg = new CallGroup();
        cg.setName("sales");
        cg.setSipPassword("pass4321");

        Credentials credentials = new Credentials();
        credentials.addCallGroup(items, cg, "sipx.sipfoundry.org", "sipfoundry.org");

        // Md5Encoder.digestPassword("sales", "sipfoundry.org", "pass4321");
        String digest = "282e44b75e1e04d379d3157c34e31814";
        Map<String, String> item = items.get(0);
        assertEquals("sip:sales@sipx.sipfoundry.org", item.get("uri"));
        assertEquals(digest, item.get("pintoken"));
        assertEquals("pass4321", item.get("passtoken"));
        assertEquals("sipfoundry.org", item.get("realm"));
        assertEquals("DIGEST", item.get("authtype"));
    }

    public void testAddUser() throws Exception {
        List<Map<String, String>> items = new ArrayList<Map<String, String>>();
        User user = new User();
        user.setUserName("superadmin");
        final String PIN = "pin1234";
        user.setPin(PIN, "sipfoundry.org");
        user.setSipPassword("pass4321");

        Credentials credentials = new Credentials();
        credentials.addUser(items, user, "sipx.sipfoundry.org", "sipfoundry.org");

        assertEquals(1, items.size());
        Map<String, String> item = items.get(0);

        assertEquals("sip:superadmin@sipx.sipfoundry.org", item.get("uri"));
        assertEquals(Md5Encoder.digestPassword("superadmin", "sipfoundry.org", PIN), item.get("pintoken"));
        assertEquals("pass4321", item.get("passtoken"));
        assertEquals("sipfoundry.org", item.get("realm"));
        assertEquals("DIGEST", item.get("authtype"));
    }

    public void testAddUserEmptyPasswords() throws Exception {
        List<Map<String, String>> items = new ArrayList<Map<String, String>>();

        User user = new User();
        user.setUserName("superadmin");
        user.setPin("", "sipfoundry.org");

        Credentials credentials = new Credentials();
        credentials.addUser(items, user, "sipx.sipfoundry.org", "sipfoundry.org");

        assertEquals(1, items.size());
        Map<String, String> item = items.get(0);

        assertEquals("sip:superadmin@sipx.sipfoundry.org", item.get("uri"));
        String emptyHash = Md5Encoder.digestPassword("superadmin", "sipfoundry.org", "");
        assertEquals(emptyHash, item.get("pintoken"));
        assertEquals("", item.get("passtoken"));
        assertEquals("sipfoundry.org", item.get("realm"));
        assertEquals("DIGEST", item.get("authtype"));
    }
}
