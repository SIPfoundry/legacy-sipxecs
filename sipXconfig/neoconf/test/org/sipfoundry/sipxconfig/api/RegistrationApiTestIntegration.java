package org.sipfoundry.sipxconfig.api;

import org.sipfoundry.sipxconfig.common.CoreContext;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.phone.Phone;
import org.sipfoundry.sipxconfig.phone.PhoneContext;
import org.sipfoundry.sipxconfig.phone.TestPhoneModel;
import org.sipfoundry.sipxconfig.test.RestApiIntegrationTestCase;
import org.skyscreamer.jsonassert.JSONAssert;
import org.springframework.data.mongodb.core.MongoTemplate;

import com.mongodb.BasicDBObject;
import com.mongodb.DBCollection;
import com.mongodb.DBObject;

public class RegistrationApiTestIntegration extends RestApiIntegrationTestCase {
    private MongoTemplate m_nodeDb;
    private CoreContext m_coreContext;
    private PhoneContext m_phoneContext;

    private final Object[][] DATA = {
        {
            "063b4c2f5e11bf66a232762a7cf9e73a", "2395", "sip:3000@example.org",
            "\"John Doe\"<sip:john.doe@example.org;LINEID=f57f2117d5997f8d03d8395732f463f3>", true,
            "3000@example.org", 1299762967, 1299762667, "0004f22aa38a", "1",
            "3f404b64-fc8490c3-6b14ac9a@192.168.2.21"
        },
        {
            "063b4c2f5e11bf66a232762a7cf9e73b", "2399", "sip:3001@example.org",
            "\"John Doe\"<sip:jane.doe@example.org>", false, "3001@example.org", 1299762968, 1299762668,
            "0004f2a9b633", "2", "3f404b64-fc8490c3-6b14ac9a@192.168.2.19"
        },
        {
            "063b4c2f5e11bf66a232762a7cf9e73c", "2396", "sip:3003@example.org",
            "\"John Doe\"<sip:jane.doe@example.org>", false, "3003@example.org", 1299762969, 1299762669,
            "0004f2a9b633", "3", "3f404b64-fc8490c3-6b14ac9a@192.168.2.20", "192.168.0.26"
        },
        {
            "063b4c2f5e11bf66a232762a7cf9e73d", "2397", "sip:3004@example.org", "\"Xxx\"<sip:xxx.yyy@example.org>",
            false, "3004@example.org", 1299762969, 1299762669, "0004f2a9b634", "4",
            "3f404b64-fc8490c3-6b14ac9a@192.168.2.25"
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

        // this one has the same contact , it will in the retrieved list
        // which filters unique regs by contact and it chooses the newer one
        DBObject reg3 = new BasicDBObject();
        reg3.put("contact", DATA[2][3]);
        reg3.put("expirationTime", DATA[2][6]);
        reg3.put("uri", DATA[2][2]);
        reg3.put("instrument", DATA[2][8]);
        reg3.put("expired", DATA[2][4]);
        reg3.put("identity", DATA[2][5]);
        reg3.put("_id", DATA[2][9]);
        reg3.put("callId", DATA[2][10]);
        reg3.put("localAddress", DATA[2][11]);

        DBObject reg4 = new BasicDBObject();
        reg4.put("contact", DATA[3][3]);
        reg4.put("expirationTime", DATA[3][6]);
        reg4.put("uri", DATA[3][2]);
        reg4.put("instrument", DATA[3][8]);
        reg4.put("expired", DATA[3][4]);
        reg4.put("identity", DATA[3][5]);
        reg4.put("_id", DATA[3][9]);
        reg4.put("callId", DATA[3][10]);

        m_nodeDb.getDb().dropDatabase();
        getRegistrarCollection().insert(reg1, reg2, reg3, reg4);
    }

    public void testGetRegistrations() throws Exception {
        String registrations = getAsJson("/registrations/");
        String expected = "{'registrations':["
                + "{'secondsToExpire':0,'status':'expired','primary':'192.168.0.26','uri':'sip:3003@example.org','identity':'3003@example.org','contact':'\"John Doe\"<sip:jane.doe@example.org>','expires':1299762969,'instrument':'0004f2a9b633','regCallId':'3f404b64-fc8490c3-6b14ac9a@192.168.2.20'},"
                + "{'secondsToExpire':0,'status':'expired','primary':null,'uri':'sip:3004@example.org','identity':'3004@example.org','contact':'\"Xxx\"<sip:xxx.yyy@example.org>','expires':1299762969,'instrument':'0004f2a9b634','regCallId':'3f404b64-fc8490c3-6b14ac9a@192.168.2.25'}"
                + "]}";
        JSONAssert.assertEquals(expected, registrations, false);

        String registrations1 = getAsJson("/registrations/?start=0&limit=1");
        String expected1 = "{'registrations':["
                + "{'secondsToExpire':0,'status':'expired','primary':'192.168.0.26','uri':'sip:3003@example.org','identity':'3003@example.org','contact':'\"John Doe\"<sip:jane.doe@example.org>','expires':1299762969,'instrument':'0004f2a9b633','regCallId':'3f404b64-fc8490c3-6b14ac9a@192.168.2.20'},"
                + "]}";
        JSONAssert.assertEquals(expected1, registrations1, false);

        String registrations2 = getAsJson("/registrations/?start=1&limit=1");
        String expected2 = "{'registrations':["
                + "{'secondsToExpire':0,'status':'expired','primary':null,'uri':'sip:3004@example.org','identity':'3004@example.org','contact':'\"Xxx\"<sip:xxx.yyy@example.org>','expires':1299762969,'instrument':'0004f2a9b634','regCallId':'3f404b64-fc8490c3-6b14ac9a@192.168.2.25'}"
                + "]}";
        JSONAssert.assertEquals(expected2, registrations2, false);

    }

    public void testGetRegistrationsByUser() throws Exception {
        disableDaoEventPublishing();
        clear();
        User u = m_coreContext.newUser();
        u.setUserName("3003");
        m_coreContext.saveUser(u);
        commit();
        String registrations = getAsJson("/registrations/user/3003");
        String expected = "{'registrations':["
                + "{'secondsToExpire':0,'status':'expired','primary':'192.168.0.26','uri':'sip:3003@example.org','identity':'3003@example.org','contact':'\"John Doe\"<sip:jane.doe@example.org>','expires':1299762969,'instrument':'0004f2a9b633','regCallId':'3f404b64-fc8490c3-6b14ac9a@192.168.2.20'},"
                + "]}";
        JSONAssert.assertEquals(expected, registrations, false);

    }

    public void testGetRegistrationsByMac() throws Exception {
        disableDaoEventPublishing();
        clear();
        Phone p = m_phoneContext.newPhone(new TestPhoneModel());
        p.setDescription("unittest-sample phone1");
        p.setSerialNumber("0004f2a9b633");
        m_phoneContext.storePhone(p);
        commit();
        String registrations = getAsJson("/registrations/serialNo/0004f2a9b633");
        String expected = "{'registrations':["
                + "{'secondsToExpire':0,'status':'expired','primary':'192.168.0.26','uri':'sip:3003@example.org','identity':'3003@example.org','contact':'\"John Doe\"<sip:jane.doe@example.org>','expires':1299762969,'instrument':'0004f2a9b633','regCallId':'3f404b64-fc8490c3-6b14ac9a@192.168.2.20'},"
                + "]}";
        JSONAssert.assertEquals(expected, registrations, false);

    }

    public void testGetRegistrationsByIp() throws Exception {
        String registrations = getAsJson("/registrations/ip/192.168.2.20");
        String expected = "{'registrations':["
                + "{'secondsToExpire':0,'status':'expired','primary':'192.168.0.26','uri':'sip:3003@example.org','identity':'3003@example.org','contact':'\"John Doe\"<sip:jane.doe@example.org>','expires':1299762969,'instrument':'0004f2a9b633','regCallId':'3f404b64-fc8490c3-6b14ac9a@192.168.2.20'},"
                + "]}";
        JSONAssert.assertEquals(expected, registrations, false);

    }

    public void testGetRegistrationsByCallId() throws Exception {
        String registrations = getAsJson("/registrations/callid/3f404b64-fc8490c3-6b14ac9a@192.168.2.20");
        String expected = "{'registrations':["
                + "{'secondsToExpire':0,'status':'expired','primary':'192.168.0.26','uri':'sip:3003@example.org','identity':'3003@example.org','contact':'\"John Doe\"<sip:jane.doe@example.org>','expires':1299762969,'instrument':'0004f2a9b633','regCallId':'3f404b64-fc8490c3-6b14ac9a@192.168.2.20'},"
                + "]}";
        JSONAssert.assertEquals(expected, registrations, false);
    }

    public void testGetRegistrationsByServer() throws Exception {
        loadDataSetXml("commserver/seedLocations.xml");
        commit();
        String registrations = getAsJson("/registrations/server/101");
        String expected = "{'registrations':["
                + "{'secondsToExpire':0,'status':'expired','primary':'192.168.0.26','uri':'sip:3003@example.org','identity':'3003@example.org','contact':'\"John Doe\"<sip:jane.doe@example.org>','expires':1299762969,'instrument':'0004f2a9b633','regCallId':'3f404b64-fc8490c3-6b14ac9a@192.168.2.20'},"
                + "]}";
        JSONAssert.assertEquals(expected, registrations, false);
    }

    public void testDropRegistrationsByUser() throws Exception {
        disableDaoEventPublishing();
        clear();
        User u = m_coreContext.newUser();
        u.setUserName("3003");
        m_coreContext.saveUser(u);
        commit();
        int code = delete("/registrations/user/3003");
        assertEquals(200, code);
        String registrations = getAsJson("/registrations/user/3003");
        String expected = "{'registrations':[]}";
        JSONAssert.assertEquals(expected, registrations, false);

    }

    public void testDropRegistrationsByServer() throws Exception {
        loadDataSetXml("commserver/seedLocations.xml");
        commit();
        int code = delete("/registrations/server/101");
        assertEquals(200, code);
        String registrations = getAsJson("/registrations/server/101");
        String expected = "{'registrations':[]}";
        JSONAssert.assertEquals(expected, registrations, false);
    }

    public void testDropRegistrationsByCallId() throws Exception {
        int code = delete("/registrations/callid/3f404b64-fc8490c3-6b14ac9a@192.168.2.20");
        assertEquals(200, code);
        String registrations = getAsJson("/registrations/callid/3f404b64-fc8490c3-6b14ac9a@192.168.2.20");
        String expected = "{'registrations':[]}";
        JSONAssert.assertEquals(expected, registrations, false);
    }

    public void testDropRegistrationsByIp() throws Exception {
        int code = delete("/registrations/ip/192.168.2.20");
        assertEquals(200, code);
        String registrations = getAsJson("/registrations/ip/192.168.2.20");
        String expected = "{'registrations':[]}";
        JSONAssert.assertEquals(expected, registrations, false);

    }

    public void testDropRegistrationsByMac() throws Exception {
        disableDaoEventPublishing();
        clear();
        Phone p = m_phoneContext.newPhone(new TestPhoneModel());
        p.setDescription("unittest-sample phone1");
        p.setSerialNumber("0004f2a9b633");
        m_phoneContext.storePhone(p);
        commit();
        int code = delete("/registrations/serialNo/0004f2a9b633");
        assertEquals(200, code);
        String registrations = getAsJson("/registrations/serialNo/0004f2a9b633");
        String expected = "{'registrations':[]}";
        JSONAssert.assertEquals(expected, registrations, false);

    }

    public void setNodeDb(MongoTemplate nodeDb) {
        m_nodeDb = nodeDb;
    }

    public void setCoreContext(CoreContext coreContext) {
        m_coreContext = coreContext;
    }

    public void setPhoneContext(PhoneContext phoneContext) {
        m_phoneContext = phoneContext;
    }
}
