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
import java.util.HashSet;
import java.util.List;
import java.util.Map;
import java.util.Set;

import org.apache.commons.collections.CollectionUtils;
import org.apache.commons.collections.Predicate;
import org.apache.commons.lang.StringUtils;
import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import org.sipfoundry.sipxconfig.common.DaoUtils;
import org.sipfoundry.sipxconfig.common.SipxHibernateDaoSupport;
import org.sipfoundry.sipxconfig.common.UserException;
import org.sipfoundry.sipxconfig.common.event.DaoEventListener;
import org.sipfoundry.sipxconfig.commserver.Location;
import org.sipfoundry.sipxconfig.feature.Bundle;
import org.sipfoundry.sipxconfig.feature.FeatureChangeRequest;
import org.sipfoundry.sipxconfig.feature.FeatureChangeValidator;
import org.sipfoundry.sipxconfig.feature.FeatureManager;
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
        FeatureProvider, SetupListener, BeanFactoryAware, DaoEventListener {
    private static final Log LOG = LogFactory.getLog(FirewallManagerImpl.class);
    private static final String SERVER_GROUP_COL = "firewall_server_group_id";
    private BeanWithSettingsDao<FirewallSettings> m_settingsDao;
    private List<FirewallProvider> m_providers;
    private ListableBeanFactory m_beanFactory;
    private JdbcTemplate m_jdbc;

    @Override
    public FirewallSettings getSettings() {
        return m_settingsDao.findOrCreateOne();
    }

    @Override
    public void saveSettings(FirewallSettings settings) {
        settings.validate();
        m_settingsDao.upsert(settings);
    }

    public void setSettingsDao(BeanWithSettingsDao<FirewallSettings> settingsDao) {
        m_settingsDao = settingsDao;
    }

    @Override
    public Collection<GlobalFeature> getAvailableGlobalFeatures(FeatureManager featureManager) {
        return Collections.singleton(FEATURE);
    }

    @Override
    public Collection<LocationFeature> getAvailableLocationFeatures(FeatureManager featureManager, Location l) {
        return null;
    }

    @Override
    public void getBundleFeatures(FeatureManager featureManager, Bundle b) {
        if (b == Bundle.CORE) {
            b.addFeature(FEATURE);
        }
    }

    @Override
    public boolean setup(SetupManager manager) {
        if (manager.isFalse(FEATURE.getId())) {
            manager.getFeatureManager().enableGlobalFeature(FEATURE, true);
            manager.setTrue(FEATURE.getId());
        }
        return true;
    }

    public List<ServerGroup> getServerGroups() {
        String sql = "select * from firewall_server_group";
        return getServerGroupsBySql(sql);
    }

    List<ServerGroup> getServerGroupsBySql(String sql, Object... args) {
        RowMapper<ServerGroup> reader = new RowMapper<ServerGroup>() {
            public ServerGroup mapRow(ResultSet rs, int arg1) throws SQLException {
                ServerGroup group = new ServerGroup();
                group.setName(rs.getString("name"));
                group.setServerList(rs.getString("servers"));
                group.setUniqueId(rs.getInt(SERVER_GROUP_COL));
                return group;
            }
        };
        List<ServerGroup> groups = m_jdbc.query(sql, args, reader);
        return groups;
    }

    public List<DefaultFirewallRule> getDefaultFirewallRules() {
        List<DefaultFirewallRule> rules = new ArrayList<DefaultFirewallRule>();
        for (FirewallProvider provider : getProviders()) {
            Collection<DefaultFirewallRule> addRules = provider.getFirewallRules(this);
            if (addRules != null && !addRules.isEmpty()) {
                rules.addAll(addRules);
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
                + "system_id, address_type, prioritize) values (nextval('firewall_rule_seq'),?,?,?,?)";
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
                    arg.setString(2, rule.getSystemId().toString());
                } else {
                    arg.setInt(1, serverGroup == null ? 0 : serverGroup.getId());
                    arg.setNull(2, Types.VARCHAR);
                }
                arg.setString(3, rule.getAddressType().getId());
                arg.setBoolean(4, rule.isPriority());
            }
        };
        m_jdbc.batchUpdate(sql, inserter);
    }

    public void setJdbc(JdbcTemplate jdbc) {
        m_jdbc = jdbc;
    }

    @Override
    public ServerGroup getServerGroup(Integer groupId) {
        String sql = "select * from firewall_server_group where firewall_server_group_id = ?";
        return DaoUtils.requireOneOrZero(getServerGroupsBySql(sql, groupId), sql);
    }

    @Override
    public void saveServerGroup(final ServerGroup serverGroup) {
        String sql;
        if (serverGroup.isNew()) {
            if (isGroupDefined(serverGroup.getName())) {
                throw new UserException("&err.serverGroupExists");
            } else {
                sql = "insert into firewall_server_group (firewall_server_group_id, name, servers) values "
                        + "(nextval('firewall_server_group_seq'),?,?) returning firewall_server_group_id";
            }
        } else {
            sql = "update firewall_server_group set name = ?, servers = ? where firewall_server_group_id = ? "
                    + "returning firewall_server_group_id";
        }
        ArrayList<Object> params = new ArrayList<Object>();
        params.add(serverGroup.getName());
        params.add(serverGroup.getServerList());
        if (!serverGroup.isNew()) {
            params.add(serverGroup.getId());
        }
        Integer newId = m_jdbc.queryForInt(sql, params.toArray());
        serverGroup.setUniqueId(newId);
    }

    private boolean isGroupDefined(String groupName) {
        int check = m_jdbc.queryForInt("select count(*) from firewall_server_group where name = ?", groupName);
        return (check >= 1);
    }

    @Override
    public void deleteServerGroup(ServerGroup serverGroup) {
        m_jdbc.update("delete from firewall_server_group where firewall_server_group_id = ?", serverGroup.getId());
    }

    @Override
    public void onDelete(Object entity) {
        if (entity instanceof ServerGroup) {
            ServerGroup group = (ServerGroup) entity;
            m_jdbc.update("delete from firewall_rule where firewall_server_group_id = ?", group.getId());
        }
    }

    @Override
    public void onSave(Object entity) {
    }

    @Override
    public List<CustomFirewallRule> getCustomRules(Location location, Map<Object, Object> requestData) {
        List<CustomFirewallRule> custom = new ArrayList<CustomFirewallRule>();
        for (FirewallProvider provider : getProviders()) {
            if (provider instanceof FirewallCustomRuleProvider) {
                FirewallCustomRuleProvider customProvider = (FirewallCustomRuleProvider) provider;
                Collection<CustomFirewallRule> customRules = customProvider.getCustomRules(this, location, requestData);
                if (customRules != null && !customRules.isEmpty()) {
                    custom.addAll(customRules);
                }
            }
        }

        return custom;
    }

    @Override
    public void featureChangePrecommit(FeatureManager manager, FeatureChangeValidator validator) {
    }

    @Override
    public void featureChangePostcommit(FeatureManager manager, FeatureChangeRequest request) {
    }

    @Override
    public Set<String> getRequiredModules(Location location, Map<Object, Object> requestData) {
        Set<String> mods = new HashSet<String>();
        for (FirewallProvider provider : getProviders()) {
            if (provider instanceof FirewallCustomRuleProvider) {
                FirewallCustomRuleProvider customProvider = (FirewallCustomRuleProvider) provider;
                Collection<String> customMods = customProvider.getRequiredModules(this, location, requestData);
                if (customMods != null && !customMods.isEmpty()) {
                    mods.addAll(customMods);
                }
            }
        }

        return mods;
    }
}
