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
import org.sipfoundry.sipxconfig.commserver.imdb.RegistrationItem;
import org.sipfoundry.sipxconfig.domain.DomainManager;
import org.sipfoundry.sipxconfig.registrar.RegistrationContextImpl;
import org.sipfoundry.sipxconfig.test.ImdbTestCase;
import org.springframework.data.mongodb.core.MongoTemplate;

import com.mongodb.BasicDBList;
import com.mongodb.BasicDBObject;
import com.mongodb.DBCollection;
import com.mongodb.DBCursor;
import com.mongodb.DBObject;

public class RegistrationContextTestIntegration extends ImdbTestCase {
    private RegistrationContextImpl m_builder;
    private MongoTemplate m_nodeDb;
    private DomainManager m_mgr;

    private Object[][] DATA = {
        {
            "063b4c2f5e11bf66a232762a7cf9e73a", "2395", "sip:3000@example.org",
            "\"John Doe\"<sip:john.doe@example.org;LINEID=f57f2117d5997f8d03d8395732f463f3>", true,
            "3000@example.org", 1299762967, 1299762667, "0004f22aa38a", "1", "3f404b64-fc8490c3-6b14ac9a@192.168.2.21"
        },
        {
            "063b4c2f5e11bf66a232762a7cf9e73b", "2399", "sip:3001@example.org",
            "\"John Doe\"<sip:jane.doe@example.org>", false, "3001@example.org", 1299762968, 1299762668,
            "0004f2a9b633", "2", "3f404b64-fc8490c3-6b14ac9a@192.168.2.19"
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
        reg1.put("identity", DATA[0][5]);
        reg1.put("_id", DATA[0][9]);
        reg1.put("callId", DATA[0][10]);
        DBObject reg2 = new BasicDBObject();
        reg2.put("contact", DATA[1][3]);
        reg2.put("expirationTime", DATA[1][6]);
        reg2.put("uri", DATA[1][2]);
        reg2.put("instrument", DATA[1][8]);
        reg2.put("expired", DATA[1][4]);
        reg2.put("identity", DATA[1][5]);
        reg2.put("_id", DATA[1][9]);
        reg2.put("callId", DATA[1][10]);

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

    public void testGetRegistrationsByMac() throws Exception {
        DBCursor registrations = m_builder.getRegistrationsByMac("0004f22aa38a");
        assertFalse(registrations.hasNext());
        registrations = m_builder.getRegistrationsByMac("0004f2a9b633");
        assertTrue(registrations.hasNext());
        BasicDBList list = new BasicDBList();
        while (registrations.hasNext()) {
            BasicDBObject registration = (BasicDBObject) registrations.next();
            list.add(registration);
        }
        assertEquals(
                "[ { \"_id\" : \"2\" , \"contact\" : \"\\\"John Doe\\\"<sip:jane.doe@example.org>\" , \"expirationTime\" : 1299762968 , \"uri\" : \"sip:3001@example.org\" , \"instrument\" : \"0004f2a9b633\" , \"expired\" : false , \"identity\" : \"3001@example.org\" , \"callId\" : \"3f404b64-fc8490c3-6b14ac9a@192.168.2.19\"}]",
                list.toString());
    }

    public void testGetRegistrationsByIp() throws Exception {
        DBCursor registrations = m_builder.getRegistrationsByIp("192.168.2.21");
        assertFalse(registrations.hasNext());
        registrations = m_builder.getRegistrationsByIp("192.168.2.19");
        assertTrue(registrations.hasNext());
        BasicDBList list = new BasicDBList();
        while (registrations.hasNext()) {
            BasicDBObject registration = (BasicDBObject) registrations.next();
            list.add(registration);
        }
        assertEquals(
                "[ { \"_id\" : \"2\" , \"contact\" : \"\\\"John Doe\\\"<sip:jane.doe@example.org>\" , \"expirationTime\" : 1299762968 , \"uri\" : \"sip:3001@example.org\" , \"instrument\" : \"0004f2a9b633\" , \"expired\" : false , \"identity\" : \"3001@example.org\" , \"callId\" : \"3f404b64-fc8490c3-6b14ac9a@192.168.2.19\"}]",
                list.toString());
    }

    public void testGetRegistrationsByUid() throws Exception {
        DBCursor registrations = m_builder.getRegistrationsByLineId("3000");
        assertFalse(registrations.hasNext());
        registrations = m_builder.getRegistrationsByLineId("3001");
        assertTrue(registrations.hasNext());
        BasicDBList list = new BasicDBList();
        while (registrations.hasNext()) {
            BasicDBObject registration = (BasicDBObject) registrations.next();
            list.add(registration);
        }
        assertEquals(
                "[ { \"_id\" : \"2\" , \"contact\" : \"\\\"John Doe\\\"<sip:jane.doe@example.org>\" , \"expirationTime\" : 1299762968 , \"uri\" : \"sip:3001@example.org\" , \"instrument\" : \"0004f2a9b633\" , \"expired\" : false , \"identity\" : \"3001@example.org\" , \"callId\" : \"3f404b64-fc8490c3-6b14ac9a@192.168.2.19\"}]",
                list.toString());
    }
    
    public void setNodeDb(MongoTemplate nodeDb) {
        m_nodeDb = nodeDb;
    }

    public void setDomainManager(DomainManager mgr) {
        m_mgr = mgr;
    }

}
