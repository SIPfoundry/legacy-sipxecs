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
package org.sipfoundry.sipxconfig.openacd;

import org.sipfoundry.sipxconfig.test.IntegrationTestCase;

public class OpenAcdSkillGroupMigrationContextTestIntegration extends IntegrationTestCase {
    private OpenAcdSkillGroupMigrationContext m_migrationContext;
    private OpenAcdContextImpl m_openAcdContextImpl;

    public void testMigrateSkillGroup() throws Exception {
        assertEquals(0, m_openAcdContextImpl.getSkillGroups().size());

        m_migrationContext.migrateSkillGroup();

        // test existing 'Language' and 'Magic' skill groups
        assertEquals(2, m_openAcdContextImpl.getSkillGroups().size());
        OpenAcdSkillGroup magicSkillGroup = m_openAcdContextImpl.getSkillGroupByName("Magic");
        assertNotNull(magicSkillGroup);
        assertEquals("Magic", magicSkillGroup.getName());
        OpenAcdSkillGroup languageSkillGroup = m_openAcdContextImpl.getSkillGroupByName("Language");
        assertNotNull(languageSkillGroup);
        assertEquals("Language", languageSkillGroup.getName());
    }

    public void setOpenAcdSkillGroupMigrationContext(OpenAcdSkillGroupMigrationContext migrationContext) {
        m_migrationContext = migrationContext;
    }

    public void setOpenAcdContextImpl(OpenAcdContextImpl openAcdContext) {
        m_openAcdContextImpl = openAcdContext;
    }
}
