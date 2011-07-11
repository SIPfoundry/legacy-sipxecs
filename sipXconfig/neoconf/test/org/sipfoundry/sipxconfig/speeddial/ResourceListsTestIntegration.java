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
