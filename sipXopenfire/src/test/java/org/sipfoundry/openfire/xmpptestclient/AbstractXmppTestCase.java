/*
 * Copyright (C) 2010 Avaya, certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 */
package org.sipfoundry.openfire.xmpptestclient;

import junit.framework.TestCase;

import org.jivesoftware.smack.XMPPConnection;
import org.sipfoundry.openfire.config.ConfigurationParser;
import org.sipfoundry.openfire.config.WatcherConfig;

public abstract class AbstractXmppTestCase extends TestCase {

    protected WatcherConfig watcherConfig;

    protected XMPPConnection connection1 ;
    
    protected XMPPConnection connection2;

    public void setUp() throws Exception {
        super.setUp();
        ConfigurationParser configurationParser = new ConfigurationParser();
        // Copy the openfire file into the /usr/local/etc/sipxpbx/sipxopenfire.xml location.
        // Copy the xmpp-accounts-info.xml to that same location.
        this.watcherConfig = configurationParser
                .parse("file:///usr/local/sipx/etc/sipxpbx/sipxopenfire.xml");
        connection1 = new XMPPConnection(watcherConfig.getProxyDomain());
        connection1.connect();
        connection1.login("user1", "123" , "user1@" + watcherConfig.getProxyDomain()+ "/Home");
        connection2 = new XMPPConnection(watcherConfig.getProxyDomain());
        connection2.connect();
        connection2.login("user2", "123" , "user2@" + watcherConfig.getProxyDomain()+ "/Home");
     
    }
    
    public void tearDown() throws Exception {
        connection1.disconnect();
        connection2.disconnect();
    }

}
