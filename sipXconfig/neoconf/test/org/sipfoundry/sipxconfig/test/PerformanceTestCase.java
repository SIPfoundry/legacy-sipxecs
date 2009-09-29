/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.test;

import junit.framework.TestCase;

public abstract class PerformanceTestCase extends TestCase {

    public static double FAST_MACHINE = 0;           // P4, 3GHz, 2GB RAM
    public static double SLOW_MACHINE = 10;          // P3, 1GHz, 128MB RAM
    public static double VERY_SLOW_MACHINE = 1000;   // XT, 16Hz, 640K RAM

    private static double FUDGE = FAST_MACHINE;

    private boolean m_setup;

    public PerformanceTestCase(String singleTestMethodName) {
        super(singleTestMethodName);

        // avoids having setup time affect timed results, not sure
        // why it's not like this by default
        setUp();
    }

    protected static long getTolerance(long time) {
        return (long)(time + ((FUDGE / 100) * time));
    }

    /**
     * subclasses must call super.setUp() if overridden
     */
    protected void setUp() {
        if (m_setup) {
            return;
        }
        setUpUnTimed();
        m_setup = true;
    }

    protected abstract void setUpUnTimed();

    protected void tearDown() {
        m_setup = false;
    }
}
