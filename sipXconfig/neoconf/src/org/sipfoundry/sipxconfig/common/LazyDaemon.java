/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.common;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;

/**
 * Daemon thread that sleeps most of the time in waitForWork method. When there is something to be
 * do it wakes up, sleeps some more and then does it.
 */
public abstract class LazyDaemon extends Thread {
    public static final Log LOG = LogFactory.getLog(LazyDaemon.class);
    private int m_sleepInterval;
    private volatile long m_workAddedTimestamp;

    public LazyDaemon(String name, int sleepInterval) {
        super(name);
        m_sleepInterval = sleepInterval;
        setPriority(Thread.MIN_PRIORITY);
        setDaemon(true);
    }

    public final void run() {
        try {
            boolean moreWork = true;
            while (moreWork) {

                // initial work added
                waitForWork();
                long quietPeriod = 0;

                // loop here for quiet time
                do {
                    LOG.debug("Waiting for quiet period, ms: " + m_sleepInterval);
                    sleep(m_sleepInterval);
                    quietPeriod = System.currentTimeMillis() - m_workAddedTimestamp;
                } while(quietPeriod < m_sleepInterval);
                try {
                    LOG.debug("Staring work");
                    moreWork = work();
                } catch (Throwable e) {
                    LOG.error(getName() + "exception in background task.", e);
                }
            }
        } catch (InterruptedException e) {
            LOG.error(getName() + "exiting due to exception.", e);
        }
    }

    /**
     * Call this every time you add work to the queue, this is the key to implementing "quiet" time
     * so that work won't be called into there was no *new* work in the timeout period.
     */
    public void workScheduled() {
        m_workAddedTimestamp = System.currentTimeMillis();
    }

    /**
     * Overwrite to call wait function to suspend this thread until there is something to be done.
     * It's pefectly valid not to wait for anything - in that case this thread will still sleep
     * for m_sleepInterval before calling work
     *
     * @throws InterruptedException
     */
    protected abstract void waitForWork() throws InterruptedException;

    /**
     * Overwrite to do something with low priority that needs to be done regularly (house cleaning
     * tasks).
     *
     * @return moreWork - true if thread should keep on calling work, false to finish thread
     */
    protected abstract boolean work();
}
