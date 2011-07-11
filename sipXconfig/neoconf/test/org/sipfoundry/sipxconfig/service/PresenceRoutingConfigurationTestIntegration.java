/*
 *
 *
 * Copyright (C) 2009 Nortel, certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 *
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
