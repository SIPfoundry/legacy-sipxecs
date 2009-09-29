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

import junit.framework.TestCase;

public class LazyDaemonTest extends TestCase {

    private StringBuffer m_testBuffer = new StringBuffer();

    public void testRun() throws Exception {
        LazyDaemonMock mock = new LazyDaemonMock();
        mock.start();
        // do not give it more than 1 second to complete
        mock.join(1000);
        assertEquals("xXxxXxX", m_testBuffer.toString());
    }

    private class LazyDaemonMock extends LazyDaemon {
        private int m_i = 0;

        LazyDaemonMock() {
            super(LazyDaemonTest.class.getName(), 2);
        }

        protected void waitForWork() throws InterruptedException {
            m_testBuffer.append("x");
        }

        protected boolean work() {
            m_i++;
            if (m_i == 2) {
                // check if we can recover from that without stopping the thread
                throw new Error("ignore - thrown on purpose");
            }
            m_testBuffer.append("X");
            return m_i < 4;
        }
    }
}
