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
package org.sipfoundry.sipxconfig.time;

import static org.junit.Assert.assertEquals;

import java.io.IOException;
import java.io.StringWriter;
import java.util.ArrayList;
import java.util.List;

import org.apache.commons.io.IOUtils;
import org.apache.commons.net.util.SubnetUtils;
import org.apache.commons.net.util.SubnetUtils.SubnetInfo;
import org.junit.Test;

public class NtpConfigTest {
    private NtpConfig m_config = new NtpConfig();

    @Test
    public void writeNtpYaml() throws IOException {
        StringWriter actual = new StringWriter();
        List<String> servers = new ArrayList<String>();
        servers.add("0.pool.ntp.org");
        servers.add("1.pool.ntp.org");
        servers.add("2.pool.ntp.org");
        servers.add("3.pool.ntp.org");
        List<SubnetInfo> subnetInfo = new ArrayList<SubnetUtils.SubnetInfo>();
        subnetInfo.add(new SubnetUtils("192.168.0.5/16").getInfo());
        subnetInfo.add(new SubnetUtils("10.5.1.0/32").getInfo());
        m_config.writeNtpdConfig(actual, true, servers, true, subnetInfo, true, true, "/var/lib/ntp/drift");
        String expected = IOUtils.toString(getClass().getResourceAsStream("ntp-test.yml"));
        assertEquals(expected, actual.toString());
    }
}
