/**
 *
 *
 * Copyright (c) 2010 / 2011 eZuce, Inc. All rights reserved.
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
package org.sipfoundry.sipxconfig.service;

import java.io.InputStream;

import org.apache.commons.io.IOUtils;
import org.dom4j.Document;
import org.sipfoundry.sipxconfig.IntegrationTestCase;
import org.sipfoundry.sipxconfig.test.TestUtil;

public class PresenceRoutingConfigurationTestIntegration extends IntegrationTestCase {
    private PresenceRoutingConfiguration m_routing;

    public void testXmlGeneration() throws Exception {
        loadDataSet("service/PresenceRoutingSeed.xml");
        Document document = m_routing.getDocument();
        String domDoc = TestUtil.asString(document);

        InputStream referenceXml = PresenceRoutingConfiguration.class
                .getResourceAsStream("presencerouting-prefs.test.xml");
        assertEquals(IOUtils.toString(referenceXml), domDoc);
    }

    public void setPresenceRoutingConfiguration(PresenceRoutingConfiguration config) {
        m_routing = config;
    }
}
