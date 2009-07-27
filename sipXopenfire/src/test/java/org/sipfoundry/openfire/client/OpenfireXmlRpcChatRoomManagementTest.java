package org.sipfoundry.openfire.client;
import java.util.HashMap;
import java.util.Map;

import org.sipfoundry.openfire.client.OpenfireXmlRpcChatRoomManagementClient;
import org.sipfoundry.openfire.client.OpenfireXmlRpcPresenceClient;
import org.sipfoundry.openfire.config.ConfigurationParser;
import org.sipfoundry.openfire.config.WatcherConfig;

import junit.framework.TestCase;


public class OpenfireXmlRpcChatRoomManagementTest extends TestCase {
    String domain = "sipxpbx.example.local";
    String sipDomain = "sipxpbx.example.local";
    private OpenfireXmlRpcChatRoomManagementClient client;
    private WatcherConfig watcherConfig;
  
    // HARD CODED -- please change as needed.
    private String configDir = "/usr/local/sipx/etc/sipxpbx";
    
    // NOTE - you MUST BE LOGGED IN AS ADMIN from your XMPP CLIENT
    
    public void setUp() throws Exception {
        ConfigurationParser configParser = new ConfigurationParser();
        watcherConfig = configParser.parse("file://" + configDir + "/sipxopenfire.xml");
        this.domain = watcherConfig.getOpenfireHost();
        this.sipDomain = watcherConfig.getProxyDomain();
        this.client = new OpenfireXmlRpcChatRoomManagementClient(domain,watcherConfig.getOpenfireXmlRpcPort());
    }
    public void testCreateChatRoom() throws Exception {
        client.createChatRoom("foo" , "bazbaz","eat scrum and die.",
        "password",        
        "2412");
        String[] members = client.getMembers("foo", "bazbaz");
        assertTrue("Must be zero members ", members.length == 0 );
        Map attributes = client.getAttributes("foo", "bazbaz");
        boolean isLoginRestrictedToNickName = Boolean.parseBoolean(attributes.get("isLoginRestrictedToNickName").toString());
        
        Map<String,String> newAttributes = new HashMap<String,String>();
        newAttributes.put("isLoginRestrictedToNickName",""+!isLoginRestrictedToNickName);
        
        Map changedAttribs = client.setAttributes("foo","bazbaz",newAttributes);
        assertTrue(!isLoginRestrictedToNickName == Boolean.parseBoolean(newAttributes.get("isLoginRestrictedToNickName").toString()));
        System.out.println("changedAttribs " + changedAttribs);
        
        String extension = client.getConferenceExtension("foo" , "bazbaz");
        System.out.println("extension " + extension);
      //  assertTrue(extension.equals("2412"));
        
        client.removeChatRoom("foo","bazbaz");
        
    }
}
