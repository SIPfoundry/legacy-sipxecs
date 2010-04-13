/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.admin.commserver;

import java.util.ArrayList;
import java.util.Collections;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

import junit.framework.TestCase;

import org.sipfoundry.sipxconfig.admin.commserver.imdb.RegistrationItem;
import org.sipfoundry.sipxconfig.common.User;

public class RegistrationContextImplTest extends TestCase {

    private RegistrationContextImpl m_builder;

    private Object[][] DATA = {
        {
            "063b4c2f5e11bf66a232762a7cf9e73a", "2395", "3000@example.org",
            "\"John Doe\"<sip:john.doe@example.org;LINEID=f57f2117d5997f8d03d8395732f463f3>",
            2000
        },
        {
            "063b4c2f5e11bf66a232762a7cf9e73b", "2399", "3001@example.org",
            "\"John Doe\"<sip:jane.doe@example.org>", 2001
        }
    };

    private String[] FIELDS = {
        "callid", "cseq", "uri", "contact", "expires"
    };

    private List<Map<String, ?>> m_data;

    protected void setUp() throws Exception {
        m_builder = new RegistrationContextImpl();

        m_data = new ArrayList<Map<String, ?>>();
        for (int i = 0; i < DATA.length; i++) {
            Map<String, Object> item = new HashMap<String, Object>();
            for (int f = 0; f < FIELDS.length; f++) {
                item.put(FIELDS[f], DATA[i][f]);
            }
            m_data.add(item);
        }
    }

    public void testGetRegistrations() throws Exception {
        List registrations = m_builder.getRegistrations(m_data);
        assertEquals(2, registrations.size());
        for (int i = 0; i < 2; i++) {
            RegistrationItem ri = (RegistrationItem) registrations.get(i);
            assertEquals(2000 + i, ri.getExpires());
            assertTrue(ri.getUri().startsWith("300" + i));
            assertTrue(ri.getContact().indexOf("Doe") > 0);
        }
    }

    public void testGetRegistrationsByUser() throws Exception {
        List<RegistrationItem> registrations = m_builder.getRegistrations(m_data);
        User user = new User();
        user.setUserName("3000");
        registrations = m_builder.getRegistrationsByUser(registrations, user);
        assertEquals(1, registrations.size());
        RegistrationItem ri = registrations.get(0);
        assertEquals(2000, ri.getExpires());
        assertTrue(ri.getUri().startsWith("3000"));
        assertTrue(ri.getContact().indexOf("Doe") > 0);
    }

    public void testGetRegistrationsEmpty() throws Exception {
        List registrations = m_builder.getRegistrations(Collections.<Map<String, ?>>emptyList());
        assertTrue(registrations.isEmpty());
    }
}
