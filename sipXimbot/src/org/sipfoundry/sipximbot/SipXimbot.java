/*
 *
 *
 * Copyright (C) 2008-2009 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 */
package org.sipfoundry.sipximbot;

import org.apache.log4j.Logger;

public class SipXimbot {
    static final Logger LOG = Logger.getLogger("org.sipfoundry.sipximbot");
    private static ImbotConfiguration s_config;


    /**
     * Load the configuration from the sipximbot.properties file.
     * Wait for FreeSWITCH to make a TCP connection to s_eventSocketPort.
     * Spawn off a thread to handle each connection.
     *
     * @throws Throwable
     */
    static void init() throws Throwable {

        // Load the configuration
        s_config = ImbotConfiguration.get();

        // Create Web Server
        WebServer webServer = new WebServer(s_config);
        webServer.addServlet("IM", "/IM/*", ImbotServlet.class.getName());
        webServer.start();

        IMBot.init();

        ConfTask confThread = new ConfTask();
        confThread.setConfConfiguration(s_config);
        confThread.start();

    }

    /**
     * Main entry point for sipXimbot
     * @param args
     */
    public static void main(String[] args) {
        try {
            init();
        } catch (Exception e) {
            LOG.fatal(e,e);
            e.printStackTrace();
            System.exit(1);
        } catch (Throwable t) {
            LOG.fatal(t,t);
            t.printStackTrace();
            System.exit(1);
        }
    }
}
