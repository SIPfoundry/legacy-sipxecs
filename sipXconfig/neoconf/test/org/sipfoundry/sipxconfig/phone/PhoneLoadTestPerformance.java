/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.phone;

import java.util.Iterator;

import junit.framework.Test;
import junit.framework.TestSuite;

import org.sipfoundry.sipxconfig.TestHelper;
import org.sipfoundry.sipxconfig.test.PerformanceTestCase;

import com.clarkware.junitperf.TimedTest;

/**
 * Performace of dealing with large amounts of phones
 */
public class PhoneLoadTestPerformance extends PerformanceTestCase {

    private PhoneContext m_context;

    public PhoneLoadTestPerformance(String singleTestMethodName) {
        super(singleTestMethodName);
    }

    public static Test suite() {

        TestSuite suite = new TestSuite();

        suite.addTest(new TimedTest(new PhoneLoadTestPerformance("testLoad"), getTolerance(5000)));

        // lazy loading off, these are about same time
        suite.addTest(new TimedTest(new PhoneLoadTestPerformance("testLoadWithLines"), getTolerance(5000)));

        // substantially smaller than above
        suite.addTest(new TimedTest(new PhoneLoadTestPerformance("testLoadPage"), getTolerance(200)));

        return suite;
    }

    protected void setUpUnTimed() {
        m_context = (PhoneContext) TestHelper.getApplicationContext().getBean(
                PhoneContext.CONTEXT_BEAN_NAME);
        m_context.clear();
        PhoneCreateTestPerformance.seedPhones(m_context, 1000);
    }

    public void testLoad() {
        m_context.loadPhones();
    }

    public void testLoadWithLines() {
        Iterator i = m_context.loadPhones().iterator();
        while (i.hasNext()) {
            ((Phone) i.next()).getLines();
        }
    }

    public void testLoadPage() {
        m_context.loadPhonesByPage(null, 0, 30, new String[] { "serialNumber" }, true);
    }
}
