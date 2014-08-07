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
import org.sipfoundry.commons.hz.HzConstants;
import org.sipfoundry.commons.hz.HzImEvent;
import org.sipfoundry.commons.hz.HzMediaEvent;

import com.hazelcast.core.Hazelcast;
import com.hazelcast.core.HazelcastInstance;
import com.hazelcast.core.ITopic;

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

        // Create & configure hazelcast instance
        HazelcastInstance hzInstance = Hazelcast.newHazelcastInstance();
        ITopic<HzMediaEvent> vmTopic = hzInstance.getTopic(HzConstants.VM_TOPIC);
        vmTopic.addMessageListener(new MediaMessageListener());
        ITopic<HzMediaEvent> confTopic = hzInstance.getTopic(HzConstants.CONF_TOPIC);
        confTopic.addMessageListener(new MediaMessageListener());
        ITopic<HzImEvent> imTopic = hzInstance.getTopic(HzConstants.IM_TOPIC);
        imTopic.addMessageListener(new ImMessageListener());

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
