/**
 *
 *
 * Copyright (c) 2012 eZuce, Inc. All rights reserved.
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
