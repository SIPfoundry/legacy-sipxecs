/**
 * Copyright (c) 2014 eZuce, Inc. All rights reserved.
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
package org.sipfoundry.sipxconfig.api;

import org.junit.Test;
import org.sipfoundry.sipxconfig.phone.Phone;
import org.sipfoundry.sipxconfig.phone.PhoneContext;
import org.sipfoundry.sipxconfig.setting.Group;
import org.sipfoundry.sipxconfig.setting.SettingDao;
import org.sipfoundry.sipxconfig.test.RestApiIntegrationTestCase;

public class PhoneApiTestIntegration extends RestApiIntegrationTestCase {
    private PhoneContext m_phoneContext;
    private SettingDao m_settingDao;

    @Override
    protected void onSetUpBeforeTransaction() throws Exception {
        super.onSetUpBeforeTransaction();
        clear();
    }

    @Test
    public void testPhoneJsonApi() throws Exception {
        // query empty phones
        String emptyPhones = getAsJson("phones");
        assertEquals("{\"phones\":[]}", emptyPhones);

        // create phone
        String createPhone = "{\"description\":\"TestPhone\",\"model\":{\"modelId\":\"acmePhoneStandard\"},"
                + "\"groups\":null,\"lines\":null,\"serialNo\":\"0004f2ae4838\",\"deviceVersion\":\"acmePhone1\"}";
        int code = postJsonString(createPhone, "phones");
        assertEquals(200, code);
        assertEquals(1, m_phoneContext.getPhonesCount());

        Phone phone = m_phoneContext.getPhoneBySerialNumber("0004f2ae4838");

        // retrieve phones
        String phones = getAsJson("phones");
        assertEquals(
                String.format(
                        "{\"phones\":"
                                + "[{\"id\":%s,\"serialNo\":\"0004f2ae4838\",\"deviceVersion\":\"acmePhone1\",\"description\":\"TestPhone\","
                                + "\"model\":{\"modelId\":\"acmePhoneStandard\",\"label\":\"Acme\",\"vendor\":null,\"versions\":[\"acmePhone1\",\"acmePhone2\"]},"
                                + "\"lines\":null,\"groups\":null}]}", phone.getId()), phones);

        // retrieve phone
        String phoneJson = getAsJson("phones/0004f2ae4838");
        assertEquals(
                String.format(
                        "{\"id\":%s,\"serialNo\":\"0004f2ae4838\",\"deviceVersion\":\"acmePhone1\",\"description\":\"TestPhone\","
                                + "\"model\":{\"modelId\":\"acmePhoneStandard\",\"label\":\"Acme\",\"vendor\":null,\"versions\":[\"acmePhone1\",\"acmePhone2\"]},"
                                + "\"lines\":null,\"groups\":null}", phone.getId()), phoneJson);

        // modify phone MAC, description and version. Add phone in phone group
        String modifyPhone = "{\"description\":\"TestPhone Modified\",\"model\":{\"modelId\":\"acmePhoneStandard\"},"
                + "\"groups\":[{\"name\":\"testPhoneGroup\"}],\"lines\":null,\"serialNo\":\"0004f2ae4888\",\"deviceVersion\":\"acmePhone2\"}";
        int putCode = putJsonString(modifyPhone, "phones/0004f2ae4838");
        assertEquals(200, putCode);

        // check phone with groups
        String modifiedPhone = getAsJson("phones/0004f2ae4888");
        Group group = m_settingDao.getGroupByName("phone", "testPhoneGroup");
        assertEquals(
                String.format(
                        "{\"id\":%s,\"serialNo\":\"0004f2ae4888\",\"deviceVersion\":\"acmePhone2\","
                                + "\"description\":\"TestPhone Modified\",\"model\":{\"modelId\":\"acmePhoneStandard\",\"label\":\"Acme\","
                                + "\"vendor\":null,\"versions\":[\"acmePhone1\",\"acmePhone2\"]},\"lines\":null,\"groups\":[{\"id\":%s,\"name\":\"testPhoneGroup\",\"weight\":%s}]}",
                        phone.getId(), group.getId(), group.getWeight()), modifiedPhone);

        // retrieve phone groups
        String groups = getAsJson("phones/0004f2ae4888/groups");
        assertEquals(String.format("{\"groups\":[{\"id\":%s,\"name\":\"testPhoneGroup\",\"weight\":%s}]}",
                group.getId(), group.getWeight()), groups);

        // remove phone from group
        int deleteStatus = delete("phones/0004f2ae4888/groups/testPhoneGroup");
        assertEquals(200, deleteStatus);
        String newGroups = getAsJson("phones/0004f2ae4888/groups");
        assertEquals("{\"groups\":[]}", newGroups);

        // get setting
        String setting = getAsJson("phones/0004f2ae4888/settings/server/outboundProxy");
        assertEquals(
                "{\"path\":\"server/outboundProxy\",\"type\":\"string\",\"options\":null,\"value\":null,\"defaultValue\":null,\"label\":null,\"description\":null}",
                setting);

        // modify setting
        int settingCode = putPlainText("test.org", "phones/0004f2ae4888/settings/server/outboundProxy");
        assertEquals(200, settingCode);
        String modifiedSetting = getAsJson("phones/0004f2ae4888/settings/server/outboundProxy");
        assertEquals(
                "{\"path\":\"server/outboundProxy\",\"type\":\"string\",\"options\":null,\"value\":\"test.org\",\"defaultValue\":null,\"label\":null,\"description\":null}",
                modifiedSetting);

        // reset setting
        int resetCode = delete("phones/0004f2ae4888/settings/server/outboundProxy");
        assertEquals(200, resetCode);

        // delete phone
        int deletePhone = delete("phones/0004f2ae4888");
        assertEquals(200, deletePhone);
        assertEquals(0, m_phoneContext.getPhonesCount());
    }

    @Test
    public void testPhoneXmlApi() throws Exception {
        // query empty phones
        String emptyPhones = getAsXml("phones");
        assertEquals("<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?><Phones/>", emptyPhones);

        // create phone
        String createPhone = "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>"
                + "<Phone><serialNo>0004f2ae4838</serialNo><deviceVersion>acmePhone1</deviceVersion><description>TestPhone</description>"
                + "<Model><modelId>acmePhoneStandard</modelId></Model></Phone>";
        int code = postXmlString(createPhone, "phones");
        assertEquals(200, code);
        assertEquals(1, m_phoneContext.getPhonesCount());

        Phone phone = m_phoneContext.getPhoneBySerialNumber("0004f2ae4838");

        // retrieve phones
        String phones = getAsXml("phones");
        assertEquals(
                String.format(
                        "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>"
                                + "<Phones><Phone><id>%s</id><serialNo>0004f2ae4838</serialNo>"
                                + "<deviceVersion>acmePhone1</deviceVersion><description>TestPhone</description>"
                                + "<Model><modelId>acmePhoneStandard</modelId><label>Acme</label><Versions><Version>acmePhone1</Version><Version>acmePhone2</Version></Versions></Model>"
                                + "</Phone></Phones>", phone.getId()), phones);

        // retrieve phone
        String phoneXml = getAsXml("phones/0004f2ae4838");
        assertEquals(
                String.format(
                        "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>"
                                + "<Phone><id>%s</id><serialNo>0004f2ae4838</serialNo><deviceVersion>acmePhone1</deviceVersion><description>TestPhone</description>"
                                + "<Model><modelId>acmePhoneStandard</modelId><label>Acme</label><Versions><Version>acmePhone1</Version><Version>acmePhone2</Version></Versions></Model>"
                                + "</Phone>", phone.getId()), phoneXml);

        // modify phone MAC, description and version. Add phone in phone group
        String modifyPhone = "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>"
                + "<Phone><serialNo>0004f2ae4888</serialNo><deviceVersion>acmePhone2</deviceVersion><description>TestPhone Modified</description>"
                + "<Model><modelId>acmePhoneStandard</modelId></Model>"
                + "<Groups><Group><name>testPhoneGroup</name></Group></Groups>" + "</Phone>";
        int putCode = putXmlString(modifyPhone, "phones/0004f2ae4838");
        assertEquals(200, putCode);

        // check phone with groups
        String modifiedPhone = getAsXml("phones/0004f2ae4888");
        Group group = m_settingDao.getGroupByName("phone", "testPhoneGroup");
        assertEquals(
                String.format(
                        "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>"
                                + "<Phone><id>%s</id><serialNo>0004f2ae4888</serialNo><deviceVersion>acmePhone2</deviceVersion><description>TestPhone Modified</description>"
                                + "<Model><modelId>acmePhoneStandard</modelId><label>Acme</label><Versions><Version>acmePhone1</Version><Version>acmePhone2</Version></Versions></Model>"
                                + "<Groups><Group><id>%s</id><name>testPhoneGroup</name><weight>%s</weight></Group></Groups></Phone>",
                        phone.getId(), group.getId(), group.getWeight()), modifiedPhone);

        // retrieve phone groups
        String groups = getAsXml("phones/0004f2ae4888/groups");
        assertEquals(String.format("<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>"
                + "<Groups><Group><id>%s</id><name>testPhoneGroup</name><weight>%s</weight></Group></Groups>",
                group.getId(), group.getWeight()), groups);

        // remove phone from group
        int deleteStatus = delete("phones/0004f2ae4888/groups/testPhoneGroup");
        assertEquals(200, deleteStatus);
        String newGroups = getAsXml("phones/0004f2ae4888/groups");
        assertEquals("<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?><Groups/>", newGroups);

        // get setting
        String setting = getAsXml("phones/0004f2ae4888/settings/server/outboundProxy");
        assertEquals(
                "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?><Setting><path>server/outboundProxy</path><type>string</type></Setting>",
                setting);

        // modify setting
        int settingCode = putPlainText("test.org", "phones/0004f2ae4888/settings/server/outboundProxy");
        assertEquals(200, settingCode);
        String modifiedSetting = getAsXml("phones/0004f2ae4888/settings/server/outboundProxy");
        assertEquals(
                "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?><Setting><path>server/outboundProxy</path><type>string</type><value>test.org</value></Setting>",
                modifiedSetting);

        // reset setting
        int resetCode = delete("phones/0004f2ae4888/settings/server/outboundProxy");
        assertEquals(200, resetCode);

        // delete phone
        int deletePhone = delete("phones/0004f2ae4888");
        assertEquals(200, deletePhone);
        assertEquals(0, m_phoneContext.getPhonesCount());
    }

    public void setPhoneContext(PhoneContext context) {
        m_phoneContext = context;
    }

    public void setSettingDao(SettingDao settingDao) {
        m_settingDao = settingDao;
    }

}
