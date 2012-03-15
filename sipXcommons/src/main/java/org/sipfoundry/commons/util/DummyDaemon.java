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

import java.io.IOException;

import org.apache.log4j.BasicConfigurator;
import org.apache.log4j.FileAppender;
import org.apache.log4j.Logger;
import org.apache.log4j.SimpleLayout;

public class DummyDaemon implements Daemon {
    private static final Logger log = Logger.getLogger(DummyDaemon.class);

    public static final void main(String[] args) {
        try {
            BasicConfigurator.configure(new FileAppender(new SimpleLayout(), "/tmp/out.log"));
        } catch (IOException e) {
            e.printStackTrace();
        }
        new DaemonRunner(new DummyDaemon()).run();
    }

    @Override
    public void stop() {
        log.info("Going to stop");
        synchronized (this) {
            log.info("Going to notify");
            this.notifyAll();
            log.info("Finsihed notify");
        }
    }

    @Override
    public void start() {
        synchronized (this) {
            try {
                log.info("Going to sleep");
                wait();
                log.info("Done wait");
            } catch (InterruptedException e) {
                log.error("Error in wait", e);
            }
        }
    }
}
