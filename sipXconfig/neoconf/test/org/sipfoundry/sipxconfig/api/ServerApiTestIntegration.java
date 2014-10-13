package org.sipfoundry.sipxconfig.api;

import org.sipfoundry.sipxconfig.commserver.LocationsManager;
import org.sipfoundry.sipxconfig.test.RestApiIntegrationTestCase;
import org.skyscreamer.jsonassert.JSONAssert;
import org.springframework.data.mongodb.core.MongoTemplate;

public class ServerApiTestIntegration extends RestApiIntegrationTestCase {
    private MongoTemplate m_imdb;
    LocationsManager m_locationsManager;

    @Override
    protected void onSetUpBeforeTransaction() throws Exception {
        super.onSetUpBeforeTransaction();
        clear();
        m_imdb.dropCollection("entity");

    }

    @Override
    protected void onTearDownAfterTransaction() throws Exception {
        super.onTearDownAfterTransaction();
        m_imdb.dropCollection("entity");
    }

    @Override
    protected void onSetUpInTransaction() throws Exception {
        super.onSetUpInTransaction();

    }

    public void testGetServers() throws Exception {
        loadDataSetXml("commserver/seedLocations.xml");
        flush();
        commit();
        String servers = getAsJson("/servers/");
        String expected = "{'servers':[{'registered':true,'id':101,'host':'localhost','description':'Config Server, Media Server and Comm Server','primary':true,'ip':'192.168.0.26'},{'registered':false,'id':102,'host':'remotehost.example.org','description':'Distributed Comm Server','primary':false,'ip':'192.168.0.27'}]}";
        JSONAssert.assertEquals(expected, servers, false);
    }

    public void testNewServer() throws Exception {
        disableDaoEventPublishing();
        loadDataSetXml("commserver/seedLocations.xml");
        flush();
        String newServer = "{\"host\":\"newserver1.example.org\",\"description\":\"Server description\",\"ip\":\"192.168.0.43\"}";
        int code = postJsonString(newServer, "/servers/");

        assertEquals(200, code);
        int id = m_locationsManager.getLocationByFqdn("newserver1.example.org").getId();
        commit();
        String expected = "{'servers':["
                + "{'registered':true,'id':101,'host':'localhost','description':'Config Server, Media Server and Comm Server','primary':true,'ip':'192.168.0.26'},"
                + "{'registered':false,'id':102,'host':'remotehost.example.org','description':'Distributed Comm Server','primary':false,'ip':'192.168.0.27'},"
                + "{'registered':false,'id':"
                + id
                + ",'host':'newserver1.example.org','description':'Server description','primary':false,'ip':'192.168.0.43'}"
                + "]}";
        String servers = getAsJson("/servers/");
        JSONAssert.assertEquals(expected, servers, false);

        String server = "{'registered':false,'id':"
                + id
                + ",'host':'newserver1.example.org','description':'Server description','primary':false,'ip':'192.168.0.43'}";
        JSONAssert.assertEquals(server, getAsJson("/servers/" + id), false);
        JSONAssert.assertEquals(server, getAsJson("/servers/newserver1.example.org"), false);

        String serverUpdated = "{\"registered\":false,\"id\":"
                + id
                + ",\"host\":\"newserver2.example.org\",\"description\":\"Server description2\",\"primary\":false,\"ip\":\"192.168.0.44\"}";
        int codeupd = putJsonString(serverUpdated, "/servers/" + id);
        assertEquals(200, codeupd);
        JSONAssert.assertEquals(serverUpdated, getAsJson("/servers/" + id), false);

        int codeDel = delete("/servers/" + id);
        assertEquals(200, codeDel);
        String serversAfterDel = getAsJson("/servers/");
        String expectedAfterDel = "{'servers':[{'registered':true,'id':101,'host':'localhost','description':'Config Server, Media Server and Comm Server','primary':true,'ip':'192.168.0.26'},{'registered':false,'id':102,'host':'remotehost.example.org','description':'Distributed Comm Server','primary':false,'ip':'192.168.0.27'}]}";
        JSONAssert.assertEquals(expectedAfterDel, serversAfterDel, false);
    }

    public void testFeaturesAndBundles() throws Exception {
        loadDataSetXml("commserver/seedLocations.xml");
        commit();
        String bundles = getAsJson("/servers/bundles/");
        String expectedBundles = "{\"bundles\":[{\"globalFeatures\":[\"sipxlogwatcher\",\"snmp\",\"mail\",\"firewall\",\"ntpd\",\"fail2ban\",\"alarms\"],\"locationFeatures\":[\"dhcpd\",\"sipxdns\",\"event\"],\"name\":\"core\"},{\"globalFeatures\":[\"intercom\"],\"locationFeatures\":[\"authCode\",\"freeSwitch\",\"redis\",\"ivr\",\"moh\",\"saa\",\"park\",\"sbcBridge\",\"registrar\",\"rls\",\"sipxcdr\",\"sipxsqa\",\"mwi\",\"page\",\"proxy\",\"restServer\",\"conference\",\"mysql\"],\"name\":\"coreTelephony\"},{\"globalFeatures\":[],\"locationFeatures\":[],\"name\":\"callCenter\"},{\"globalFeatures\":[],\"locationFeatures\":[\"imbot\"],\"name\":\"im\"},{\"globalFeatures\":[],\"locationFeatures\":[\"dhcpd\",\"phonelog\",\"ftp\",\"tftp\"],\"name\":\"provision\"},{\"globalFeatures\":[],\"locationFeatures\":[],\"name\":\"experimental\"}]}";
        JSONAssert.assertEquals(bundles, expectedBundles, false);

        String bundle = getAsJson("/servers/bundles/core");
        String expectedBundle = "{\"globalFeatures\":[\"sipxlogwatcher\",\"snmp\",\"mail\",\"firewall\",\"ntpd\",\"fail2ban\",\"alarms\"],\"locationFeatures\":[\"dhcpd\",\"sipxdns\",\"event\"],\"name\":\"core\"}";
        JSONAssert.assertEquals(bundle, expectedBundle, false);

        String features = getAsJson("/servers/features/");
        String expectedFeatures = "{\"features\":[{\"bundle\":\"core\",\"enabled\":false,\"type\":\"global\",\"name\":\"sipxlogwatcher\"},{\"bundle\":\"core\",\"enabled\":false,\"type\":\"global\",\"name\":\"snmp\"},{\"bundle\":\"core\",\"enabled\":false,\"type\":\"location\",\"name\":\"dhcpd\"},{\"bundle\":\"core\",\"enabled\":false,\"type\":\"global\",\"name\":\"mail\"},{\"bundle\":\"core\",\"enabled\":false,\"type\":\"global\",\"name\":\"firewall\"},{\"bundle\":\"core\",\"enabled\":false,\"type\":\"location\",\"name\":\"sipxdns\"},{\"bundle\":\"core\",\"enabled\":false,\"type\":\"location\",\"name\":\"event\"},{\"bundle\":\"core\",\"enabled\":false,\"type\":\"global\",\"name\":\"ntpd\"},{\"bundle\":\"core\",\"enabled\":false,\"type\":\"global\",\"name\":\"fail2ban\"},{\"bundle\":\"core\",\"enabled\":false,\"type\":\"global\",\"name\":\"alarms\"},{\"bundle\":\"coreTelephony\",\"enabled\":false,\"type\":\"location\",\"name\":\"authCode\"},{\"bundle\":\"coreTelephony\",\"enabled\":false,\"type\":\"location\",\"name\":\"freeSwitch\"},{\"bundle\":\"coreTelephony\",\"enabled\":false,\"type\":\"location\",\"name\":\"redis\"},{\"bundle\":\"coreTelephony\",\"enabled\":false,\"type\":\"location\",\"name\":\"ivr\"},{\"bundle\":\"coreTelephony\",\"enabled\":false,\"type\":\"location\",\"name\":\"moh\"},{\"bundle\":\"coreTelephony\",\"enabled\":false,\"type\":\"location\",\"name\":\"saa\"},{\"bundle\":\"coreTelephony\",\"enabled\":false,\"type\":\"location\",\"name\":\"park\"},{\"bundle\":\"coreTelephony\",\"enabled\":false,\"type\":\"location\",\"name\":\"sbcBridge\"},{\"bundle\":\"coreTelephony\",\"enabled\":false,\"type\":\"location\",\"name\":\"registrar\"},{\"bundle\":\"coreTelephony\",\"enabled\":false,\"type\":\"location\",\"name\":\"rls\"},{\"bundle\":\"coreTelephony\",\"enabled\":false,\"type\":\"location\",\"name\":\"sipxcdr\"},{\"bundle\":\"coreTelephony\",\"enabled\":false,\"type\":\"global\",\"name\":\"intercom\"},{\"bundle\":\"coreTelephony\",\"enabled\":false,\"type\":\"location\",\"name\":\"sipxsqa\"},{\"bundle\":\"coreTelephony\",\"enabled\":false,\"type\":\"location\",\"name\":\"mwi\"},{\"bundle\":\"coreTelephony\",\"enabled\":false,\"type\":\"location\",\"name\":\"page\"},{\"bundle\":\"coreTelephony\",\"enabled\":false,\"type\":\"location\",\"name\":\"proxy\"},{\"bundle\":\"coreTelephony\",\"enabled\":false,\"type\":\"location\",\"name\":\"restServer\"},{\"bundle\":\"coreTelephony\",\"enabled\":false,\"type\":\"location\",\"name\":\"conference\"},{\"bundle\":\"coreTelephony\",\"enabled\":false,\"type\":\"location\",\"name\":\"mysql\"},{\"bundle\":\"im\",\"enabled\":false,\"type\":\"location\",\"name\":\"imbot\"},{\"bundle\":\"provision\",\"enabled\":false,\"type\":\"location\",\"name\":\"dhcpd\"},{\"bundle\":\"provision\",\"enabled\":false,\"type\":\"location\",\"name\":\"phonelog\"},{\"bundle\":\"provision\",\"enabled\":false,\"type\":\"location\",\"name\":\"ftp\"},{\"bundle\":\"provision\",\"enabled\":false,\"type\":\"location\",\"name\":\"tftp\"}]}";
        JSONAssert.assertEquals(expectedFeatures, features, false);

        String serverFeatures = getAsJson("/servers/101/features/");
        String expectedServerFeatures = "{\"features\":[{\"bundle\":\"core\",\"enabled\":false,\"name\":\"dhcpd\",\"type\":\"location\"},{\"bundle\":\"core\",\"enabled\":false,\"name\":\"sipxdns\",\"type\":\"location\"},{\"bundle\":\"core\",\"enabled\":false,\"name\":\"event\",\"type\":\"location\"},{\"bundle\":\"coreTelephony\",\"enabled\":false,\"name\":\"authCode\",\"type\":\"location\"},{\"bundle\":\"coreTelephony\",\"enabled\":false,\"name\":\"freeSwitch\",\"type\":\"location\"},{\"bundle\":\"coreTelephony\",\"enabled\":false,\"name\":\"redis\",\"type\":\"location\"},{\"bundle\":\"coreTelephony\",\"enabled\":false,\"name\":\"ivr\",\"type\":\"location\"},{\"bundle\":\"coreTelephony\",\"enabled\":false,\"name\":\"moh\",\"type\":\"location\"},{\"bundle\":\"coreTelephony\",\"enabled\":false,\"name\":\"saa\",\"type\":\"location\"},{\"bundle\":\"coreTelephony\",\"enabled\":false,\"name\":\"park\",\"type\":\"location\"},{\"bundle\":\"coreTelephony\",\"enabled\":false,\"name\":\"sbcBridge\",\"type\":\"location\"},{\"bundle\":\"coreTelephony\",\"enabled\":false,\"name\":\"registrar\",\"type\":\"location\"},{\"bundle\":\"coreTelephony\",\"enabled\":false,\"name\":\"rls\",\"type\":\"location\"},{\"bundle\":\"coreTelephony\",\"enabled\":false,\"name\":\"sipxcdr\",\"type\":\"location\"},{\"bundle\":\"coreTelephony\",\"enabled\":false,\"name\":\"sipxsqa\",\"type\":\"location\"},{\"bundle\":\"coreTelephony\",\"enabled\":false,\"name\":\"mwi\",\"type\":\"location\"},{\"bundle\":\"coreTelephony\",\"enabled\":false,\"name\":\"page\",\"type\":\"location\"},{\"bundle\":\"coreTelephony\",\"enabled\":false,\"name\":\"proxy\",\"type\":\"location\"},{\"bundle\":\"coreTelephony\",\"enabled\":false,\"name\":\"restServer\",\"type\":\"location\"},{\"bundle\":\"coreTelephony\",\"enabled\":false,\"name\":\"conference\",\"type\":\"location\"},{\"bundle\":\"coreTelephony\",\"enabled\":false,\"name\":\"mysql\",\"type\":\"location\"},{\"bundle\":\"im\",\"enabled\":false,\"name\":\"imbot\",\"type\":\"location\"},{\"bundle\":\"provision\",\"enabled\":false,\"name\":\"dhcpd\",\"type\":\"location\"},{\"bundle\":\"provision\",\"enabled\":false,\"name\":\"phonelog\",\"type\":\"location\"},{\"bundle\":\"provision\",\"enabled\":false,\"name\":\"ftp\",\"type\":\"location\"},{\"bundle\":\"provision\",\"enabled\":false,\"name\":\"tftp\",\"type\":\"location\"}]}";
        JSONAssert.assertEquals(expectedServerFeatures, serverFeatures, false);
        
        int code = putPlainText("", "/servers/features/snmp");
        assertEquals(200, code);
        String serverFeaturesSnmp = getAsJson("/servers/features/");
        String expectedServerFeaturesSnmp = "{\"features\":[{\"bundle\":\"core\",\"enabled\":false,\"type\":\"global\",\"name\":\"sipxlogwatcher\"},{\"bundle\":\"core\",\"enabled\":true,\"type\":\"global\",\"name\":\"snmp\"},{\"bundle\":\"core\",\"enabled\":false,\"type\":\"location\",\"name\":\"dhcpd\"},{\"bundle\":\"core\",\"enabled\":false,\"type\":\"global\",\"name\":\"mail\"},{\"bundle\":\"core\",\"enabled\":false,\"type\":\"global\",\"name\":\"firewall\"},{\"bundle\":\"core\",\"enabled\":false,\"type\":\"location\",\"name\":\"sipxdns\"},{\"bundle\":\"core\",\"enabled\":false,\"type\":\"location\",\"name\":\"event\"},{\"bundle\":\"core\",\"enabled\":false,\"type\":\"global\",\"name\":\"ntpd\"},{\"bundle\":\"core\",\"enabled\":false,\"type\":\"global\",\"name\":\"fail2ban\"},{\"bundle\":\"core\",\"enabled\":false,\"type\":\"global\",\"name\":\"alarms\"},{\"bundle\":\"coreTelephony\",\"enabled\":false,\"type\":\"location\",\"name\":\"authCode\"},{\"bundle\":\"coreTelephony\",\"enabled\":false,\"type\":\"location\",\"name\":\"freeSwitch\"},{\"bundle\":\"coreTelephony\",\"enabled\":false,\"type\":\"location\",\"name\":\"redis\"},{\"bundle\":\"coreTelephony\",\"enabled\":false,\"type\":\"location\",\"name\":\"ivr\"},{\"bundle\":\"coreTelephony\",\"enabled\":false,\"type\":\"location\",\"name\":\"moh\"},{\"bundle\":\"coreTelephony\",\"enabled\":false,\"type\":\"location\",\"name\":\"saa\"},{\"bundle\":\"coreTelephony\",\"enabled\":false,\"type\":\"location\",\"name\":\"park\"},{\"bundle\":\"coreTelephony\",\"enabled\":false,\"type\":\"location\",\"name\":\"sbcBridge\"},{\"bundle\":\"coreTelephony\",\"enabled\":false,\"type\":\"location\",\"name\":\"registrar\"},{\"bundle\":\"coreTelephony\",\"enabled\":false,\"type\":\"location\",\"name\":\"rls\"},{\"bundle\":\"coreTelephony\",\"enabled\":false,\"type\":\"location\",\"name\":\"sipxcdr\"},{\"bundle\":\"coreTelephony\",\"enabled\":false,\"type\":\"global\",\"name\":\"intercom\"},{\"bundle\":\"coreTelephony\",\"enabled\":false,\"type\":\"location\",\"name\":\"sipxsqa\"},{\"bundle\":\"coreTelephony\",\"enabled\":false,\"type\":\"location\",\"name\":\"mwi\"},{\"bundle\":\"coreTelephony\",\"enabled\":false,\"type\":\"location\",\"name\":\"page\"},{\"bundle\":\"coreTelephony\",\"enabled\":false,\"type\":\"location\",\"name\":\"proxy\"},{\"bundle\":\"coreTelephony\",\"enabled\":false,\"type\":\"location\",\"name\":\"restServer\"},{\"bundle\":\"coreTelephony\",\"enabled\":false,\"type\":\"location\",\"name\":\"conference\"},{\"bundle\":\"coreTelephony\",\"enabled\":false,\"type\":\"location\",\"name\":\"mysql\"},{\"bundle\":\"im\",\"enabled\":false,\"type\":\"location\",\"name\":\"imbot\"},{\"bundle\":\"provision\",\"enabled\":false,\"type\":\"location\",\"name\":\"dhcpd\"},{\"bundle\":\"provision\",\"enabled\":false,\"type\":\"location\",\"name\":\"phonelog\"},{\"bundle\":\"provision\",\"enabled\":false,\"type\":\"location\",\"name\":\"ftp\"},{\"bundle\":\"provision\",\"enabled\":false,\"type\":\"location\",\"name\":\"tftp\"}]}";
        JSONAssert.assertEquals(expectedServerFeaturesSnmp, serverFeaturesSnmp, false);
        
        int codeDisable = delete("/servers/features/snmp");
        assertEquals(200, codeDisable);
        JSONAssert.assertEquals(expectedFeatures, getAsJson("/servers/features/"), false);
        
        assertEquals(200,putPlainText("", "/servers/101/features/dhcpd"));
        JSONAssert.assertEquals("{\"features\":[{\"bundle\":\"core\",\"enabled\":true,\"name\":\"dhcpd\",\"type\":\"location\"},{\"bundle\":\"core\",\"enabled\":false,\"name\":\"sipxdns\",\"type\":\"location\"},{\"bundle\":\"core\",\"enabled\":false,\"name\":\"event\",\"type\":\"location\"},{\"bundle\":\"coreTelephony\",\"enabled\":false,\"name\":\"authCode\",\"type\":\"location\"},{\"bundle\":\"coreTelephony\",\"enabled\":false,\"name\":\"freeSwitch\",\"type\":\"location\"},{\"bundle\":\"coreTelephony\",\"enabled\":false,\"name\":\"redis\",\"type\":\"location\"},{\"bundle\":\"coreTelephony\",\"enabled\":false,\"name\":\"ivr\",\"type\":\"location\"},{\"bundle\":\"coreTelephony\",\"enabled\":false,\"name\":\"moh\",\"type\":\"location\"},{\"bundle\":\"coreTelephony\",\"enabled\":false,\"name\":\"saa\",\"type\":\"location\"},{\"bundle\":\"coreTelephony\",\"enabled\":false,\"name\":\"park\",\"type\":\"location\"},{\"bundle\":\"coreTelephony\",\"enabled\":false,\"name\":\"sbcBridge\",\"type\":\"location\"},{\"bundle\":\"coreTelephony\",\"enabled\":false,\"name\":\"registrar\",\"type\":\"location\"},{\"bundle\":\"coreTelephony\",\"enabled\":false,\"name\":\"rls\",\"type\":\"location\"},{\"bundle\":\"coreTelephony\",\"enabled\":false,\"name\":\"sipxcdr\",\"type\":\"location\"},{\"bundle\":\"coreTelephony\",\"enabled\":false,\"name\":\"sipxsqa\",\"type\":\"location\"},{\"bundle\":\"coreTelephony\",\"enabled\":false,\"name\":\"mwi\",\"type\":\"location\"},{\"bundle\":\"coreTelephony\",\"enabled\":false,\"name\":\"page\",\"type\":\"location\"},{\"bundle\":\"coreTelephony\",\"enabled\":false,\"name\":\"proxy\",\"type\":\"location\"},{\"bundle\":\"coreTelephony\",\"enabled\":false,\"name\":\"restServer\",\"type\":\"location\"},{\"bundle\":\"coreTelephony\",\"enabled\":false,\"name\":\"conference\",\"type\":\"location\"},{\"bundle\":\"coreTelephony\",\"enabled\":false,\"name\":\"mysql\",\"type\":\"location\"},{\"bundle\":\"im\",\"enabled\":false,\"name\":\"imbot\",\"type\":\"location\"},{\"bundle\":\"provision\",\"enabled\":true,\"name\":\"dhcpd\",\"type\":\"location\"},{\"bundle\":\"provision\",\"enabled\":false,\"name\":\"phonelog\",\"type\":\"location\"},{\"bundle\":\"provision\",\"enabled\":false,\"name\":\"ftp\",\"type\":\"location\"},{\"bundle\":\"provision\",\"enabled\":false,\"name\":\"tftp\",\"type\":\"location\"}]}", getAsJson("/servers/101/features"), false);
        
        assertEquals(200,delete("/servers/101/features/dhcpd"));
        JSONAssert.assertEquals("{\"features\":[{\"bundle\":\"core\",\"enabled\":false,\"name\":\"dhcpd\",\"type\":\"location\"},{\"bundle\":\"core\",\"enabled\":false,\"name\":\"sipxdns\",\"type\":\"location\"},{\"bundle\":\"core\",\"enabled\":false,\"name\":\"event\",\"type\":\"location\"},{\"bundle\":\"coreTelephony\",\"enabled\":false,\"name\":\"authCode\",\"type\":\"location\"},{\"bundle\":\"coreTelephony\",\"enabled\":false,\"name\":\"freeSwitch\",\"type\":\"location\"},{\"bundle\":\"coreTelephony\",\"enabled\":false,\"name\":\"redis\",\"type\":\"location\"},{\"bundle\":\"coreTelephony\",\"enabled\":false,\"name\":\"ivr\",\"type\":\"location\"},{\"bundle\":\"coreTelephony\",\"enabled\":false,\"name\":\"moh\",\"type\":\"location\"},{\"bundle\":\"coreTelephony\",\"enabled\":false,\"name\":\"saa\",\"type\":\"location\"},{\"bundle\":\"coreTelephony\",\"enabled\":false,\"name\":\"park\",\"type\":\"location\"},{\"bundle\":\"coreTelephony\",\"enabled\":false,\"name\":\"sbcBridge\",\"type\":\"location\"},{\"bundle\":\"coreTelephony\",\"enabled\":false,\"name\":\"registrar\",\"type\":\"location\"},{\"bundle\":\"coreTelephony\",\"enabled\":false,\"name\":\"rls\",\"type\":\"location\"},{\"bundle\":\"coreTelephony\",\"enabled\":false,\"name\":\"sipxcdr\",\"type\":\"location\"},{\"bundle\":\"coreTelephony\",\"enabled\":false,\"name\":\"sipxsqa\",\"type\":\"location\"},{\"bundle\":\"coreTelephony\",\"enabled\":false,\"name\":\"mwi\",\"type\":\"location\"},{\"bundle\":\"coreTelephony\",\"enabled\":false,\"name\":\"page\",\"type\":\"location\"},{\"bundle\":\"coreTelephony\",\"enabled\":false,\"name\":\"proxy\",\"type\":\"location\"},{\"bundle\":\"coreTelephony\",\"enabled\":false,\"name\":\"restServer\",\"type\":\"location\"},{\"bundle\":\"coreTelephony\",\"enabled\":false,\"name\":\"conference\",\"type\":\"location\"},{\"bundle\":\"coreTelephony\",\"enabled\":false,\"name\":\"mysql\",\"type\":\"location\"},{\"bundle\":\"im\",\"enabled\":false,\"name\":\"imbot\",\"type\":\"location\"},{\"bundle\":\"provision\",\"enabled\":false,\"name\":\"dhcpd\",\"type\":\"location\"},{\"bundle\":\"provision\",\"enabled\":false,\"name\":\"phonelog\",\"type\":\"location\"},{\"bundle\":\"provision\",\"enabled\":false,\"name\":\"ftp\",\"type\":\"location\"},{\"bundle\":\"provision\",\"enabled\":false,\"name\":\"tftp\",\"type\":\"location\"}]}", getAsJson("/servers/101/features"), false);
        
    }

    public void setImdb(MongoTemplate imdb) {
        m_imdb = imdb;
    }

    public void setLocationsManager(LocationsManager locationsManager) {
        m_locationsManager = locationsManager;
    }

}
