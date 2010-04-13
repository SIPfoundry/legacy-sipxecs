/*
 * Copyright (C) 2010 Avaya, certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 */
package org.sipfoundry.openfire.client;

import java.util.Map;

import junit.framework.TestCase;

import org.sipfoundry.openfire.config.ConfigurationParser;
import org.sipfoundry.openfire.config.WatcherConfig;
import org.sipfoundry.openfire.plugin.presence.UnifiedPresence;


public class OpenfireXmlRpcPresenceClientTest extends TestCase {
    String domain = "sipxpbx.example.local";
    String sipDomain = "sipxpbx.example.local";
    private OpenfireXmlRpcPresenceClient presenceClient;
    private WatcherConfig watcherConfig;
    // HARD CODED -- please change as needed.
    private String configDir = "/usr/local/sipx/etc/sipxpbx";
    
    // NOTE - you MUST BE LOGGED IN AS ADMIN from your XMPP CLIENT
    
    public void setUp() throws Exception {
        ConfigurationParser configParser = new ConfigurationParser();
        watcherConfig = configParser.parse("file://" + configDir + "/sipxopenfire.xml");
        this.domain = watcherConfig.getOpenfireHost();
        this.sipDomain = watcherConfig.getProxyDomain();
        this.presenceClient = new OpenfireXmlRpcPresenceClient(domain,watcherConfig.getOpenfireXmlRpcPort());
    }
    
    /*
     * You must sign in to the presence server as admin to run these tests.
     */
    public void testPresence() throws Exception {
        String presenceInfo = presenceClient.getXmppPresenceState("admin");
        System.out.println("presenceInfo = " + presenceInfo);
        presenceClient.setXmppPresenceState("admin", UnifiedPresence.XmppPresence.AVAILABLE.toString());    
        String presenceState = presenceClient.getXmppPresenceState("admin");
        assertEquals( UnifiedPresence.XmppPresence.AVAILABLE.toString(), presenceState);
        presenceClient.setXmppPresenceState("admin", UnifiedPresence.XmppPresence.BUSY.toString());
        presenceState = presenceClient.getXmppPresenceState("admin");
        assertEquals(presenceState,UnifiedPresence.XmppPresence.BUSY.toString());
        presenceClient.setXmppCustomPresenceMessage("admin", "Hacking");
        String status = presenceClient.getXmppCustomPresenceMessage("admin");
        System.out.println("Server returned " + status);
        assertEquals(status,"Hacking");
        Map unifiedPresence = presenceClient.getUnifiedPresenceInfo("user1@" + this.domain);
        System.out.println("Unified Presence = " + unifiedPresence);
        
    }
}
