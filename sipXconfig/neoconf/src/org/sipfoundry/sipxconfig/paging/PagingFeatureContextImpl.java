/**
 *
 *
 * Copyright (c) 2014 eZuce, Inc. All rights reserved.
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
package org.sipfoundry.sipxconfig.paging;

import static org.sipfoundry.sipxconfig.paging.PagingContext.FEATURE;

import java.util.Collection;
import java.util.Collections;

import org.sipfoundry.sipxconfig.cfgmgt.ConfigManager;
import org.sipfoundry.sipxconfig.common.SipxHibernateDaoSupport;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.common.event.DaoEventListener;
import org.sipfoundry.sipxconfig.commserver.Location;
import org.sipfoundry.sipxconfig.dialplan.DialPlanContext;
import org.sipfoundry.sipxconfig.feature.Bundle;
import org.sipfoundry.sipxconfig.feature.FeatureChangeRequest;
import org.sipfoundry.sipxconfig.feature.FeatureChangeValidator;
import org.sipfoundry.sipxconfig.feature.FeatureManager;
import org.sipfoundry.sipxconfig.feature.FeatureProvider;
import org.sipfoundry.sipxconfig.feature.GlobalFeature;
import org.sipfoundry.sipxconfig.feature.LocationFeature;
import org.sipfoundry.sipxconfig.proxy.ProxyManager;
import org.springframework.jdbc.core.JdbcTemplate;

public class PagingFeatureContextImpl extends SipxHibernateDaoSupport<PagingGroup> implements FeatureProvider,
        DaoEventListener, PagingFeatureContext {
    private ConfigManager m_configManager;
    private JdbcTemplate m_jdbc;
    private PagingContext m_pagingContext;

    @Override
    public Collection<GlobalFeature> getAvailableGlobalFeatures(FeatureManager featureManager) {
        return Collections.emptyList();
    }

    @Override
    public Collection<LocationFeature> getAvailableLocationFeatures(FeatureManager featureManager, Location l) {
        return Collections.singleton(FEATURE);
    }

    @Override
    public void getBundleFeatures(FeatureManager featureManager, Bundle b) {
        if (b == Bundle.CORE_TELEPHONY) {
            b.addFeature(FEATURE);
        }
    }

    @Override
    public void featureChangePrecommit(FeatureManager manager, FeatureChangeValidator validator) {
        validator.requiresAtLeastOne(FEATURE, ProxyManager.FEATURE);
        validator.singleLocationOnly(FEATURE);
    }

    @Override
    public void featureChangePostcommit(FeatureManager manager, FeatureChangeRequest request) {
        if (request.getAllNewlyEnabledFeatures().contains(FEATURE)) {
            PagingSettings settings = m_pagingContext.getSettings();
            if (settings.isNew()) {
                m_pagingContext.saveSettings(settings);
            }
        }

        if (request.hasChanged(FEATURE)) {
            m_configManager.configureEverywhere(DialPlanContext.FEATURE);
        }
    }

    /*
     * (non-Javadoc)
     *
     * @see
     * org.sipfoundry.sipxconfig.paging.PagingFeatureContext#deletePagingGroupsById(java.util.
     * Collection)
     */
    @Override
    public void deletePagingGroupsById(Collection<Integer> groupsIds) {
        if (groupsIds != null && groupsIds.size() > 0) {
            removeAll(PagingGroup.class, groupsIds);
            m_configManager.configureEverywhere(FEATURE);
        }
    }

    @Override
    public void onDelete(Object entity) {
        if (entity instanceof User) {
            User user = (User) entity;
            // check if users in any paging group
            int check = m_jdbc.queryForInt("SELECT count(*) FROM user_paging_group where user_id = ?", user.getId());
            if (check >= 1) {
                // cleanup paging group
                m_jdbc.update("delete from user_paging_group where user_id = ?", user.getId());
                // reconfigure paging group
                m_configManager.configureEverywhere(PagingContext.FEATURE);
            }
        }
    }

    @Override
    public void onSave(Object entity) {
    }

    public void setPagingContext(PagingContext pagingContext) {
        m_pagingContext = pagingContext;
    }

    public void setConfigManager(ConfigManager configManager) {
        m_configManager = configManager;
    }

    public void setJdbc(JdbcTemplate jdbc) {
        m_jdbc = jdbc;
    }
}
