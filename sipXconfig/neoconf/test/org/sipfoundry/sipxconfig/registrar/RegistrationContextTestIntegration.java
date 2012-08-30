/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.registrar;

import java.util.List;

import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.commserver.imdb.ImdbTestCase;
import org.sipfoundry.sipxconfig.commserver.imdb.RegistrationItem;
import org.sipfoundry.sipxconfig.domain.DomainManager;
import org.sipfoundry.sipxconfig.registrar.RegistrationContextImpl;
import org.springframework.data.mongodb.core.MongoTemplate;

import com.mongodb.BasicDBObject;
import com.mongodb.DBCollection;
import com.mongodb.DBObject;

public class RegistrationContextTestIntegration extends ImdbTestCase {
    private RegistrationContextImpl m_builder;
    private MongoTemplate m_nodeDb;
    private DomainManager m_mgr;

    private Object[][] DATA = {
        {
            "063b4c2f5e11bf66a232762a7cf9e73a", "2395", "sip:3000@example.org",
            "\"John Doe\"<sip:john.doe@example.org;LINEID=f57f2117d5997f8d03d8395732f463f3>", true,
            "3000@example.org", 1299762967, 1299762667, ""
        },
        {
            "063b4c2f5e11bf66a232762a7cf9e73b", "2399", "sip:3001@example.org",
            "\"John Doe\"<sip:jane.doe@example.org>", false, "3001@example.org", 1299762968, 1299762668, ""
        }
    };
    
    private DBCollection getRegistrarCollection() {
        return m_nodeDb.getDb().getCollection("registrar");
    }

    @Override
    protected void onSetUpInTransaction() throws Exception {
        super.onSetUpInTransaction();
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

        m_nodeDb.getDb().dropDatabase();
        getRegistrarCollection().insert(reg1, reg2);

        m_builder = new RegistrationContextImpl();
        m_builder.setNodedb(m_nodeDb);
        m_builder.setDomainManager(m_mgr);
    }

    public void testGetRegistrations() throws Exception {
        List registrations = m_builder.getRegistrations();
        assertEquals(1, registrations.size());
        RegistrationItem ri = (RegistrationItem) registrations.get(0);
        assertEquals(1299762968, ri.getExpires());
        assertEquals("sip:3001@example.org", ri.getUri());
        assertTrue(ri.getContact().indexOf("Doe") > 0);
    }

    public void testGetRegistrationsByUser() throws Exception {
        List<RegistrationItem> registrations = m_builder.getRegistrations();
        User user = new User();
        user.setUserName("3001");
        registrations = m_builder.getRegistrationsByUser(user);
        assertEquals(1, registrations.size());
        RegistrationItem ri = registrations.get(0);
        assertEquals(1299762968, ri.getExpires());
        assertEquals("sip:3001@example.org", ri.getUri());
        assertTrue(ri.getContact().indexOf("Doe") > 0);
    }

    public void setNodeDb(MongoTemplate nodeDb) {
        m_nodeDb = nodeDb;
    }

    public void setDomainManager(DomainManager mgr) {
        m_mgr = mgr;
    }

}
