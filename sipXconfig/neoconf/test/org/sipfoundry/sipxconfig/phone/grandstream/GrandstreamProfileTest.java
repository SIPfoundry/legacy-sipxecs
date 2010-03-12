/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.phone.grandstream;

import java.io.ByteArrayOutputStream;
import java.io.IOException;

import junit.framework.TestCase;

import org.apache.commons.io.IOUtils;
import org.sipfoundry.sipxconfig.phone.PhoneTestDriver;
import org.sipfoundry.sipxconfig.test.TestUtil;

public class GrandstreamProfileTest extends TestCase {

    GrandstreamPhone phone;

    PhoneTestDriver tester;

    public void testHandytone() throws Exception {
        assertProfile("gsHt496", "expected-ht496.cfg");
    }

    public void testBudgetone10x() throws Exception {
        assertProfile("gsPhoneBt100", "expected-gsbt100.cfg");
    }

    public void testBudgetone200() throws Exception {
        assertProfile("gsPhoneBt200", "expected-gsbt200.cfg");
    }

    public void testGxp1200() throws Exception {
        assertProfile("gsPhoneGxp1200", "expected-gxp1200.cfg");
    }

    public void testGxp2000() throws Exception {
        assertProfile("gsPhoneGxp2000", "expected-gxp2000.cfg");
    }

    public void testGxp2010() throws Exception {
        assertProfile("gsPhoneGxp2010", "expected-gxp2010.cfg");
    }

    public void testGxp2020() throws Exception {
        assertProfile("gsPhoneGxp2020", "expected-gxp2020.cfg");
    }

    public void testGxv3000() throws Exception {
        assertProfile("gsPhoneGxv3000", "expected-gxv3000.cfg");
    }

    public void testGxw4004() throws Exception {
        assertProfile("gsFxsGxw4004", "expected-gxw4004.cfg");
    }

    protected void assertProfile(String modelId, String expectedProfile) throws IOException {
        GrandstreamModel model = new GrandstreamModel();
        model.setModelDir("grandstream");
        model.setMaxLineCount(1);
        model.setModelId(modelId);
        phone = new GrandstreamPhone();
        phone.setModel(model);
        tester = PhoneTestDriver.supplyTestData(phone);
        phone.setTextFormatEnabled(true);
        ByteArrayOutputStream profile = new ByteArrayOutputStream();
        GrandstreamProfileWriter pwtr = new GrandstreamProfileWriter(phone);
        pwtr.write(profile);
        String expected = IOUtils.toString(this.getClass().getResourceAsStream(expectedProfile));
        String actual = TestUtil.cleanEndOfLines(profile.toString());
        assertEquals(expected, actual);
    }
}
