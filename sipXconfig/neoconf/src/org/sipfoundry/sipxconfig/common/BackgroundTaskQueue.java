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

import org.apache.commons.collections.Buffer;
import org.apache.commons.collections.BufferUtils;
import org.apache.commons.collections.buffer.UnboundedFifoBuffer;
import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;

public class BackgroundTaskQueue {
    private static final Log LOG = LogFactory.getLog(BackgroundTaskQueue.class);

    private final Buffer m_queue = BufferUtils.blockingBuffer(new UnboundedFifoBuffer());
    private final Worker m_worker = new Worker();

    public void addTask(Runnable task) {
        m_queue.add(task);
        startWorker();
    }

    public boolean isEmpty() {
        return m_queue.isEmpty();
    }

    private Runnable removeTask() {
        return (Runnable) m_queue.remove();
    }

    private synchronized void startWorker() {
        if (!m_worker.isAlive()) {
            m_worker.start();
        }
    }

    /**
     * This is one of the methods that can be used to actively wait till queue is empty Do not use
     * it unless you need it in testing.
     */
    public void yieldTillEmpty() {
        Runnable sentinel = new Runnable() {
            public void run() {
                // do nothing
            }
        };
        m_queue.add(sentinel);
        while (!m_queue.isEmpty()) {
            Thread.yield();
        }
    }

    /**
     * It's not going to suspend currently executed task. However the next task in the queue will
     * wait till resume is called.
     */
    void suspend() {
        m_worker.setSuspend(true);
    }

    void resume() {
        m_worker.setSuspend(false);
    }

    /**
     * Worker thread - simple background worker. Takes one task from queue, executes it, takes
     * next task etc.
     */
    private class Worker extends Thread {
        private boolean m_suspend;

        public Worker() {
            super("background");
            // do not stop JVM from dying
            setDaemon(true);
        }

        public synchronized void setSuspend(boolean suspend) {
            m_suspend = suspend;
            if (!m_suspend) {
                notify();
            }
        }

        @Override
        public void run() {
            try {
                while (true) {
                    synchronized (this) {
                        if (m_suspend) {
                            wait();
                        }
                    }
                    Runnable task = removeTask();
                    try {
                        task.run();
                    } catch (Throwable e) {
                        LOG.error("Exception in background task.", e);
                    }
                }
            } catch (InterruptedException e) {
                LOG.error("Background task thread exiting due to exception.", e);
            }
        }
    }
}
