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
        assertProfile("gsHt496", "expected-ht.cfg");
    }

    public void testBudgetone() throws Exception {
        assertProfile("gsPhoneBt", "expected-gsbt.cfg");
    }

    public void testGxs2000() throws Exception {
        assertProfile("gsPhoneGxp", "expected-gxp.cfg");
    }

    public void testGxv3000() throws Exception {
        assertProfile("gsPhoneGxv3000", "expected-gxv.cfg");
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
