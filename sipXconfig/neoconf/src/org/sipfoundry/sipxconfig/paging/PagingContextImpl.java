/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.paging;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collection;
import java.util.Collections;
import java.util.List;
import java.util.regex.Matcher;
import java.util.regex.Pattern;

import org.apache.commons.lang.StringUtils;
import org.sipfoundry.sipxconfig.address.Address;
import org.sipfoundry.sipxconfig.address.AddressManager;
import org.sipfoundry.sipxconfig.address.AddressProvider;
import org.sipfoundry.sipxconfig.address.AddressType;
import org.sipfoundry.sipxconfig.alias.AliasManager;
import org.sipfoundry.sipxconfig.common.BeanId;
import org.sipfoundry.sipxconfig.common.SipxHibernateDaoSupport;
import org.sipfoundry.sipxconfig.common.UserException;
import org.sipfoundry.sipxconfig.commserver.Location;
import org.sipfoundry.sipxconfig.dialplan.DialingRule;
import org.sipfoundry.sipxconfig.dialplan.PagingRule;
import org.sipfoundry.sipxconfig.feature.FeatureManager;
import org.sipfoundry.sipxconfig.firewall.DefaultFirewallRule;
import org.sipfoundry.sipxconfig.firewall.FirewallManager;
import org.sipfoundry.sipxconfig.firewall.FirewallProvider;
import org.sipfoundry.sipxconfig.firewall.FirewallRule;
import org.sipfoundry.sipxconfig.setting.BeanWithSettingsDao;
import org.sipfoundry.sipxconfig.snmp.ProcessDefinition;
import org.sipfoundry.sipxconfig.snmp.ProcessProvider;
import org.sipfoundry.sipxconfig.snmp.SnmpManager;
import org.springframework.dao.support.DataAccessUtils;
import org.springframework.jdbc.core.JdbcTemplate;

public class PagingContextImpl extends SipxHibernateDaoSupport implements PagingContext,
        AddressProvider, ProcessProvider, FirewallProvider {

    /** Default ALERT-INFO - hardcoded in Polycom phone configuration */
    private static final String ALERT_INFO = "sipXpage";
    private static final String PARAM_PAGING_GROUP_NUMBER = "pageGroupNumber";
    private static final String PARAM_PAGING_GROUP_ID = "pagingGroupId";
    private static final String ERROR_ALIAS_IN_USE = "&error.aliasinuse";
    private static final Collection<AddressType> ADDRESSES = Arrays.asList(SIP_TCP, SIP_TLS, SIP_UDP, RTP_PORT);
    private static final String VALID_SIP_REGEX = "([-_.!~*'\\(\\)=+$,;?/a-zA-Z0-9]|(%[0-9a-fA-F]{2}))+";
    private AliasManager m_aliasManager;
    private BeanWithSettingsDao<PagingSettings> m_settingsDao;

    private FeatureManager m_featureManager;
    private JdbcTemplate m_jdbc;

    @Override
    public PagingSettings getSettings() {
        return m_settingsDao.findOrCreateOne();
    }

    @Override
    public void saveSettings(PagingSettings settings) {
        if (StringUtils.isBlank(settings.getSettingValue(PagingSettings.PREFIX))) {
            throw new UserException("&error.blank.prefix");
        }
        Pattern p = Pattern.compile(VALID_SIP_REGEX);
        Matcher m = p.matcher(settings.getSettingValue(PagingSettings.PREFIX));
        if (!m.matches()) {
            throw new UserException("&error.invalid.prefix");
        }
        m_settingsDao.upsert(settings);
    }

    @Override
    public List<PagingGroup> getPagingGroups() {
        return getHibernateTemplate().loadAll(PagingGroup.class);
    }

    @Override
    public PagingGroup getPagingGroupById(Integer pagingGroupId) {
        return getHibernateTemplate().load(PagingGroup.class, pagingGroupId);
    }

    void checkAliasUse(PagingGroup group, String prefix) {
        String code = prefix + group.getPageGroupNumber();
        if (!m_aliasManager.canObjectUseAlias(group, code)) {
            throw new UserException(ERROR_ALIAS_IN_USE, code);
        }
    }

    @Override
    public void savePagingGroup(PagingGroup group) {
        checkAliasUse(group, getSettings().getPrefix());
        if (group.isNew()) {
            // check if new object
            checkForDuplicateNames(group);
            getHibernateTemplate().save(group);
        } else {
            // on edit action - check if the group number for this group was modified
            // if the group number was changed then perform duplicate group number checking
            if (isNameChanged(group)) {
                checkForDuplicateNames(group);
            }
            getHibernateTemplate().merge(group);
        }
    }

    private void checkForDuplicateNames(PagingGroup group) {
        if (isNameInUse(group)) {
            throw new UserException("&error.duplicateGroupNumbers");
        }
    }

    private boolean isNameInUse(PagingGroup group) {
        List count = getHibernateTemplate().findByNamedQueryAndNamedParam("anotherPagingGroupWithSameName",
                new String[] {
                    PARAM_PAGING_GROUP_NUMBER
                }, new Object[] {
                    group.getPageGroupNumber()
                });

        return DataAccessUtils.intResult(count) > 0;
    }

    private boolean isNameChanged(PagingGroup group) {
        List count = getHibernateTemplate().findByNamedQueryAndNamedParam("countPagingGroupWithSameName",
                new String[] {
                    PARAM_PAGING_GROUP_ID, PARAM_PAGING_GROUP_NUMBER
                }, new Object[] {
                    group.getId(), group.getPageGroupNumber()
                });

        return DataAccessUtils.intResult(count) == 0;
    }

    @Override
    public void clear() {
        removeAll(PagingGroup.class);
    }

    @Override
    public List< ? extends DialingRule> getDialingRules(Location location) {
        if (!m_featureManager.isFeatureEnabled(FEATURE)) {
            return null;
        }

        String prefix = getSettings().getPrefix();
        if (StringUtils.isEmpty(prefix)) {
            return Collections.emptyList();
        }
        PagingRule rule = new PagingRule(prefix, ALERT_INFO);
        return Arrays.asList(rule);
    }

/*    public UserDeleteListener createUserDeleteListener() {
        return new OnUserDelete();
    }

    private class OnUserDelete extends UserDeleteListener {
        protected void onUserDelete(User user) {
            boolean affectPaging = false;
            List<PagingGroup> groups = getPagingGroups();
            for (PagingGroup group : groups) {
                Set<User> users = group.getUsers();
                if (users.remove(user)) {
                    getHibernateTemplate().saveOrUpdate(group);
                    getHibernateTemplate().flush();
                    affectPaging = true;
                }
            }
            if (affectPaging) {
                m_configManager.configureEverywhere(FEATURE);
            }
        }
    }*/

    @Override
    public boolean isAliasInUse(String alias) {
        if (!m_featureManager.isFeatureEnabled(FEATURE)) {
            return false;
        }

        String prefix = getSettings().getPrefix();
        List<Integer> pagingGroups = m_jdbc.queryForList("SELECT page_group_number FROM paging_group", Integer.class);

        for (int pg : pagingGroups) {
            String code = new StringBuilder(prefix).append(pg).toString();
            if (StringUtils.equals(alias, code)) {
                return true;
            }
        }
        return false;
    }

    @Override
    public Collection getBeanIdsOfObjectsWithAlias(String alias) {
        if (!m_featureManager.isFeatureEnabled(FEATURE)) {
            return null;
        }

        String prefix = getSettings().getPrefix();
        Collection<Integer> ids = new ArrayList<Integer>();
        for (PagingGroup pg : getPagingGroups()) {
            String code = new StringBuilder(prefix).append(pg.getPageGroupNumber()).toString();
            if (StringUtils.equals(alias, code)) {
                ids.add(pg.getId());
            }
        }
        return BeanId.createBeanIdCollection(ids, PagingGroup.class);
    }

    public void setAliasManager(AliasManager aliasManager) {
        m_aliasManager = aliasManager;
    }

    public void setSettingsDao(BeanWithSettingsDao<PagingSettings> settingsDao) {
        m_settingsDao = settingsDao;
    }

    public void setFeatureManager(FeatureManager featureManager) {
        m_featureManager = featureManager;
    }

    public void setJdbc(JdbcTemplate jdbc) {
        m_jdbc = jdbc;
    }

    @Override
    public Collection<DefaultFirewallRule> getFirewallRules(FirewallManager manager) {
        List<AddressType> cluster = Arrays.asList(SIP_TCP, SIP_TLS, SIP_UDP);
        List<DefaultFirewallRule> rules = DefaultFirewallRule.rules(cluster);
        rules.add(new DefaultFirewallRule(RTP_PORT, FirewallRule.SystemId.PUBLIC, true));
        return rules;
    }

    @Override
    public Collection<Address> getAvailableAddresses(AddressManager manager, AddressType type, Location requester) {
        if (!ADDRESSES.contains(type)) {
            return null;
        }

        PagingSettings settings = getSettings();
        List<Location> locations = manager.getFeatureManager().getLocationsForEnabledFeature(FEATURE);
        List<Address> addresses = new ArrayList<Address>(locations.size());
        for (Location l : locations) {
            Address address = null;
            if (type.equals(SIP_TCP)) {
                address = new Address(SIP_TCP, l.getAddress(), settings.getSipTcpPort());
            } else if (type.equals(SIP_UDP)) {
                address = new Address(SIP_UDP, l.getAddress(), settings.getSipUdpPort());
            } else if (type.equals(SIP_TLS)) {
                address = new Address(SIP_TLS, l.getAddress(), settings.getSipTlsPort());
            } else if (type.equals(RTP_PORT)) {
                int startPort = settings.getRtpPort();
                int maxSessions = settings.getMaxSessions();
                address = new Address(RTP_PORT, l.getAddress(), startPort);
                // no code was found in sipXpage that seemed to limit rtp range
                // but assume ports that are 2x max calls seems reasonable
                address.setEndPort(startPort + (2 * maxSessions));
            }
            addresses.add(address);
        }

        return addresses;
    }

    @Override
    public Collection<ProcessDefinition> getProcessDefinitions(SnmpManager manager, Location location) {
        boolean enabled = manager.getFeatureManager().isFeatureEnabled(FEATURE, location);
        return (enabled ? Collections.singleton(ProcessDefinition.sipxByRegex("sipxpage",
                ".*\\s-Dprocname=sipxpage\\s.*")) : null);
    }

}
