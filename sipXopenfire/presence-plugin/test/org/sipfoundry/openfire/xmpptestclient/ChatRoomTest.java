/*
 * Copyright (C) 2010 Avaya, certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 */
package org.sipfoundry.openfire.xmpptestclient;

import java.util.ArrayList;
import java.util.Iterator;
import java.util.List;

import org.jivesoftware.openfire.forms.FormField;
import org.jivesoftware.openfire.forms.spi.XDataFormImpl;
import org.jivesoftware.smack.PacketListener;
import org.jivesoftware.smack.packet.Packet;
import org.jivesoftware.smack.packet.Presence.Mode;
import org.jivesoftware.smackx.Form;
import org.jivesoftware.smackx.muc.MultiUserChat;
import org.jivesoftware.smackx.packet.DataForm;

public class ChatRoomTest extends AbstractXmppTestCase {
    
    
    
    public class TestPacketListener implements PacketListener {

        @Override
        public void processPacket(Packet packet) {
          System.out.println(packet.toXML());
          
        }
        
    }
    
    @Override
    public void setUp() throws Exception {
        super.setUp();
        
    }
    
    public void testJoinChatRoom() throws Exception {
           
        // Join the chat room with pidgin and run this test.
         
        MultiUserChat muc2 = new MultiUserChat(super.connection2, "mychat@subdomain." + super.watcherConfig.getProxyDomain());
        muc2.addMessageListener(new TestPacketListener());
        muc2.join("user2", "password");
        muc2.sendMessage("Hello world");
       
        Thread.sleep(1000);

    }
    
    public void tearDown() throws Exception {
        super.tearDown();
    }


}
