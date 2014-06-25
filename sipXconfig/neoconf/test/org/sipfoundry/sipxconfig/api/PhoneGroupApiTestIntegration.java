package org.sipfoundry.sipxconfig.api;

import org.junit.Test;
import org.sipfoundry.sipxconfig.phone.PhoneContext;
import org.sipfoundry.sipxconfig.setting.Group;
import org.sipfoundry.sipxconfig.test.RestApiIntegrationTestCase;

public class PhoneGroupApiTestIntegration extends RestApiIntegrationTestCase {
    private PhoneContext m_phoneContext;

    @Override
    protected void onSetUpBeforeTransaction() throws Exception {
        super.onSetUpBeforeTransaction();
        clear();
    }

    @Test
    public void testPhoneGroupJsonApi() throws Exception {
        // query empty phone groups
        String emptyGroups = getAsJson("phoneGroups");
        assertEquals("{\"groups\":[]}", emptyGroups);

        // create phone group
        String createGroup = "{\"name\":\"wwwwwwww5\",\"description\":\"ewe\",\"weight\":1,\"count\":null}";
        int code = postJsonString(createGroup, "phoneGroups");
        assertEquals(200, code);
        assertEquals(1, m_phoneContext.getGroups().size());

        Group phoneGroup = m_phoneContext.getGroupByName("wwwwwwww5", false);

        // retrieve phone groups
        String phoneGroups = getAsJson("phoneGroups");
        assertEquals(
                String.format(
                        "{\"groups\":[{\"id\":%s,\"name\":\"wwwwwwww5\",\"description\":\"ewe\",\"weight\":1,\"count\":null}]}", phoneGroup.getId()), phoneGroups);

        // retrieve phone group
        String phoneGroupJson = getAsJson("phoneGroups/wwwwwwww5");
        assertEquals(
                String.format(
                        "{\"id\":%s,\"name\":\"wwwwwwww5\",\"description\":\"ewe\",\"weight\":1,\"count\":null}", phoneGroup.getId()), phoneGroupJson);

        // modify phone group
        String modifyPhoneGroup = "{\"name\":\"wwwwwwww5\",\"description\":\"ewe-modified\",\"weight\":1,\"count\":null}";
        int putCode = putJsonString(modifyPhoneGroup, "phoneGroups/wwwwwwww5");
        assertEquals(200, putCode);

        //retrieve modified phone group
        phoneGroupJson = getAsJson("phoneGroups/wwwwwwww5");
        assertEquals(
                String.format(
                        "{\"id\":%s,\"name\":\"wwwwwwww5\",\"description\":\"ewe-modified\",\"weight\":1,\"count\":null}", phoneGroup.getId()), phoneGroupJson);


        // get setting
        String setting = getAsJson("phoneGroups/wwwwwwww5/model/testPhoneModel/settings/server/outboundProxy");
        assertEquals(
                "{\"path\":\"server/outboundProxy\",\"type\":\"string\",\"options\":null,\"value\":null,\"defaultValue\":null,\"label\":null,\"description\":null}",
                setting);

        // modify setting
        int settingCode = putPlainText("test.org", "phoneGroups/wwwwwwww5/model/testPhoneModel/settings/server/outboundProxy");
        assertEquals(200, settingCode);
        String modifiedSetting = getAsJson("phoneGroups/wwwwwwww5/model/testPhoneModel/settings/server/outboundProxy");
        assertEquals(
            "{\"path\":\"server/outboundProxy\",\"type\":\"string\",\"options\":null,\"value\":\"test.org\",\"defaultValue\":null,\"label\":null,\"description\":null}",
                modifiedSetting);

        // reset setting
        int resetCode = delete("phoneGroups/wwwwwwww5/model/testPhoneModel/settings/server/outboundProxy");
        assertEquals(200, resetCode);

        // delete phone group
        int deletePhone = delete("phoneGroups/wwwwwwww5");
        assertEquals(200, deletePhone);
        assertEquals(0, m_phoneContext.getPhonesCount());
    }

    @Test
    public void testPhoneGroupXmlApi() throws Exception {
        // query empty phone groups
        String emptyGroups = getAsXml("phoneGroups");
        assertEquals("<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?><Groups/>", emptyGroups);

        // create phone group
        String createGroup = "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>"
            + "<group><name>wwwwwwww6</name><description>ewe</description><weight>2</weight></group>";
        int code = postXmlString(createGroup, "phoneGroups");
        assertEquals(200, code);
        assertEquals(1, m_phoneContext.getGroups().size());

        Group phoneGroup = m_phoneContext.getGroupByName("wwwwwwww6", false);

        // retrieve phone groups
        String phoneGroups = getAsXml("phoneGroups");
        assertEquals(
                String.format(
                        "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>"
                        + "<Groups><Group><id>%s</id><name>wwwwwwww6</name><description>ewe</description><weight>2</weight></Group></Groups>", phoneGroup.getId()), phoneGroups);

        // retrieve phone group
        String phoneGroupXml = getAsXml("phoneGroups/wwwwwwww6");
        assertEquals(
                String.format(
                        "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>"
                        + "<group><id>%s</id><name>wwwwwwww6</name><description>ewe</description><weight>2</weight></group>", phoneGroup.getId()), phoneGroupXml);

        // modify phone group
        String modifyPhoneGroup = "<group><name>wwwwwwww6</name><description>ewe-modified</description><weight>2</weight></group>";
        int putCode = putXmlString(modifyPhoneGroup, "phoneGroups/wwwwwwww6");
        assertEquals(200, putCode);

        //retrieve modified phone group
        phoneGroupXml = getAsXml("phoneGroups/wwwwwwww6");
        assertEquals(
                String.format(
                        "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>"
                        + "<group><id>%s</id><name>wwwwwwww6</name><description>ewe-modified</description><weight>2</weight></group>", phoneGroup.getId()), phoneGroupXml);


        // get setting
        String setting = getAsXml("phoneGroups/wwwwwwww6/model/testPhoneModel/settings/server/outboundProxy");
        assertEquals(
                "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?><Setting><path>server/outboundProxy</path><type>string</type></Setting>",
                setting);

        // modify setting
        int settingCode = putPlainText("test.org", "phoneGroups/wwwwwwww6/model/testPhoneModel/settings/server/outboundProxy");
        assertEquals(200, settingCode);
        String modifiedSetting = getAsXml("phoneGroups/wwwwwwww6/model/testPhoneModel/settings/server/outboundProxy");
        assertEquals(
            "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?><Setting><path>server/outboundProxy</path><type>string</type><value>test.org</value></Setting>",
                modifiedSetting);

        // reset setting
        int resetCode = delete("phoneGroups/wwwwwwww6/model/testPhoneModel/settings/server/outboundProxy");
        assertEquals(200, resetCode);

        // delete phone group
        int deletePhone = delete("phoneGroups/wwwwwwww6");
        assertEquals(200, deletePhone);
        assertEquals(0, m_phoneContext.getPhonesCount());
    }

    public void setPhoneContext(PhoneContext phoneContext) {
        m_phoneContext = phoneContext;
    }
}
