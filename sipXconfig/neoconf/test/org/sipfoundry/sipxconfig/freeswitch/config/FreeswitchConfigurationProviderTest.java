/**
 *
 *
 * Copyright (c) 2013 eZuce, Inc. All rights reserved.
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
package org.sipfoundry.sipxconfig.freeswitch.config;

import static org.junit.Assert.assertEquals;

import java.io.IOException;
import java.io.StringWriter;
import java.util.Arrays;
import java.util.Collections;
import java.util.List;

import org.junit.Before;
import org.junit.Test;
import org.sipfoundry.sipxconfig.commserver.Location;
import org.sipfoundry.sipxconfig.freeswitch.FreeswitchFeature;

public class FreeswitchConfigurationProviderTest {
    FreeswitchConfigurationProvider m_provider;

    @Before
    public void setUp() {
        m_provider = new FreeswitchConfigurationProvider();  
    }

    @Test
    public void test() throws IOException {
        StringWriter actual = new StringWriter();
        Location l = new Location();
        FreeswitchProvider p = new FreeswitchProvider() {
            
            @Override
            public List<String> getRequiredModules(FreeswitchFeature feature, Location location) {
                return Arrays.asList(new String[] {"aaa", "bbb"});
            }
        };
        m_provider.writeModsParts(actual, Collections.singleton(p), l);
        assertEquals("<load module=\"aaa\"/>\n<load module=\"bbb\"/>\n", actual.toString());
    }
}
