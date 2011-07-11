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
package org.sipfoundry.sipxconfig.admin.commserver.imdb;

import java.util.ArrayList;
import java.util.List;
import java.util.Map;

import org.sipfoundry.sipxconfig.IntegrationTestCase;
import org.sipfoundry.sipxconfig.TestHelper;
import org.sipfoundry.sipxconfig.common.SpecialUser.SpecialUserType;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.permission.PermissionManagerImpl;
import org.sipfoundry.sipxconfig.permission.PermissionName;
import org.sipfoundry.sipxconfig.setting.Group;

public class PermissionsTestIntegration extends IntegrationTestCase {
    // needs to be adjusted every time a new permission is added
    private static int PERM_COUNT = 5;
    private static int SPEC_COUNT = SpecialUserType.values().length;
    private Permissions m_permissions;

    public void testGenerateEmpty() throws Exception {
        loadDataSetXml("domain/DomainSeed.xml");
        List<Map<String, String>> items = ((DataSetGenerator) m_permissions).generate();

        // As PHONE_PROVISION does NOT require any permissions, don't count it.
        assertEquals((SPEC_COUNT -1) * PERM_COUNT, items.size());
        // 5 permissions per special user

        for (SpecialUserType su : SpecialUserType.values()) {
            int i = su.ordinal();
            // As PHONE_PROVISION does NOT require any permissions, skip it.
            if (!su.equals(SpecialUserType.PHONE_PROVISION)) {
                String name = "sip:" + su.getUserName() + "@example.org";
                assertEquals(name, items.get(i * PERM_COUNT).get("identity"));
                assertEquals(name, items.get((i + 1) * PERM_COUNT - 1).get("identity"));
            }
        }
    }

    public void testCallGroupPerms() throws Exception {
        loadDataSetXml("domain/DomainSeed.xml");
        loadDataSetXml("admin/commserver/imdb/PermissionsCallGroupSeed.xml");

        List<Map<String, String>> items = ((DataSetGenerator) m_permissions).generate();

        // As PHONE_PROVISION does NOT require any permissions, don't count it.
        int start = (SPEC_COUNT - 1) * PERM_COUNT;

        assertEquals(start + 10, items.size());

        assertEquals("sip:sales@example.org", items.get(start + 0).get("identity"));
        assertEquals("sip:sales@example.org", items.get(start + 4).get("identity"));
        assertEquals("sip:eng@example.org", items.get(start + 5).get("identity"));
        assertEquals("sip:eng@example.org", items.get(start + 9).get("identity"));

    }

    public void testUsersPerms() throws Exception {
        loadDataSetXml("domain/DomainSeed.xml");
        loadDataSet("admin/commserver/imdb/PermissionsUsersSeed.xml");

        List<Map<String, String>> items = ((DataSetGenerator) m_permissions).generate();
        // As PHONE_PROVISION does NOT require any permissions, don't count it.
        int start = (SPEC_COUNT - 1) * PERM_COUNT;
        // 9 + 8 + 7 + 9
        assertEquals(start + 33, items.size());

        assertEquals("\"sam\"<sip:alpha@example.org>", items.get(start + 0).get("identity"));
        assertEquals("sip:~~vm~alpha@example.org", items.get(start + 8).get("identity"));
        assertEquals("sip:alpha1@example.org", items.get(start + 9).get("identity"));
        assertEquals("sip:~~vm~alpha1@example.org", items.get(start + 16).get("identity"));
        assertEquals("sip:alpha2@example.org", items.get(start + 17).get("identity"));
        assertEquals("sip:~~vm~alpha2@example.org", items.get(start + 23).get("identity"));
        assertEquals("\"sam3\"<sip:alpha3@example.org>", items.get(start + 24).get("identity"));
        assertEquals("sip:~~vm~alpha3@example.org", items.get(start + 32).get("identity"));

    }

    public void testAddUser() throws Exception {
        List<Map<String, String>> items = new ArrayList<Map<String, String>>();

        PermissionManagerImpl pm = new PermissionManagerImpl();
        pm.setModelFilesContext(TestHelper.getModelFilesContext());

        User user = new User();
        user.setPermissionManager(pm);

        Group g = new Group();
        PermissionName.INTERNATIONAL_DIALING.setEnabled(g, false);
        PermissionName.LONG_DISTANCE_DIALING.setEnabled(g, false);
        PermissionName.TOLL_FREE_DIALING.setEnabled(g, false);
        PermissionName.LOCAL_DIALING.setEnabled(g, true);
        PermissionName.FREESWITH_VOICEMAIL.setEnabled(g, false);
        PermissionName.EXCHANGE_VOICEMAIL.setEnabled(g, true);

        user.addGroup(g);
        user.setUserName("goober");

        Permissions permissions = new Permissions();
        permissions.addUser(items, user, "sipx.sipfoundry.org");

        assertEquals(PERM_COUNT, items.size());
        assertEquals("sip:goober@sipx.sipfoundry.org", items.get(0).get("identity"));
        assertEquals("LocalDialing", items.get(0).get("permission"));

        assertEquals("sip:goober@sipx.sipfoundry.org", items.get(3).get("identity"));
        assertEquals("ExchangeUMVoicemailServer", items.get(3).get("permission"));

        assertEquals("sip:~~vm~goober@sipx.sipfoundry.org", items.get(4).get("identity"));
        assertEquals("ExchangeUMVoicemailServer", items.get(4).get("permission"));
    }

    public void setPermissionDataSet(Permissions permissions) {
        m_permissions = permissions;
    }
}
