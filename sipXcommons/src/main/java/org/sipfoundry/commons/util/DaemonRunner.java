/*
 * Copyright (C) 2011 eZuce Inc., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the AGPL license.
 *
 * $
 */
package org.sipfoundry.commons.util;

import org.apache.log4j.Logger;

public class DaemonRunner {
    static final Logger LOG = Logger.getLogger(DaemonRunner.class);
    private Daemon m_daemon;
    
    public DaemonRunner(Daemon daemon) {
        m_daemon = daemon;
    }
    
    public void run() {
        try {
            Runtime.getRuntime().addShutdownHook(new OnStop());
            m_daemon.start();
        } catch (SecurityException e) {
            e.printStackTrace();
        }        
    }

    class OnStop extends Thread {
        @Override
        public void run() {
            m_daemon.stop();
        }        
    }
}
