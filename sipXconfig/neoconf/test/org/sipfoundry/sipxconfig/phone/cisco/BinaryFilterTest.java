/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.phone.cisco;

import java.io.ByteArrayOutputStream;
import java.io.File;
import java.io.InputStream;

import junit.framework.TestCase;

import org.apache.commons.io.IOUtils;

public class BinaryFilterTest extends TestCase {

    protected void setUp() throws Exception {

    }

    public void testCfgmtUtility() throws Exception {
        BinaryFilter filter = new BinaryFilter("/etc/ciscoAta", null);
        assertEquals("/etc/ciscoAta/cfgfmt", filter.getCfgfmtUtility());
    }

    public void testPtagDat() throws Exception {
        CiscoModel model = new CiscoModel();
        model.setCfgPrefix("gk");
        BinaryFilter filter = new BinaryFilter("/etc/ciscoAta", model);
        assertEquals("/etc/ciscoAta/gk-ptag.dat", filter.getPtagDat());
    }

    public void testCopy() throws Exception {
        File cfgfmt = new File(getClass().getResource("cfgfmt").getFile());
        assertTrue(cfgfmt.exists());

        CiscoModel model = new CiscoModel();
        model.setCfgPrefix("gk");

        BinaryFilter filter = new BinaryFilter(cfgfmt.getParent(), model);

        InputStream in = getClass().getResourceAsStream("expected-7960.cfg");

        ByteArrayOutputStream out = new ByteArrayOutputStream();
        filter.copy(in, out);

        in = getClass().getResourceAsStream("expected-7960.cfg");
        String expected = IOUtils.toString(in);

        assertEquals(expected, out.toString("UTF-8"));
    }
}
