/**
 * Copyright (c) 2012 eZuce, Inc. All rights reserved.
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
package org.sipfoundry.sipxconfig.firewall;

import java.sql.PreparedStatement;
import java.sql.ResultSet;
import java.sql.SQLException;
import java.sql.Types;
import java.util.ArrayList;
import java.util.Collection;
import java.util.Collections;
import java.util.List;
import java.util.Map;

import org.apache.commons.collections.CollectionUtils;
import org.apache.commons.collections.Predicate;
import org.sipfoundry.sipxconfig.address.AddressManager;
import org.sipfoundry.sipxconfig.address.AddressType;
import org.sipfoundry.sipxconfig.common.SipxHibernateDaoSupport;
import org.sipfoundry.sipxconfig.commserver.Location;
import org.sipfoundry.sipxconfig.feature.Bundle;
import org.sipfoundry.sipxconfig.feature.FeatureProvider;
import org.sipfoundry.sipxconfig.feature.GlobalFeature;
import org.sipfoundry.sipxconfig.feature.LocationFeature;
import org.sipfoundry.sipxconfig.setting.BeanWithSettingsDao;
import org.sipfoundry.sipxconfig.setup.SetupListener;
import org.sipfoundry.sipxconfig.setup.SetupManager;
import org.springframework.beans.factory.BeanFactory;
import org.springframework.beans.factory.BeanFactoryAware;
import org.springframework.beans.factory.ListableBeanFactory;
import org.springframework.jdbc.core.BatchPreparedStatementSetter;
import org.springframework.jdbc.core.JdbcTemplate;
import org.springframework.jdbc.core.RowMapper;

public class FirewallManagerImpl extends SipxHibernateDaoSupport implements FirewallManager, FeatureProvider,
        SetupListener, BeanFactoryAware {
    private BeanWithSettingsDao<FirewallSettings> m_settingsDao;
    private AddressManager m_addressManager;
    private List<FirewallProvider> m_providers;
    private ListableBeanFactory m_beanFactory;
    private JdbcTemplate m_jdbc;

    @Override
    public FirewallSettings getSettings() {
        return m_settingsDao.findOrCreateOne();
    }

    @Override
    public void saveSavings(FirewallSettings settings) {
        m_settingsDao.upsert(settings);
    }

    public void setSettingsDao(BeanWithSettingsDao<FirewallSettings> settingsDao) {
        m_settingsDao = settingsDao;
    }

    @Override
    public Collection<GlobalFeature> getAvailableGlobalFeatures() {
        return Collections.singleton(FEATURE);
    }

    @Override
    public Collection<LocationFeature> getAvailableLocationFeatures(Location l) {
        return null;
    }

    @Override
    public void getBundleFeatures(Bundle b) {
        if (b.isRouter()) {
            b.addFeature(FEATURE);
        }
    }

    @Override
    public void setup(SetupManager manager) {
        if (!manager.isSetup(FEATURE.getId())) {
            manager.getFeatureManager().enableGlobalFeature(FEATURE, true);
            manager.setSetup(FEATURE.getId());
        }
    }

    public List<ServerGroup> getServerGroups() {
        RowMapper<ServerGroup> reader = new RowMapper<ServerGroup>() {
            public ServerGroup mapRow(ResultSet rs, int arg1) throws SQLException {
                ServerGroup group = new ServerGroup();
                group.setName(rs.getString("name"));
                group.setServerList("servers");
                return group;
            }
        };
        String sql = "select * from firewall_server_group";
        List<ServerGroup> groups = m_jdbc.query(sql, reader);
        return groups;
    }

    public List<DefaultFirewallRule> getDefaultFirewallRules() {
        List<DefaultFirewallRule> rules = new ArrayList<DefaultFirewallRule>();
        for (AddressType type : m_addressManager.getAddressTypes()) {
            for (FirewallProvider provider : getProviders()) {
                DefaultFirewallRule rule = provider.getFirewallRule(this, type);
                if (rule == null) {
                    rule = new DefaultFirewallRule(type, FirewallRule.SystemId.CLUSTER, false);
                }
                rules.add(rule);
            }
        }
        return rules;
    }

    public List<EditableFirewallRule> getEditableFirewallRules() {
        List<EditableFirewallRule> editable = new ArrayList<EditableFirewallRule>();

        // TODO load from db

        for (DefaultFirewallRule def : getDefaultFirewallRules()) {
            editable.add(new EditableFirewallRule(def));
        }

        return editable;
    }

    List<FirewallProvider> getProviders() {
        if (m_providers == null) {
            Map<String, FirewallProvider> beanMap = m_beanFactory.getBeansOfType(FirewallProvider.class, false,
                    false);
            m_providers = new ArrayList<FirewallProvider>(beanMap.values());
        }
        return m_providers;
    }

    @Override
    public ServerGroup getPublicGroup() {
        // TODO
        return new ServerGroup();
    }

    @Override
    public ServerGroup getClusterGroup() {
        // TODO
        return new ServerGroup();
    }

    public void setAddressManager(AddressManager addressManager) {
        m_addressManager = addressManager;
    }

    @Override
    public void setBeanFactory(BeanFactory beanFactory) {
        m_beanFactory = (ListableBeanFactory) beanFactory;
    }

    @Override
    public List<FirewallRule> getFirewallRules() {
        return new ArrayList<FirewallRule>(getEditableFirewallRules());
    }

    @Override
    public void saveRules(List<EditableFirewallRule> rules) {
        m_jdbc.execute("delete from firewall_rule");
        String sql = "insert into firewall_rule (firewall_rule_id, firewall_server_group_id, "
                + "address_type, prioritize, system_id) values (nextval('firewall_rule_seq'),?,?,?,?)";
        Predicate selectChanged = new Predicate() {
            public boolean evaluate(Object arg0) {
                return ((EditableFirewallRule) arg0).isChangedFromDefault();
            }
        };

        Collection changedCol = CollectionUtils.select(rules, selectChanged);
        final EditableFirewallRule[] changed = (EditableFirewallRule[]) changedCol
                .toArray(new EditableFirewallRule[0]);
        if (changed.length == 0) {
            return;
        }
        BatchPreparedStatementSetter inserter = new BatchPreparedStatementSetter() {
            public int getBatchSize() {
                return changed.length;
            }

            public void setValues(PreparedStatement arg, int i) throws SQLException {
                EditableFirewallRule rule = changed[i];
                ServerGroup serverGroup = rule.getServerGroup();
                if (serverGroup == null) {
                    arg.setNull(1, Types.INTEGER);
                } else {
                    arg.setInt(1, serverGroup == null ? 0 : serverGroup.getId());
                }
                arg.setString(2, rule.getAddressType().getId());
                arg.setBoolean(3, rule.isPriority());
                arg.setString(4, rule.getSystemId().name());
            }
        };
        m_jdbc.batchUpdate(sql, inserter);
    }

    public void setJdbc(JdbcTemplate jdbc) {
        m_jdbc = jdbc;
    }
}
