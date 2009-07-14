package org.sipfoundry.openfire.client;

import org.sipfoundry.openfire.config.ConfigurationParser;
import org.sipfoundry.openfire.config.WatcherConfig;

import junit.framework.TestCase;

public class OpenfireXmlRpcUserAccountClientTest extends TestCase {
    
    OpenfireXmlRpcUserAccountClient client ;
    
    String domain;
    String sipDomain;
    private WatcherConfig watcherConfig;
    String configDir = "/usr/local/sipx/etc/sipxpbx";
    
    public void setUp() throws Exception {
        ConfigurationParser configParser = new ConfigurationParser();
        
        this.watcherConfig = configParser.parse("file://" + configDir + "/sipxopenfire.xml");
        this.domain = watcherConfig.getOpenfireHost();
        this.sipDomain = watcherConfig.getProxyDomain();
     
        client = new OpenfireXmlRpcUserAccountClient(domain,watcherConfig.getOpenfireXmlRpcPort());
        
    }
    
    public void testUserExists() throws Exception {
        if ( ! client.userExists("admin") ) {
            fail("Admin must exist!");
        }
        if ( client.userExists("foobarbaz")) {
            fail("foobarbaz must not exist!");
        }
    }
    
    public void testCreateUserAndAddSipDomain() throws Exception {
        
        client.createUserAccount("user1", "user1", "My Display Name", "user1@gmail.com");
        if ( ! client.userExists("foobar") ) {
            fail("foobar must exist!");
        }
        String sipUser = client.getSipId("user1");
        
        System.out.println("sipUser = " + sipUser);
        assertTrue( sipUser == null || sipUser.equals("user1@" + sipDomain));
        
        client.setSipId("foobar", "user1@" + sipDomain);
        
        sipUser = client.getSipId("user1");
        
        assertEquals(sipUser, "user1@"  +  sipDomain);
        
        final String message = "I am goofing off";
        client.setOnThePhoneMessage("user1@"+sipDomain,message );
        
        String onThePhoneMessage = client.getOnThePhoneMessage("user1@" + sipDomain);
        
        assertEquals("Message mismatch",message,onThePhoneMessage  );
        
    }
    
    
 

}
