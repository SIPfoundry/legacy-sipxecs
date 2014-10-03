package org.sipfoundry.sipxconfig.api;

import org.codehaus.jackson.JsonNode;
import org.codehaus.jackson.map.ObjectMapper;
import org.junit.Test;
import org.sipfoundry.sipxconfig.phone.PhoneContext;
import org.sipfoundry.sipxconfig.test.RestApiIntegrationTestCase;
import org.sipfoundry.sipxconfig.test.TestHelper;

public class PhoneLineApiTestIntegration extends RestApiIntegrationTestCase {
    private PhoneContext m_phoneContext;

    @Override
    protected void onSetUpBeforeTransaction() throws Exception {
        super.onSetUpBeforeTransaction();
        clear();
    }

    @Test
    public void testJson() throws Exception {
        TestHelper.cleanInsert("ClearDb.xml");
        clear();
        loadDataSet("api/phoneSeed.xml");
        loadDataSet("common/TestUserSeed.db.xml");
        assertEquals(1, m_phoneContext.getPhonesCount());
        commit();

        String addLine = "{\"uri\": \"sip:1000@ezuce.ro\"," + "\"user\": \"testuser\"," + "\"userId\": \"1000\","
                + "\"extension\": null," + "\"displayName\": null," + "\"password\": \"1234\","
                + "\"registrationServer\": \"ezuce.ro\"," + "\"registrationServerPort\": \"0\","
                + "\"voicemail\": null}";
        // test getPhoneLines
        String lines = getAsJson("phones/1000/lines");
        assertEquals("{\"lines\":[]}", lines);

        // test newLine
        int code = postJsonString(addLine, "phones/1000/lines");
        assertEquals(200, code);

        // test getPhoneLines
        lines = getAsJson("phones/1000/lines");
        int lineId = m_phoneContext.getPhoneBySerialNumber("000000000000").getLine(0).getId();
        assertEquals(
                "{\"lines\":[{\"id\":"
                        + lineId
                        + ",\"uri\":\"\\\"Test User\\\"<sip:testuser@example.org>\",\"user\":\"testuser\",\"userId\":\"testuser\",\"extension\":null,\"displayName\":\"Test User\",\"password\":\"1234\",\"registrationServer\":\"example.org\",\"registrationServerPort\":null,\"voicemail\":null}]}",
                lines);

        // test getPhoneLine
        String line = getAsJson("phones/1000/lines/" + lineId);
        assertEquals(
                "{\"id\":"
                        + lineId
                        + ",\"uri\":\"\\\"Test User\\\"<sip:testuser@example.org>\",\"user\":\"testuser\",\"userId\":\"testuser\",\"extension\":null,\"displayName\":\"Test User\",\"password\":\"1234\",\"registrationServer\":\"example.org\",\"registrationServerPort\":null,\"voicemail\":null}",
                line);
        // test line settings
        putPlainText("AAA BBB", "phones/1000/lines/" + lineId + "/settings/credential/displayName");

        String transport = getAsJson("phones/1000/lines/" + lineId + "/settings/credential/displayName");
        ObjectMapper mapper = new ObjectMapper();
        JsonNode node = mapper.readTree(transport);
        assertEquals("\"AAA BBB\"", node.get("value").toString());

        int resetCode = delete("phones/1000/lines/" + lineId + "/settings/credential/displayName");
        assertEquals(200, resetCode);
        transport = getAsJson("phones/1000/lines/" + lineId + "/settings/credential/displayName");
        node = mapper.readTree(transport);
        assertEquals("\"Test User\"", node.get("value").toString());

        // test delete line; we can add the same line twice
        code = postJsonString(addLine, "phones/1000/lines");
        assertEquals(200, code);

        int secondLineId = m_phoneContext.getPhoneBySerialNumber("000000000000").getLine(1).getId();
        String secondLine = getAsJson("phones/1000/lines/" + secondLineId);
        assertEquals(
                "{\"id\":"
                        + secondLineId
                        + ",\"uri\":\"\\\"Test User\\\"<sip:testuser@example.org>\",\"user\":\"testuser\",\"userId\":\"testuser\",\"extension\":null,\"displayName\":\"Test User\",\"password\":\"1234\",\"registrationServer\":\"example.org\",\"registrationServerPort\":null,\"voicemail\":null}",
                secondLine);
        assertEquals(2, m_phoneContext.getPhoneBySerialNumber("000000000000").getLines().size());
        int deleteCode = delete("phones/1000/lines/" + lineId);
        assertEquals(200, deleteCode);
        assertEquals(1, m_phoneContext.getPhoneBySerialNumber("000000000000").getLines().size());

        deleteCode = delete("phones/1000/lines");
        assertEquals(200, deleteCode);
        assertEquals(0, m_phoneContext.getPhoneBySerialNumber("000000000000").getLines().size());
    }

    public void testXml() throws Exception {
        TestHelper.cleanInsert("ClearDb.xml");
        clear();
        loadDataSet("api/phoneSeed.xml");
        loadDataSet("common/TestUserSeed.db.xml");
        assertEquals(1, m_phoneContext.getPhonesCount());
        commit();

        assertEquals("<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?><Lines/>",
                getAsXml("phones/1000/lines"));

        String addLine = "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>" + "<Line>"
                + "<uri>sip:1000@ezuce.ro</uri>" + "<user>testuser</user>" + "<userId>1000</userId>"
                + "<password>1234</password>" + "<registrationServer>ezuce.ro</registrationServer>"
                + "<registrationServerPort>0</registrationServerPort>" + " </Line>";
        int code = postXmlString(addLine, "phones/1000/lines");
        assertEquals(200, code);

        String lines = getAsXml("phones/1000/lines");

        int lineId = m_phoneContext.getPhoneBySerialNumber("000000000000").getLine(0).getId();

        String line = "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?><Lines>" + "<Line><id>" + lineId
                + "</id>" + "<uri>\"Test User\"&lt;sip:testuser@example.org&gt;</uri>" + "<user>testuser</user>"
                + "<userId>testuser</userId>" + "<displayName>Test User</displayName>" + "<password>1234</password>"
                + "<registrationServer>example.org</registrationServer>" + "</Line></Lines>";

        assertEquals(line, lines);

        code = postXmlString(addLine, "phones/1000/lines");
        assertEquals(200, code);

    }

    public void setPhoneContext(PhoneContext phoneContext) {
        m_phoneContext = phoneContext;
    }
}
