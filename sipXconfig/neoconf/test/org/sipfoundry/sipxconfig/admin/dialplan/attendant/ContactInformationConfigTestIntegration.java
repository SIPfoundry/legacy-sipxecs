/*
 *
 *
 * Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
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
