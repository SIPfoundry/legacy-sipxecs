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

import java.sql.Connection;
import java.sql.SQLException;
import java.sql.Statement;
import java.util.List;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import org.hibernate.classic.Session;
import org.sipfoundry.sipxconfig.common.SipxHibernateDaoSupport;
import org.sipfoundry.sipxconfig.common.UserException;
import org.springframework.beans.factory.annotation.Required;

public class OpenAcdSkillGroupMigrationContextImpl extends SipxHibernateDaoSupport implements
        OpenAcdSkillGroupMigrationContext {

    public static final Log LOG = LogFactory.getLog(OpenAcdSkillGroupMigrationContextImpl.class);

    private static final String SQL = "alter table openacd_skill drop column group_name";

    private OpenAcdContext m_openAcdContext;

    @Override
    public void migrateSkillGroup() {
        List skills = getHibernateTemplate().findByNamedQuery("allSkills");
        for (int i = 0; i < skills.size(); i++) {
            Object[] row = (Object[]) skills.get(i);
            Integer skillId = (Integer) row[0];
            String skillGroupName = (String) row[1];
            if (skillId == null || skillGroupName == null) {
                continue;
            }
            try {
                OpenAcdSkill skill = m_openAcdContext.getSkillById(skillId);
                OpenAcdSkillGroup existingSkillGroup = m_openAcdContext.getSkillGroupByName(skillGroupName);
                OpenAcdSkillGroup skillGroup = (existingSkillGroup != null) ? existingSkillGroup
                        : createAssociateSkillGroup(skillGroupName);
                skill.setGroup(skillGroup);
                m_openAcdContext.saveSkill(skill);
                getHibernateTemplate().flush();
            } catch (UserException e) {
                LOG.warn("Cannot migrate skill group names", e);
            }
        }

        cleanSchema();
    }

    private OpenAcdSkillGroup createAssociateSkillGroup(String skillGroupName) {
        OpenAcdSkillGroup skillGroup = new OpenAcdSkillGroup();
        skillGroup.setName(skillGroupName);
        skillGroup.setDescription(skillGroupName + " skill group");
        m_openAcdContext.saveSkillGroup(skillGroup);
        getHibernateTemplate().flush();
        return skillGroup;
    }

    private void cleanSchema() {
        try {
            Session currentSession = getHibernateTemplate().getSessionFactory().getCurrentSession();
            Connection connection = currentSession.connection();
            Statement statement = connection.createStatement();
            statement.addBatch(SQL);
            statement.executeBatch();
            statement.close();
        } catch (SQLException e) {
            LOG.warn("cleaning schema error", e);
        }
    }

    @Required
    public void setOpenAcdContext(OpenAcdContext openAcdContext) {
        m_openAcdContext = openAcdContext;
    }
}
