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

import junit.framework.Test;
import junit.framework.TestSuite;

import org.sipfoundry.sipxconfig.TestHelper;
import org.sipfoundry.sipxconfig.test.PerformanceTestCase;

import com.clarkware.junitperf.TimedTest;

public class PhoneCreateTestPerformance  extends PerformanceTestCase {

    private PhoneContext m_context;

    public PhoneCreateTestPerformance(String singleTestMethodName) {
        super(singleTestMethodName);
    }

    public static Test suite() {

        TestSuite suite = new TestSuite();

        suite.addTest(new TimedTest(new PhoneCreateTestPerformance("testCreate"), getTolerance(2500)));

        return suite;
    }

    protected void setUpUnTimed() {
        m_context = (PhoneContext) TestHelper.getApplicationContext().getBean(
                PhoneContext.CONTEXT_BEAN_NAME);
        m_context.clear();
    }

    public void testCreate() {
        seedPhones(m_context, 100);
    }

    static void seedPhones(PhoneContext context, int count) {
        for (int i = 0; i < count; i++) {
            Phone p = context.newPhone(new PhoneModel(TestPhone.BEAN_ID));
            p.setSerialNumber("ff00" + String.valueOf(i));
            p.addLine(p.createLine());
            context.storePhone(p);
        }
    }
}
