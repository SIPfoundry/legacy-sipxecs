/*
 *
 *
 * Copyright (C) 2009 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 *
 */
package org.sipfoundry.sipxconfig.acd.stats.historical;

import static org.junit.Assert.assertEquals;

import java.io.StringWriter;
import java.util.Arrays;
import java.util.Collection;

import org.apache.commons.io.IOUtils;
import org.junit.Test;
import org.sipfoundry.sipxconfig.acd.stats.AcdStats;
import org.sipfoundry.sipxconfig.address.Address;
import org.sipfoundry.sipxconfig.test.TestHelper;


public class AcdReportConfigTest {

    @Test
    public void testReplicateReportConfig() throws Exception {
        AcdHistoryConfig config = new AcdHistoryConfig();
        AcdHistoricalSettings settings = new AcdHistoricalSettings();
        settings.setModelFilesContext(TestHelper.getModelFilesContext());
        StringWriter actual = new StringWriter();
        String expected = IOUtils.toString(getClass().getResourceAsStream("report-config-expected.xml"));
        Collection<Address> apis = Arrays.asList(new Address(AcdStats.API_ADDRESS, "one"), new Address(AcdStats.API_ADDRESS, "two"));
        config.write(actual, settings, apis);
        assertEquals(expected, actual.toString());
    }
}
