/*
 *
 *
 * Copyright (C) 2011 eZuce, Inc. All rights reserved.
 *
 * This library is free software; you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License as published by the Free
 * Software Foundation; either version 2.1 of the License, or (at your option)
 * any later version.
 *
 * This library is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more
 * details.
 */
package org.sipfoundry.sipxconfig.openacd;

import org.sipfoundry.sipxconfig.common.InitTaskListener;
import org.springframework.beans.factory.annotation.Required;

public class OpenAcdSkillGroupMigrationTrigger extends InitTaskListener {
    private OpenAcdSkillGroupMigrationContext m_openAcdSkillGroupMigrationContext;

    @Override
    public void onInitTask(String task) {
        if ("skill_group_name_migrate_skill_group".equals(task)) {
            m_openAcdSkillGroupMigrationContext.migrateSkillGroup();
        }
    }

    @Required
    public void setOpenAcdSkillGroupMigrationContext(
            OpenAcdSkillGroupMigrationContext openAcdSkillGroupMigrationContext) {
        m_openAcdSkillGroupMigrationContext = openAcdSkillGroupMigrationContext;
    }
}
