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
import java.util.Comparator;
import java.util.List;
import java.util.Map;

import org.apache.commons.collections.CollectionUtils;
import org.apache.commons.collections.Predicate;
import org.apache.commons.lang.StringUtils;
import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
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

public class FirewallManagerImpl extends SipxHibernateDaoSupport<FirewallRule> implements FirewallManager,
        FeatureProvider, SetupListener, BeanFactoryAware {
    private static final Log LOG = LogFactory.getLog(FirewallManagerImpl.class);
    private static final String SERVER_GROUP_COL = "firewall_server_group_id";
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
    public void saveSettings(FirewallSettings settings) {
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
                group.setUniqueId(rs.getInt(SERVER_GROUP_COL));
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
        final Map<String, DefaultFirewallRule> defaults = Util.defaultsByAddressTypeId(getDefaultFirewallRules());
        final Map<Integer, ServerGroup> groups = Util.groupsById(getServerGroups());
        RowMapper<EditableFirewallRule> reader = new RowMapper<EditableFirewallRule>() {
            public EditableFirewallRule mapRow(ResultSet rs, int arg1) throws SQLException {
                String addressType = rs.getString("address_type");
                DefaultFirewallRule pop = defaults.remove(addressType);
                if (pop == null) {
                    LOG.error("Cannot find default firewall rule for address type " + addressType);
                    return null;
                }
                EditableFirewallRule rule = new EditableFirewallRule(pop);
                rule.setPriority(rs.getBoolean("prioritize"));
                String systemId = rs.getString("system_id");
                if (StringUtils.isNotBlank(systemId)) {
                    rule.setSystemId(FirewallRule.SystemId.valueOf(systemId));
                }
                int groupId = rs.getInt(SERVER_GROUP_COL);
                if (groupId > 0) {
                    ServerGroup group = groups.get(groupId);
                    if (group == null) {
                        LOG.error("Cannot find server group with id " + groupId);
                    } else {
                        rule.setServerGroup(group);
                    }
                }
                rule.setUniqueId(rs.getInt("firewall_rule_id"));
                return rule;
            }
        };
        String sql = "select * from firewall_rule";
        List<EditableFirewallRule> editable = m_jdbc.query(sql, reader);
        editable.removeAll(Collections.singletonList(null));

        // add remaining defaults
        for (DefaultFirewallRule def : defaults.values()) {
            editable.add(new EditableFirewallRule(def));
        }

        // sort by address type to give consistent order and an order that could be influenced
        // by implementations.
        Collections.sort(editable, new Comparator<EditableFirewallRule>() {
            public int compare(EditableFirewallRule o1, EditableFirewallRule o2) {
                return o1.getAddressType().getId().compareTo(o2.getAddressType().getId());
            }
        });
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

    @SuppressWarnings({
        "unchecked", "rawtypes"
    })
    EditableFirewallRule[] getChanged(List<EditableFirewallRule> rules) {
        Predicate selectChanged = new Predicate() {
            public boolean evaluate(Object arg0) {
                return ((EditableFirewallRule) arg0).isChangedFromDefault();
            }
        };

        Collection changedCol = CollectionUtils.select(rules, selectChanged);
        return (EditableFirewallRule[]) changedCol.toArray(new EditableFirewallRule[0]);
    }

    @Override
    public void saveRules(List<EditableFirewallRule> rules) {
        m_jdbc.execute("delete from firewall_rule");
        String sql = "insert into firewall_rule (firewall_rule_id, firewall_server_group_id, "
                + "address_type, prioritize, system_id) values (nextval('firewall_rule_seq'),?,?,?,?)";
        final EditableFirewallRule[] changed = getChanged(rules);
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
                arg.setString(4, rule.getSystemId().toString());
            }
        };
        m_jdbc.batchUpdate(sql, inserter);
    }

    public void setJdbc(JdbcTemplate jdbc) {
        m_jdbc = jdbc;
    }
}
