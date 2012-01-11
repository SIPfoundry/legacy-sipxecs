/*
 * Copyright (C) 2011 eZuce Inc., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the AGPL license.
 *
 * $
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
