package org.sipfoundry.openfire.client;

import java.util.Set;

import org.sipfoundry.openfire.config.ConfigurationParser;
import org.sipfoundry.openfire.config.WatcherConfig;
import org.sipfoundry.openfire.plugin.presence.UserAccount;

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
       
        client.destroyUserAccount("user1");
        client.destroyUserAccount("foobar");
        
        client.createUserAccount("user1", "user1", "My Display Name", "user1@gmail.com");
        
        client.createUserAccount("foobar", "foobar", "My Display Name", "user1@gmail.com");
    
         
        assertTrue ("user1 must exist", client.userExists("user1"));
        assertTrue ("foobar must exsit", client.userExists("foobar"));
        
        Set<UserAccount> userAccounts = client.getUserAccounts();
        
        System.out.println("UserAccount  = " + userAccounts);
        boolean foundFoobar = false;
        for ( UserAccount userAccount : userAccounts ) {
        	System.out.println("SipUserName " + userAccount.getSipUserName());
        	System.out.println("xmpp userName  " + userAccount.getXmppUserName());
        	foundFoobar = foundFoobar || userAccount.getXmppUserName().equals("foobar");	
        }
        
        assertTrue("Acount foobar not found",foundFoobar);
        
        String sipUser = client.getSipId("user1");
        
        System.out.println("sipUser = " + sipUser);
        assertTrue( sipUser == null || sipUser.equals("user1@" + sipDomain));
        
        client.setSipId("user1", "user1@" + sipDomain);
        
        sipUser = client.getSipId("user1");
        
        assertEquals(sipUser, "user1@"  +  sipDomain);
        
        final String message = "I am goofing off";
        client.setOnThePhoneMessage("user1@"+sipDomain,message );
        
        String onThePhoneMessage = client.getOnThePhoneMessage("user1@" + sipDomain);
        
        assertEquals("Message mismatch",message,onThePhoneMessage  );
        

        System.out.println("UserAccount  = " + userAccounts);
        foundFoobar = false;
        for ( UserAccount userAccount : userAccounts ) {
        	System.out.println("SipUserName " + userAccount.getSipUserName());
        	System.out.println("xmpp userName  " + userAccount.getXmppUserName());
        	foundFoobar = foundFoobar || userAccount.getXmppUserName().equals("foobar");	
        }
        
        client.destroyUserAccount("user1");
        client.destroyUserAccount("foobar");
        assertTrue ("user1 must not exist", !client.userExists("user1"));
        assertTrue ("foobar must not exsit", !client.userExists("foobar"));
       
        
    }
    
    
    public void testCreateGroup() throws Exception {
        client.createGroup("TestGroup", "user1", "A test group");
        assertTrue("Group exists",client.groupExists("TestGroup"));
        client.addUserToGroup("admin", "TestGroup");
        assertTrue("User must be in group", client.isUserInGroup("admin","TestGroup"));
        client.removeUserFromGroup("admin","TestGroup");
        assertTrue("User must not be in group", !client.isUserInGroup("admin","TestGroup"));
    }
    
    
 
    
    
    
 

}
