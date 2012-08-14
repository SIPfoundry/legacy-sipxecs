/**
 *
 *
 * Copyright (c) 2012 eZuce, Inc. All rights reserved.
 * Contributed to SIPfoundry under a Contributor Agreement
 *
 * This software is free software; you can redistribute it and/or modify it under
 * the terms of the Affero General Public License (AGPL) as published by the
 * Free Software Foundation; either version 3 of the License, or (at your option)
 * any later version.
 *
 * This software is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more
 * details.
 */
package org.sipfoundry.sipxconfig.recording;

import static org.junit.Assert.assertEquals;

import java.io.StringWriter;
import java.util.ArrayList;
import java.util.List;

import org.apache.commons.io.IOUtils;
import org.junit.Test;
import org.sipfoundry.sipxconfig.address.Address;
import org.sipfoundry.sipxconfig.imbot.ImBot;
import org.sipfoundry.sipxconfig.ivr.Ivr;
import org.sipfoundry.sipxconfig.test.TestHelper;


public class RecordingConfigurationTest {

    @Test
    public void testConfig() throws Exception {
        RecordingConfig config = new RecordingConfig();
        RecordingSettings settings = new RecordingSettings();
        settings.setModelFilesContext(TestHelper.getModelFilesContext());
        StringWriter actual = new StringWriter();
        Address imbotAddress = new Address(ImBot.REST_API, "host.example.org", 8076);
        Address ivrAddress1 = new Address(Ivr.REST_API, "host.example.org", 8086);
        Address ivrAddress2 = new Address(Ivr.REST_API, "host2.example.org", 8086);
        List<Address> ivrAddresses = new ArrayList<Address>();
        ivrAddresses.add(ivrAddress1);
        ivrAddresses.add(ivrAddress2);
        config.write(actual, settings, imbotAddress, ivrAddresses, "mp3");
        String expected = IOUtils.toString(getClass().getResourceAsStream("expected-sipxrecording.properties"));
        assertEquals(expected, actual.toString());
    }
}
