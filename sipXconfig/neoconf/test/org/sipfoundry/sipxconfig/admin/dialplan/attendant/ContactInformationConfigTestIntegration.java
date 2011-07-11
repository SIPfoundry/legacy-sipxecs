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
package org.sipfoundry.sipxconfig.admin.dialplan.attendant;

import static org.sipfoundry.sipxconfig.admin.AbstractConfigurationFile.getFileContent;

import java.io.InputStream;

import org.apache.commons.io.IOUtils;
import org.sipfoundry.sipxconfig.IntegrationTestCase;


public class ContactInformationConfigTestIntegration extends IntegrationTestCase {
    private ContactInformationConfig m_config;

    public void testGenerate() throws Exception {
        loadDataSet("admin/dialplan/attendant/ContactInformationSeed.xml");
        String generatedXml = getFileContent(m_config, null);
        InputStream referenceXml = getClass().getResourceAsStream("contact-information.test.xml");
        assertEquals(IOUtils.toString(referenceXml), generatedXml);
    }

    public void setContactInformationConfig(ContactInformationConfig config) {
        m_config = config;
    }

}
