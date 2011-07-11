/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 *
 */
package org.sipfoundry.sipxconfig.speeddial;

import static org.sipfoundry.sipxconfig.admin.AbstractConfigurationFile.getFileContent;

import java.io.InputStream;

import org.apache.commons.io.IOUtils;
import org.sipfoundry.sipxconfig.IntegrationTestCase;

public class ResourceListsTestIntegration extends IntegrationTestCase {
    private ResourceLists m_resourceList;

    public void testGenerate() throws Exception {
        loadDataSetXml("domain/DomainSeed.xml");
        loadDataSet("speeddial/ResourceListsSeed.xml");
        String fileContent = getFileContent(m_resourceList, null);
        InputStream referenceXml = getClass().getResourceAsStream("resource-lists.test.xml");
        assertEquals(IOUtils.toString(referenceXml), fileContent);
    }

    public void testGenerateEmpty() throws Exception {
        loadDataSetXml("domain/DomainSeed.xml");
        String fileContent = getFileContent(m_resourceList, null);
        InputStream referenceXml = getClass().getResourceAsStream("empty-resource-lists.test.xml");
        assertEquals(IOUtils.toString(referenceXml), fileContent);
    }

    public void testGenerateConsideringSubscribePermission() throws Exception {
        loadDataSetXml("domain/DomainSeed.xml");
        loadDataSet("speeddial/ResourceListsPermissionSeed.xml");
        String fileContent = getFileContent(m_resourceList, null);
        InputStream referenceXml = getClass().getResourceAsStream("resource-lists-considering-permission.test.xml");
        assertEquals(IOUtils.toString(referenceXml), fileContent);
    }

    public void setResourceListGenerator(ResourceLists lists) {
        m_resourceList = lists;
    }

}
