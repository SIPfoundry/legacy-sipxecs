/*
 * Copyright (C) 2010 Avaya, certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 */
package org.sipfoundry.openfire.xmpptestclient;

import org.jivesoftware.smack.Chat;
import org.jivesoftware.smack.ChatManager;
import org.jivesoftware.smack.MessageListener;
import org.jivesoftware.smack.XMPPConnection;
import org.jivesoftware.smack.XMPPException;
import org.jivesoftware.smack.packet.Message;
import org.sipfoundry.openfire.config.ConfigurationParser;
import org.sipfoundry.openfire.config.WatcherConfig;

import junit.framework.TestCase;

/**
 * Test if client exists by connecting to the xmpp server and sending a simple test message to it.
 * This tests if the user accounts manager actually created two clients called user1 and user2 on
 * the server.
 * 
 */
public class ClientExistsTest extends AbstractXmppTestCase {

    private Chat chatClient1;

    private Chat chatClient2;

    private ChatClientListener listener1;

    private ChatClientListener listener2;

    class ChatClientListener implements MessageListener {
        private String expectedSender;
        private boolean messageSeen;

        public ChatClientListener(String expectedSender) {
            this.expectedSender = expectedSender;
        }

        public boolean isMessageSeen() {
            return messageSeen;
        }

        @Override
        public void processMessage(Chat chat, Message message) {
            // TODO Auto-generated method stub
            String from = message.getFrom();
            messageSeen = true;
            assertEquals("Expecting message from " + expectedSender, expectedSender, from);
        }
    };

    public void setUp() throws Exception {
        super.setUp();
        ChatManager chatManager = connection1.getChatManager();
        String userJid = "user2@" + watcherConfig.getProxyDomain();

        listener1 = new ChatClientListener("user2@" + watcherConfig.getProxyDomain());
        chatClient1 = chatManager.createChat(userJid, "Test Thread 1", listener1);
        ChatManager chatManager2 = connection2.getChatManager();
        userJid = "user1@" + watcherConfig.getProxyDomain();
        listener2 = new ChatClientListener("user1@" + watcherConfig.getProxyDomain());
        chatClient2 = chatManager2.createChat(userJid, "Test Thread 2", listener2);

      
    }

    public void testSendMessage() throws Exception {
        /*
         * Expect that user2 will get this request.
         */

        chatClient1.sendMessage("hello world 1");
        chatClient2.sendMessage("hello world 2");
        Thread.sleep(1000);
        assertTrue("Message not seen at listener1", listener1.isMessageSeen());
        assertTrue("Message not seen at listener2", listener2.isMessageSeen());
    }
    
    
    public void tearDown() throws Exception {
        super.tearDown();
    }

}
