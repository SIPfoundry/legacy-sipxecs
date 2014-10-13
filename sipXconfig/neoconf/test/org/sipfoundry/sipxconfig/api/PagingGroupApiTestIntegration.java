package org.sipfoundry.sipxconfig.api;

import org.sipfoundry.sipxconfig.test.RestApiIntegrationTestCase;
import org.skyscreamer.jsonassert.JSONAssert;

public class PagingGroupApiTestIntegration extends RestApiIntegrationTestCase {
    @Override
    protected void onSetUpBeforeTransaction() throws Exception {
        super.onSetUpBeforeTransaction();
        db().execute("select truncate_all()");
        sql("paging/paging.sql");
    }

    @Override
    protected void onSetUpInTransaction() throws Exception {
        super.onSetUpInTransaction();

    }

    public void testGetPagingGroupJson() throws Exception {
        String groups = getAsJson("/pagegroups");
        String expctedGroups = "{'groups':  "
                + "[{'enabled': true, 'id': 100, 'description': 'Sales', 'timeout': 60, users: [{'aliases': [], 'id': 1001,'userName': 'user1','firstName': null,'lastName': null},{'aliases': [], 'id': 1002,'userName': 'user2','firstName': null,'lastName': null}],'sound': 'TadaTada.wav','pageGroupNumber': 111},"
                + "{'enabled': false, 'id': 101, 'description': 'Engineering', 'timeout': 60, users: [{'aliases': [], 'id': 1002,'userName': 'user2','firstName': null,'lastName': null}], 'sound': 'TadaTada.wav', 'pageGroupNumber': 112},"
                + "{'enabled': true, 'id': 102, 'description': 'Support', 'timeout': 600, users: [{'aliases': [], 'id': 1001,'userName': 'user1','firstName': null,'lastName': null},{'aliases': [], 'id': 1002,'userName': 'user2','firstName': null,'lastName': null},{'aliases': [], 'id': 1003,'userName': 'user3','firstName': null,'lastName': null}], 'sound': 'fanfare.wav', 'pageGroupNumber': 113}"
                + "]}";
        JSONAssert.assertEquals(expctedGroups, groups, false);
    }

    public void testNewPageGroupJson() throws Exception  {
        String newGroup = "{\"enabled\": true, \"description\": \"Sales1\", \"timeout\": 70, \"users\": [{\"aliases\": [], \"id\": 1001,\"userName\": \"user1\",\"firstName\": null,\"lastName\": null}],\"sound\": \"fanfare.wav\", \"pageGroupNumber\": 114}";
        int code = postJsonString(newGroup, "/pagegroups");
        assertEquals(200, code);
        String groups = getAsJson("/pagegroups");
        String expctedGroups = "{'groups':  "
                + "[{'enabled': true, 'id': 100, 'description': 'Sales', 'timeout': 60, users: [{'aliases': [], 'id': 1001,'userName': 'user1','firstName': null,'lastName': null},{'aliases': [], 'id': 1002,'userName': 'user2','firstName': null,'lastName': null}],'sound': 'TadaTada.wav','pageGroupNumber': 111},"
                + "{'enabled': false, 'id': 101, 'description': 'Engineering', 'timeout': 60, users: [{'aliases': [], 'id': 1002,'userName': 'user2','firstName': null,'lastName': null}], 'sound': 'TadaTada.wav', 'pageGroupNumber': 112},"
                + "{'enabled': true, 'id': 102, 'description': 'Support', 'timeout': 600, users: [{'aliases': [], 'id': 1001,'userName': 'user1','firstName': null,'lastName': null},{'aliases': [], 'id': 1002,'userName': 'user2','firstName': null,'lastName': null},{'aliases': [], 'id': 1003,'userName': 'user3','firstName': null,'lastName': null}], 'sound': 'fanfare.wav', 'pageGroupNumber': 113},"
                + "{\"enabled\": true, \"description\": \"Sales1\", \"timeout\": 70, \"users\": [{\"aliases\": [], \"id\": 1001,\"userName\": \"user1\",\"firstName\": null,\"lastName\": null}],\"sound\": \"fanfare.wav\", \"pageGroupNumber\": 114}"
                + "]}";
        JSONAssert.assertEquals(expctedGroups, groups, false);
    }

    public void testPageGroupJson() throws Exception {
        String group = getAsJson("/pagegroups/100");
        String expectedgroup = "{\"enabled\": true, \"description\": \"Sales\", \"timeout\": 60, \"users\": [{\"aliases\": [], \"id\": 1001,\"userName\": \"user1\",\"firstName\": null,\"lastName\": null},{'aliases': [], 'id': 1002,'userName': 'user2','firstName': null,'lastName': null}],\"sound\": \"TadaTada.wav\", \"pageGroupNumber\": 111}";

        JSONAssert.assertEquals(expectedgroup, group, false);
    }

    public void testUpdatePageGroupJson() throws Exception {
        String group = "{\"enabled\": true, \"description\": \"Sales1\", \"timeout\": 70, \"users\": [{\"aliases\": [], \"id\": 1001,\"userName\": \"user1\",\"firstName\": null,\"lastName\": null},{\"aliases\": [], \"id\": 1003,\"userName\": \"user3\",\"firstName\": null,\"lastName\": null}],\"sound\": \"fanfare.wav\", \"pageGroupNumber\": 117}";
        int code = putJsonString(group, "/pagegroups/100");
        String expectedGroup = "{\"id\":100,\"enabled\": true, \"description\": \"Sales1\", \"timeout\": 70, \"users\": [{\"aliases\": [], \"id\": 1001,\"userName\": \"user1\",\"firstName\": null,\"lastName\": null},{\"aliases\": [], \"id\": 1003,\"userName\": \"user3\",\"firstName\": null,\"lastName\": null}],\"sound\": \"fanfare.wav\", \"pageGroupNumber\": 117}";
        assertEquals(200, code);
        JSONAssert.assertEquals(expectedGroup, getAsJson("/pagegroups/100"), false);
    }

    public void testGetPagingGroupXml() throws Exception {
        String groups = getAsXml("/pagegroups");
        String expctedGroups = "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>"
        +"<Groups>"
        + "<Group>"
        +"<id>100</id>"
        +"<enabled>true</enabled>"
        +"<pageGroupNumber>111</pageGroupNumber>"
        + "<timeout>60</timeout>"
        +"<sound>TadaTada.wav</sound>"
        + "<description>Sales</description>"
        + "<Users>"
        + "<User>"
        + "<id>1001</id>"
        + "<userName>user1</userName>"
        + "<Aliases/>"
        + "</User>"
        + "<User>"
        + "<id>1002</id>"
        + "<userName>user2</userName>"
        + "<Aliases/>"
        + "</User>"
        + "</Users>"
        + "</Group>"
        + "<Group>"
        + "<id>101</id>"
        + "<enabled>false</enabled>"
        +"<pageGroupNumber>112</pageGroupNumber>"
        + "<timeout>60</timeout>"
        +"<sound>TadaTada.wav</sound>"
        + "<description>Engineering</description>"
        + "<Users>"
        + "<User>"
        + "<id>1002</id>"
        + "<userName>user2</userName>"
        + "<Aliases/>"
        + "</User>"
        + "</Users>"
        + "</Group>"
        + "<Group>"
        + "<id>102</id>"
        + "<enabled>true</enabled>"
        +"<pageGroupNumber>113</pageGroupNumber>"
        + "<timeout>600</timeout>"
        +"<sound>fanfare.wav</sound>"
        + "<description>Support</description>"
        + "<Users>"
        + "<User>"
        + "<id>1001</id>"
        + "<userName>user1</userName>"
        + "<Aliases/>"
        + "</User>"
        + "<User>"
        + "<id>1003</id>"
        + "<userName>user3</userName>"
        + "<Aliases/>"
        + "</User>"
        + "<User>"
        + "<id>1002</id>"
        + "<userName>user2</userName>"
        + "<Aliases/>"
        + "</User>"
        + "</Users>"
        + "</Group>"
        +"</Groups>";

        assertEquals(expctedGroups, groups);
    }

    public void testNewPageGroupXml() throws Exception  {
        String newGroup = "<Group>"
                        +"<enabled>true</enabled>"
                        +"<pageGroupNumber>115</pageGroupNumber>"
                        + "<timeout>700</timeout>"
                        +"<sound>TadaTada.wav</sound>"
                        + "<description>Sales</description>"
                        + "<Users>"
                        + "<User>"
                        + "<id>1001</id>"
                        + "<userName>user1</userName>"
                        + "<Aliases/>"
                        + "</User>"
                        + "<User>"
                        + "<id>1002</id>"
                        + "<userName>user2</userName>"
                        + "<Aliases/>"
                        + "</User>"
                        + "</Users>"
                        + "</Group>";
        int code = postXmlString(newGroup, "/pagegroups");
        assertEquals(200, code);
    }

    public void testPageGroupXml() throws Exception {
        String group = getAsXml("/pagegroups/100");
        String expectedGroup = "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?><Group>"
                +"<id>100</id>"
                +"<enabled>true</enabled>"
                +"<pageGroupNumber>111</pageGroupNumber>"
                + "<timeout>60</timeout>"
                +"<sound>TadaTada.wav</sound>"
                + "<description>Sales</description>"
                + "<Users>"
                + "<User>"
                + "<id>1001</id>"
                + "<userName>user1</userName>"
                + "<Aliases/>"
                + "</User>"
                + "<User>"
                + "<id>1002</id>"
                + "<userName>user2</userName>"
                + "<Aliases/>"
                + "</User>"
                + "</Users>"
                + "</Group>";
        assertEquals(expectedGroup, group);
    }

    public void testUpdatePageGroupXml() throws Exception {
        String group = "<Group>"
                +"<enabled>false</enabled>"
                +"<pageGroupNumber>117</pageGroupNumber>"
                + "<timeout>70</timeout>"
                +"<sound>fanfare.wav</sound>"
                + "<description>Sales1</description>"
                + "<Users>"
                + "<User>"
                + "<id>1003</id>"
                + "<userName>user3</userName>"
                + "<Aliases/>"
                + "</User>"
                + "</Users>"
                + "</Group>";
        int code = putXmlString(group, "/pagegroups/100");
        assertEquals(200, code);

        String expectedGroup = "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?><Group>"
                +"<id>100</id>"
                +"<enabled>false</enabled>"
                +"<pageGroupNumber>117</pageGroupNumber>"
                + "<timeout>70</timeout>"
                +"<sound>fanfare.wav</sound>"
                + "<description>Sales1</description>"
                + "<Users>"
                + "<User>"
                + "<id>1003</id>"
                + "<userName>user3</userName>"
                + "<Aliases/>"
                + "</User>"
                + "</Users>"
                + "</Group>";
        String groupUpdated = getAsXml("/pagegroups/100");
        assertEquals(expectedGroup, groupUpdated);
    }
}
