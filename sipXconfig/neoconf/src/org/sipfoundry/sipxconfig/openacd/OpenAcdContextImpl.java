/*
 *
 *
 * Copyright (C) 2010 eZuce, Inc. All rights reserved.
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

import java.util.ArrayList;
import java.util.Collection;
import java.util.Collections;
import java.util.LinkedList;
import java.util.List;

import org.apache.commons.collections.CollectionUtils;
import org.apache.commons.lang.StringUtils;
import org.sipfoundry.sipxconfig.admin.NameInUseException;
import org.sipfoundry.sipxconfig.admin.forwarding.AliasMapping;
import org.sipfoundry.sipxconfig.alias.AliasManager;
import org.sipfoundry.sipxconfig.common.BeanId;
import org.sipfoundry.sipxconfig.common.DaoUtils;
import org.sipfoundry.sipxconfig.common.SipUri;
import org.sipfoundry.sipxconfig.common.SipxHibernateDaoSupport;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.common.UserException;
import org.sipfoundry.sipxconfig.domain.DomainManager;
import org.sipfoundry.sipxconfig.service.SipxFreeswitchService;
import org.sipfoundry.sipxconfig.service.SipxServiceManager;
import org.springframework.dao.support.DataAccessUtils;

public class OpenAcdContextImpl extends SipxHibernateDaoSupport implements OpenAcdContext {

    private static final String VALUE = "value";
    private static final String OPEN_ACD_WITH_ALIAS = "openAcdIdsWithAlias";
    private static final String OPEN_ACD_EXTENSION_WITH_NAME = "openAcdExtensionWithName";
    private static final String OPEN_ACD_AGENT_GROUP_WITH_NAME = "openAcdAgentGroupWithName";
    private static final String ALIAS_RELATION = "openacd";
    private static final String OPEN_ACD_AGENT_BY_USERID = "openAcdAgentByUserId";
    private static final String GROUP_NAME_DEFAULT = "Default";

    private DomainManager m_domainManager;
    private SipxServiceManager m_serviceManager;
    private boolean m_enabled = true;
    private AliasManager m_aliasManager;
    private OpenAcdProvisioningContext m_provisioningContext;

    @Override
    public OpenAcdExtension getExtensionById(Integer extensionId) {
        return getHibernateTemplate().load(OpenAcdExtension.class, extensionId);
    }

    @Override
    public OpenAcdExtension getExtensionByName(String extensionName) {
        List<OpenAcdExtension> extensions = getHibernateTemplate().findByNamedQueryAndNamedParam(
                OPEN_ACD_EXTENSION_WITH_NAME, VALUE, extensionName);
        return DataAccessUtils.singleResult(extensions);
    }

    @Override
    public List<OpenAcdExtension> getFreeswitchExtensions() {
        return getHibernateTemplate().loadAll(OpenAcdExtension.class);
    }

    @Override
    public boolean isEnabled() {
        return m_enabled;
    }

    @Override
    public void removeExtensions(List<Integer> extensionIds) {
        removeAll(OpenAcdExtension.class, extensionIds);
    }

    @Override
    public void saveExtension(OpenAcdExtension extension) {
        if (extension.isNew() || (!extension.isNew() && isNameChanged(extension))) {
            if (!m_aliasManager.canObjectUseAlias(extension, extension.getName())) {
                throw new NameInUseException(ALIAS_RELATION, extension.getName());
            }
        }
        getHibernateTemplate().merge(extension);
    }

    private boolean isNameChanged(OpenAcdExtension extension) {
        return !getExtensionById(extension.getId()).getName().equals(extension.getName());
    }

    @Override
    public Collection getBeanIdsOfObjectsWithAlias(String alias) {
        Collection ids = getHibernateTemplate().findByNamedQueryAndNamedParam(OPEN_ACD_WITH_ALIAS, VALUE, alias);
        Collection bids = BeanId.createBeanIdCollection(ids, OpenAcdExtension.class);
        return bids;
    }

    @Override
    public boolean isAliasInUse(String alias) {
        List confIds = getHibernateTemplate().findByNamedQueryAndNamedParam(OPEN_ACD_WITH_ALIAS, VALUE, alias);
        return !confIds.isEmpty();
    }

    @Override
    public Collection<AliasMapping> getAliasMappings() {
        List<OpenAcdExtension> extensions = getHibernateTemplate().loadAll(OpenAcdExtension.class);
        final ArrayList<AliasMapping> list = new ArrayList<AliasMapping>();
        String domainName = m_domainManager.getDomain().getName();
        for (OpenAcdExtension extension : extensions) {
            list.add(generateAlias(extension.getName(), domainName));
        }
        return list;
    }

    private AliasMapping generateAlias(String name, String domainName) {
        SipxFreeswitchService freeswitchService = (SipxFreeswitchService) m_serviceManager
                .getServiceByBeanId(SipxFreeswitchService.BEAN_ID);
        AliasMapping mapping = new AliasMapping();
        mapping.setIdentity(AliasMapping.createUri(name, domainName));
        mapping.setContact(SipUri.format(name, freeswitchService.getAddress(), freeswitchService
                .getFreeswitchSipPort()));
        mapping.setRelation(ALIAS_RELATION);
        return mapping;
    }

    public List<OpenAcdAgentGroup> getAgentGroups() {
        return getHibernateTemplate().loadAll(OpenAcdAgentGroup.class);
    }

    public OpenAcdAgentGroup getAgentGroupById(Integer agentGroupId) {
        return getHibernateTemplate().load(OpenAcdAgentGroup.class, agentGroupId);
    }

    public OpenAcdAgentGroup getAgentGroupByName(String agentGroupName) {
        List<OpenAcdAgentGroup> agentGroups = getHibernateTemplate().findByNamedQueryAndNamedParam(
                OPEN_ACD_AGENT_GROUP_WITH_NAME, VALUE, agentGroupName);
        return DataAccessUtils.singleResult(agentGroups);
    }

    public void saveAgentGroup(OpenAcdAgentGroup agentGroup) {
        // check if agent group name is empty
        if (StringUtils.isBlank(agentGroup.getName())) {
            throw new UserException("&blank.agentGroupName.error");
        }
        // Check for duplicate names before saving the agent group
        if (agentGroup.isNew() || (!agentGroup.isNew() && isNameChanged(agentGroup))) {
            checkForDuplicateName(agentGroup);
        }

        if (!agentGroup.isNew()) {
            if (isNameChanged(agentGroup)) {
                // don't rename the default group
                OpenAcdAgentGroup defaultAgentGroup = getAgentGroupByName(GROUP_NAME_DEFAULT);
                if (defaultAgentGroup != null && defaultAgentGroup.getId().equals(agentGroup.getId())) {
                    throw new UserException("&msg.err.defaultAgentGroupRename");
                }
            }
            getHibernateTemplate().merge(agentGroup);
            m_provisioningContext.updateObjects(Collections.singletonList(agentGroup));
        } else {
            getHibernateTemplate().save(agentGroup);
            m_provisioningContext.addObjects(Collections.singletonList(agentGroup));
        }
    }

    private boolean isNameChanged(OpenAcdAgentGroup agentGroup) {
        String oldName = getAgentGroupById(agentGroup.getId()).getName();
        agentGroup.setOldName(oldName);
        return !oldName.equals(agentGroup.getName());
    }

    private void checkForDuplicateName(OpenAcdAgentGroup agentGroup) {
        String agentGroupName = agentGroup.getName();
        OpenAcdAgentGroup existingAgentGroup = getAgentGroupByName(agentGroupName);
        if (existingAgentGroup != null) {
            throw new UserException("&duplicate.agentGroupName.error", agentGroupName);
        }
    }

    public boolean removeAgentGroups(Collection<Integer> agentGroupIds) {
        boolean affectDefaultAgentGroup = false;
        List<OpenAcdAgentGroup> groups = new LinkedList<OpenAcdAgentGroup>();
        List<OpenAcdAgent> agents = new LinkedList<OpenAcdAgent>();
        for (Integer id : agentGroupIds) {
            OpenAcdAgentGroup group = getAgentGroupById(id);
            if (!group.getName().equals(GROUP_NAME_DEFAULT)) {
                agents.addAll(group.getAgents());
                groups.add(group);
            } else {
                affectDefaultAgentGroup = true;
            }
        }
        getHibernateTemplate().deleteAll(groups);
        m_provisioningContext.deleteObjects(agents);
        m_provisioningContext.deleteObjects(groups);

        return affectDefaultAgentGroup;
    }

    public List<OpenAcdAgent> addAgentsToGroup(OpenAcdAgentGroup agentGroup, Collection<OpenAcdAgent> agents) {
        List<OpenAcdAgent> existingAgents = new ArrayList<OpenAcdAgent>();
        for (OpenAcdAgent agent : agents) {
            if (!isOpenAcdAgent(agent.getUser())) {
                agentGroup.addAgent(agent);
                agent.setGroup(agentGroup);
            } else {
                existingAgents.add(agent);
            }
        }
        // update the group
        saveAgentGroup(agentGroup);
        m_provisioningContext.addObjects(new LinkedList<OpenAcdConfigObject>(CollectionUtils.subtract(agents,
                existingAgents)));

        return existingAgents;
    }

    public List<OpenAcdAgent> getAgents() {
        return getHibernateTemplate().loadAll(OpenAcdAgent.class);
    }

    public OpenAcdAgent getAgentById(Integer agentId) {
        return getHibernateTemplate().load(OpenAcdAgent.class, agentId);
    }

    public OpenAcdAgent getAgentByUserId(Integer userId) {
        Collection<OpenAcdAgent> agents = getHibernateTemplate().findByNamedQueryAndNamedParam(
                OPEN_ACD_AGENT_BY_USERID, VALUE, userId);
        return DaoUtils.requireOneOrZero(agents, OPEN_ACD_AGENT_BY_USERID);
    }

    public void saveAgent(OpenAcdAgentGroup agentGroup, OpenAcdAgent agent) {
        checkAgent(agent);
        addAgentsToGroup(agentGroup, Collections.singletonList(agent));
    }

    @Override
    public void saveAgent(OpenAcdAgent agent) {
        checkAgent(agent);
        getHibernateTemplate().saveOrUpdate(agent);
        agent.setOldName(agent.getName());
        m_provisioningContext.updateObjects(Collections.singletonList(agent));
    }

    private void checkAgent(OpenAcdAgent agent) {
        if (StringUtils.isBlank(agent.getPin())) {
            throw new UserException("&blank.agentPin.error");
        }
        // check if agent security is empty
        if (StringUtils.isBlank(agent.getSecurity())) {
            throw new UserException("&blank.agentSecurity.error");
        }
    }

    @Override
    public void deleteAgents(Integer groupId, Collection<Integer> agentIds) {
        OpenAcdAgentGroup group = getAgentGroupById(groupId);
        List<OpenAcdAgent> agents = new LinkedList<OpenAcdAgent>();
        for (Integer id : agentIds) {
            OpenAcdAgent agent = getAgentById(id);
            group.removeAgent(agent);
            agents.add(agent);
        }
        getHibernateTemplate().save(group);
        m_provisioningContext.deleteObjects(agents);
    }

    @Override
    public OpenAcdAgent getAgentByUser(User user) {
        return getAgentByUserId(user.getId());
    }

    @Override
    public boolean isOpenAcdAgent(User user) {
        if (getAgentByUser(user) != null) {
            return true;
        }
        return false;
    }

    public void setDomainManager(DomainManager manager) {
        m_domainManager = manager;
    }

    public void setSipxServiceManager(SipxServiceManager manager) {
        m_serviceManager = manager;
    }

    public void setAliasManager(AliasManager aliasManager) {
        m_aliasManager = aliasManager;
    }

    public void setProvisioningContext(OpenAcdProvisioningContext context) {
        m_provisioningContext = context;
    }

}
