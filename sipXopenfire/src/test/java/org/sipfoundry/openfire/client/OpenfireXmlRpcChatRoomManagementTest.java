/*
 * Copyright (C) 2010 Avaya, certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 */
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
        System.out.println("domain  = " + this.domain);
        this.sipDomain = watcherConfig.getProxyDomain();
        this.client = new OpenfireXmlRpcChatRoomManagementClient(domain,watcherConfig.getOpenfireXmlRpcPort());
    }
    public void testGetChatRoomInfo() throws Exception {
      
        String[] members = client.getMembers("subdomain", "mychat");
        assertTrue("Must be zero members ", members.length == 0 );
       
     
        
    }
}
