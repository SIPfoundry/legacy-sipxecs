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
import java.util.HashMap;
import java.util.List;
import java.util.Map;

import junit.framework.TestCase;

import org.easymock.EasyMock;
import org.sipfoundry.sipxconfig.admin.commserver.imdb.RegistrationItem;
import org.sipfoundry.sipxconfig.common.User;

import com.mongodb.BasicDBObject;
import com.mongodb.DB;
import com.mongodb.DBCollection;
import com.mongodb.DBObject;
import com.mongodb.Mongo;

public class RegistrationContextImplTest extends TestCase {
    private String m_host = "localhost";
    private int m_port = 27017;
    private Mongo m_mongoInstance;
    private static DBCollection m_collection;
    public final static String DBNAME = "node";
    public final static String COLL_NAME = "registrar";
    private RegistrationContextImpl m_builder;

    private Object[][] DATA = {
        {
            "063b4c2f5e11bf66a232762a7cf9e73a", "2395", "3000@example.org",
            "\"John Doe\"<sip:john.doe@example.org;LINEID=f57f2117d5997f8d03d8395732f463f3>", true,
            "3000@example.org", 1299762967, 1299762667, ""
        },
        {
            "063b4c2f5e11bf66a232762a7cf9e73b", "2399", "3001@example.org",
            "\"John Doe\"<sip:jane.doe@example.org>", false, "3001@example.org", 1299762968, 1299762668, ""
        }
    };

    /*
     * private String[] FIELDS = { "callid", "cseq", "uri", "contact", "expired", "identity",
     * "expirationTime", "timestamp", "instrument" };
     */

    protected void setUp() throws Exception {
        if (m_mongoInstance == null) {
            m_mongoInstance = new Mongo(m_host, m_port);
        }
        DB datasetDb = m_mongoInstance.getDB(DBNAME);
        m_collection = datasetDb.getCollection(COLL_NAME);
        DBObject reg1 = new BasicDBObject();
        reg1.put("contact", DATA[0][3]);
        reg1.put("expirationTime", DATA[0][6]);
        reg1.put("uri", DATA[0][2]);
        reg1.put("instrument", DATA[0][8]);
        reg1.put("expired", DATA[0][4]);
        DBObject reg2 = new BasicDBObject();
        reg2.put("contact", DATA[1][3]);
        reg2.put("expirationTime", DATA[1][6]);
        reg2.put("uri", DATA[1][2]);
        reg2.put("instrument", DATA[1][8]);
        reg2.put("expired", DATA[1][4]);

        m_collection.insert(reg1, reg2);

        Location loc = new Location();
        LocationsManager lm = EasyMock.createMock(LocationsManager.class);
        lm.getLocationByBundle("primarySipRouterBundle");
        EasyMock.expectLastCall().andReturn(loc);
        EasyMock.replay(lm);
        m_builder = new RegistrationContextImpl();
        m_builder.setLocationsManager(lm);
    }

    public void testGetRegistrations() throws Exception {
        List registrations = m_builder.getRegistrations();
        assertEquals(1, registrations.size());
        RegistrationItem ri = (RegistrationItem) registrations.get(0);
        assertEquals(1299762968, ri.getExpires());
        assertEquals("3001@example.org", ri.getUri());
        assertTrue(ri.getContact().indexOf("Doe") > 0);
    }

    public void testGetRegistrationsByUser() throws Exception {
        List<RegistrationItem> registrations = m_builder.getRegistrations();
        User user = new User();
        user.setUserName("3001");
        registrations = m_builder.getRegistrationsByUser(registrations, user);
        assertEquals(1, registrations.size());
        RegistrationItem ri = registrations.get(0);
        assertEquals(1299762968, ri.getExpires());
        assertEquals("3001@example.org", ri.getUri());
        assertTrue(ri.getContact().indexOf("Doe") > 0);
    }

    @Override
    protected void tearDown() throws Exception {
        m_mongoInstance.dropDatabase(DBNAME);
        super.tearDown();
    }
}
