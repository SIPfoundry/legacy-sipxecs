/**
 *
 *
 * Copyright (c) 2011 eZuce, Inc. All rights reserved.
 * Contributed to SIPfoundry under a Contributor Agreement
 *
 * This software is free software; you can redistribute it and/or modify it under
 * the terms of the Affero General Public License (AGPL) as published by the
 * Free Software Foundation; either version 3 of the License, or (at your option)
 * any later version.
 *
 * This software is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more
 * details.
 */
package org.sipfoundry.sipxivr;

import java.io.IOException;
import java.net.ServerSocket;
import java.net.Socket;

import org.apache.log4j.Logger;
import org.apache.log4j.PropertyConfigurator;
import org.sipfoundry.commons.log4j.SipFoundryLayout;
import org.sipfoundry.voicemail.mailbox.MailboxManager;
import org.springframework.beans.BeansException;
import org.springframework.context.ApplicationContext;
import org.springframework.context.support.ClassPathXmlApplicationContext;

public abstract class SipXivrServer {
    static final Logger LOG = Logger.getLogger("org.sipfoundry.sipxivr");
    private int m_eventSocketPort;

    protected abstract SipXivr getSipxIvrHandler();

    public void runServer() {
        try {
            ServerSocket serverSocket = new ServerSocket(m_eventSocketPort);
            for (;;) {
                Socket client = serverSocket.accept();
                SipXivr sipxIvr = getSipxIvrHandler();
                sipxIvr.setClient(client);
                Thread thread = new Thread(sipxIvr);
                thread.start();
            }
        } catch (IOException ex) {
            System.out.println("FAILED TO START IVR SERVER" + ex);
            ex.printStackTrace();
            System.exit(1);
        }
    }

    public void setEventSocketPort(int port) {
        m_eventSocketPort = port;
    }

    /**
     * Main entry point for sipXivr
     * 
     * @param args
     */
    public static void main(String[] args) {
        try {
            initSystemProperties();
            ApplicationContext context = new ClassPathXmlApplicationContext(new String[] {
                "classpath:/org/sipfoundry/sipxivr/system.beans.xml",
                "classpath:/org/sipfoundry/sipxivr/imdb.beans.xml",
                "classpath:/org/sipfoundry/sipxivr/email/email.beans.xml",
                "classpath:/org/sipfoundry/attendant/attendant.beans.xml",
                "classpath:/org/sipfoundry/bridge/bridge.beans.xml",
                "classpath:/org/sipfoundry/faxrx/fax.beans.xml",
                "classpath:/org/sipfoundry/moh/moh.beans.xml",
                "classpath:/org/sipfoundry/voicemail/voicemail.beans.xml",
                "classpath:/org/sipfoundry/sipxivr/rest/rest.beans.xml",
                "classpath:/org/sipfoundry/voicemail/mailbox/mailbox.beans.xml",
                "classpath*:/sipxivrplugin.beans.xml"
            });
            SipXivrServer socket = (SipXivrServer) context.getBean("sipxIvrServer");
            for (int i = 0; i < args.length; i++) {
                if (args[i].equalsIgnoreCase("--migrate")) {
                    String pathToMailbox = args[i+1];
                    LOG.info(String.format("Starting sipXivr with --migrate from %s", pathToMailbox));
                    MailboxManager manager = (MailboxManager) context.getBean("mailboxManager");
                    manager.migrate(pathToMailbox);
                }
            }
            socket.runServer();
        } catch (BeansException ex) {
            System.out.println("FAILED TO CREATE SPRING CONTAINER" + ex);
            ex.printStackTrace();
            System.exit(1);
        }
    }

    private static void initSystemProperties() {
        String path = System.getProperty("conf.dir");
        // Configure log4j
        PropertyConfigurator.configureAndWatch(path+"/sipxivr/log4j.properties", 
                SipFoundryLayout.LOG4J_MONITOR_FILE_DELAY);
        if (path == null) {
            System.err.println("Cannot get System Property conf.dir!  Check jvm argument -Dconf.dir=") ;
            System.exit(1);
        }
        
        // Setup SSL properties so we can talk to HTTPS servers
        String keyStore = System.getProperty("javax.net.ssl.keyStore");
        if (keyStore == null) {
            // Take an educated guess as to where it should be
            keyStore = path+"/ssl/ssl.keystore";
            System.setProperty("javax.net.ssl.keyStore", keyStore);
            System.setProperty("javax.net.ssl.keyStorePassword", "changeit"); // Real security!
        }
        String trustStore = System.getProperty("javax.net.ssl.trustStore");
        if (trustStore == null) {
            // Take an educated guess as to where it should be
            trustStore = path+"/ssl/authorities.jks";
            System.setProperty("javax.net.ssl.trustStore", trustStore);
            System.setProperty("javax.net.ssl.trustStoreType", "JKS");
            System.setProperty("javax.net.ssl.trustStorePassword", "changeit"); // Real security!
        }
    }
}
