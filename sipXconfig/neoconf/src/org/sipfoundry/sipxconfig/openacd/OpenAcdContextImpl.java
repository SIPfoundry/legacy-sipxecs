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
import java.util.List;

import org.sipfoundry.sipxconfig.admin.NameInUseException;
import org.sipfoundry.sipxconfig.admin.forwarding.AliasMapping;
import org.sipfoundry.sipxconfig.alias.AliasManager;
import org.sipfoundry.sipxconfig.common.BeanId;
import org.sipfoundry.sipxconfig.common.SipUri;
import org.sipfoundry.sipxconfig.common.SipxHibernateDaoSupport;
import org.sipfoundry.sipxconfig.domain.DomainManager;
import org.sipfoundry.sipxconfig.service.SipxFreeswitchService;
import org.sipfoundry.sipxconfig.service.SipxServiceManager;
import org.springframework.dao.support.DataAccessUtils;

public class OpenAcdContextImpl extends SipxHibernateDaoSupport implements OpenAcdContext {

    private static final String VALUE = "value";
    private static final String OPEN_ACD_WITH_ALIAS = "openAcdIdsWithAlias";
    private static final String OPEN_ACD_EXTENSION_WITH_NAME = "openAcdExtensionWithName";

    private static final String ALIAS_RELATION = "openacd";

    private DomainManager m_domainManager;
    private SipxServiceManager m_serviceManager;
    private boolean m_enabled = true;
    private AliasManager m_aliasManager;

    @Override
    public OpenAcdExtension getExtensionById(Integer extensionId) {
        return getHibernateTemplate().load(OpenAcdExtension.class, extensionId);
    }

    @Override
    public OpenAcdExtension getExtensionByName(String extensionName) {
        List<OpenAcdExtension> extensions = getHibernateTemplate()
            .findByNamedQueryAndNamedParam(OPEN_ACD_EXTENSION_WITH_NAME, VALUE, extensionName);
        return (OpenAcdExtension) DataAccessUtils.singleResult(extensions);
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

    public void setDomainManager(DomainManager manager) {
        m_domainManager = manager;
    }

    public void setSipxServiceManager(SipxServiceManager manager) {
        m_serviceManager = manager;
    }

    public void setAliasManager(AliasManager aliasManager) {
        m_aliasManager = aliasManager;
    }
}
