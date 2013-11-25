/**
 *
 *
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
package org.sipfoundry.sipxconfig.dns;

import static java.lang.String.format;

import java.io.File;
import java.io.IOException;
import java.sql.ResultSet;
import java.sql.SQLException;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collection;
import java.util.Collections;
import java.util.HashMap;
import java.util.Iterator;
import java.util.LinkedHashSet;
import java.util.List;
import java.util.Map;
import java.util.Set;

import org.apache.commons.io.FileUtils;
import org.apache.commons.lang.StringUtils;
import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import org.sipfoundry.sipxconfig.address.Address;
import org.sipfoundry.sipxconfig.address.AddressManager;
import org.sipfoundry.sipxconfig.address.AddressProvider;
import org.sipfoundry.sipxconfig.address.AddressType;
import org.sipfoundry.sipxconfig.common.DaoUtils;
import org.sipfoundry.sipxconfig.common.DataCollectionUtil;
import org.sipfoundry.sipxconfig.common.event.DaoEventPublisher;
import org.sipfoundry.sipxconfig.commserver.Location;
import org.sipfoundry.sipxconfig.commserver.LocationsManager;
import org.sipfoundry.sipxconfig.feature.Bundle;
import org.sipfoundry.sipxconfig.feature.FeatureChangeRequest;
import org.sipfoundry.sipxconfig.feature.FeatureChangeValidator;
import org.sipfoundry.sipxconfig.feature.FeatureManager;
import org.sipfoundry.sipxconfig.feature.FeatureProvider;
import org.sipfoundry.sipxconfig.feature.GlobalFeature;
import org.sipfoundry.sipxconfig.feature.LocationFeature;
import org.sipfoundry.sipxconfig.firewall.DefaultFirewallRule;
import org.sipfoundry.sipxconfig.firewall.FirewallManager;
import org.sipfoundry.sipxconfig.firewall.FirewallProvider;
import org.sipfoundry.sipxconfig.firewall.FirewallRule;
import org.sipfoundry.sipxconfig.region.Region;
import org.sipfoundry.sipxconfig.region.RegionManager;
import org.sipfoundry.sipxconfig.setting.BeanWithSettingsDao;
import org.sipfoundry.sipxconfig.setup.SetupListener;
import org.sipfoundry.sipxconfig.setup.SetupManager;
import org.sipfoundry.sipxconfig.snmp.ProcessDefinition;
import org.sipfoundry.sipxconfig.snmp.ProcessProvider;
import org.sipfoundry.sipxconfig.snmp.SnmpManager;
import org.springframework.beans.factory.BeanFactory;
import org.springframework.beans.factory.BeanFactoryAware;
import org.springframework.beans.factory.ListableBeanFactory;
import org.springframework.jdbc.core.JdbcTemplate;
import org.springframework.jdbc.core.RowCallbackHandler;

public class DnsManagerImpl implements DnsManager, AddressProvider, FeatureProvider, BeanFactoryAware,
    ProcessProvider, FirewallProvider, SetupListener {
    private static final Log LOG = LogFactory.getLog(DnsManagerImpl.class);
    private static final String LOCATION_ID = "location_id";
    private static final String REGION_ID = "region_id";
    private static final String BASIC_ID = "basic_id";
    private static final String DNS_PLAN_ID = "dns_plan_id";
    private static final String NAME = "name";
    private static final String RECORDS = "records";
    private static final String ENABLED = "enabled";
    private BeanWithSettingsDao<DnsSettings> m_settingsDao;
    private List<DnsProvider> m_providers;
    private ListableBeanFactory m_beanFactory;
    private AddressManager m_addressManager;
    private File m_externalDnsStash;
    private JdbcTemplate m_db;
    private RegionManager m_regionManager;
    private LocationsManager m_locationsManager;
    private DaoEventPublisher m_daoEventPublisher;

    @Override
    public Address getSingleAddress(AddressType t, Collection<Address> addresses, Location whoIsAsking) {
        if (addresses == null || addresses.size() == 0) {
            return null;
        }

        for (DnsProvider p : getProviders()) {
            Address rewrite = p.getAddress(this, t, addresses, whoIsAsking);
            if (rewrite != null) {
                return rewrite;
            }
        }

        Iterator<Address> i = addresses.iterator();
        Address first = i.next();
        if (addresses.size() == 1 || whoIsAsking == null) {
            return first;
        }

        // return the address local to who is asking if available
        Address a = first;
        while (a != null) {
            if (a.getAddress().equals(whoIsAsking.getAddress())) {
                return a;
            }
            a = (i.hasNext() ? i.next() : null);
        }

        // first is as good as any other
        return first;
    }

    @Override
    public DnsFailoverPlan getPlan(Integer planId) {
        return DaoUtils.requireOneOrZero(loadPlans(planId), "dns plan by id");
    }

    @Override
    public Collection<DnsFailoverPlan> getPlans() {
        return loadPlans(null);
    }

    List<DnsFailoverPlan> loadPlans(Integer planId) {
        String sql = "select * from dns_plan as p"
                + " left join dns_group as g on p.dns_plan_id = g.dns_plan_id"
                + " right join dns_target as t on"
                + " t.dns_group_id = g.dns_group_id"
                + " %s"
                + " order by p.name, g.position";
        if (planId != null) {
            sql = format(sql, "where p.dns_plan_id = " + planId);
        } else {
            sql = format(sql, StringUtils.EMPTY);
        }
        List<DnsFailoverPlan> plans = new ArrayList<DnsFailoverPlan>();
        List<Region> regions = m_regionManager.getRegions();
        List<Location> locations = m_locationsManager.getLocationsList();
        m_db.query(sql, new PlanRowReader(plans, regions, locations));
        return plans;
    }

    @Override
    public void savePlan(DnsFailoverPlan plan) {
        if (plan.isNew()) {
            int id = m_db.queryForInt("select nextval('dns_plan_seq')");
            plan.setUniqueId(id);
        } else {
            deletePlan(plan);
        }
        m_db.update("insert into dns_plan (dns_plan_id, name) values (?,?)", plan.getId(), plan.getName());
        int position = 0;
        Object[] targetParams = new Object[3];
        for (DnsFailoverGroup g : plan.getGroups()) {
            int gid = m_db.queryForInt("select nextval('dns_group_seq')");
            g.setUniqueId(gid);
            m_db.update("insert into dns_group (dns_plan_id, dns_group_id, position) values (?,?,?)",
                    plan.getId(), gid, position++);
            for (DnsTarget t : g.getTargets()) {
                String sql = "insert into dns_target (dns_group_id, percentage, %s) values (?,?,?)";
                targetParams[0] = gid;
                targetParams[1] = t.getPercentage();
                switch (t.getTargetType()) {
                case BASIC:
                    sql = format(sql, BASIC_ID);
                    targetParams[2] = encodeBasicTarget(t.getBasicTarget());
                    break;
                case LOCATION:
                    sql = format(sql, LOCATION_ID);
                    targetParams[2] = t.getLocation().getId();
                    break;

                case REGION:
                    sql = format(sql, REGION_ID);
                    targetParams[2] = t.getRegion().getId();
                    break;
                default:
                    throw new IllegalStateException();
                }
                m_db.update(sql, targetParams);
            }
        }
    }

    static class PlanRowReader implements RowCallbackHandler {
        private List<DnsFailoverPlan> m_plans;
        private DnsFailoverPlan m_plan;
        private List<DnsFailoverGroup> m_groups;
        private Map<Integer, Region> m_regions = new HashMap<Integer, Region>();
        private List<DnsTarget> m_targets;
        private DnsFailoverGroup m_group;
        private Map<Integer, Location> m_locations = new HashMap<Integer, Location>();

        PlanRowReader(List<DnsFailoverPlan> plans, Collection<Region> regions, Collection<Location> locations) {
            m_plans = plans;
            for (Region r : regions) {
                m_regions.put(r.getId(), r);
            }
            for (Location l : locations) {
                m_locations.put(l.getId(), l);
            }
        }

        @Override
        public void processRow(ResultSet rs) throws SQLException {
            readPlan(rs);
            readGroup(rs);
            readTarget(rs);
        }

        void readGroup(ResultSet rs) throws SQLException {
            int id = rs.getInt("dns_group_id");
            if (m_group == null || !m_group.getId().equals(id)) {
                m_group = new DnsFailoverGroup();
                m_group.setUniqueId(id);
                m_targets = new ArrayList<DnsTarget>();
                m_group.setTargets(m_targets);
                m_groups.add(m_group);
            }
        }

        void readPlan(ResultSet rs) throws SQLException {
            int id = rs.getInt(DNS_PLAN_ID);
            if (m_plan == null || !m_plan.getId().equals(id)) {
                m_plan = new DnsFailoverPlan();
                m_plan.setUniqueId(id);
                m_groups = new ArrayList<DnsFailoverGroup>();
                m_plan.setGroups(m_groups);
                m_plan.setName(rs.getString(NAME));
                m_plans.add(m_plan);
            }
        }

        void readTarget(ResultSet rs) throws SQLException {
            DnsTarget target;
            Integer locationId = (Integer) rs.getObject(LOCATION_ID);
            if (locationId != null) {
                target = new DnsTarget(m_locations.get(locationId));
            } else {
                Integer regionId = (Integer) rs.getObject(REGION_ID);
                if (regionId != null) {
                    target = new DnsTarget(m_regions.get(regionId));
                } else {
                    target = new DnsTarget(decodeBasicTarget(rs.getString(BASIC_ID)));
                }
            }
            target.setPercentage(rs.getInt("percentage"));
            m_targets.add(target);
        }
    }

    static DnsTarget.BasicType decodeBasicTarget(String rs) {
        switch (rs.charAt(0)) {
        case 'O':
            return DnsTarget.BasicType.ALL_OTHER_REGIONS;
        case 'A':
            return DnsTarget.BasicType.ALL_REGIONS;
        case 'L':
            return DnsTarget.BasicType.LOCAL_REGION;
        default:
            throw new IllegalStateException();
        }
    }

    String encodeBasicTarget(DnsTarget.BasicType type) {
        switch (type) {
        case ALL_OTHER_REGIONS:
            return "O";
        case LOCAL_REGION:
            return "L";
        case ALL_REGIONS:
            return "A";
        default:
            throw new IllegalStateException();
        }
    }

    @Override
    public Collection<DnsSrvRecord> getResourceRecords(DnsView view) {
        Integer planId = view.getPlanId();
        DnsFailoverPlan plan;
        if (planId == null) {
            plan = createFairlyTypicalDnsFailoverPlan();
        } else {
            plan = getPlan(planId);
        }
        List<DnsSrvRecord> srvs = new ArrayList<DnsSrvRecord>();
        for (DnsProvider provider : m_providers) {
            Collection<ResourceRecords> rrs = provider.getResourceRecords(this);
            if (rrs != null) {
                for (ResourceRecords rr : rrs) {
                    for (ResourceRecord record : rr.getRecords()) {
                        srvs.addAll(plan.getDnsSrvRecords(view, record, rr));
                    }
                }
            }
        }

        return srvs;
    }

    public DnsFailoverPlan createFairlyTypicalDnsFailoverPlan() {
        DnsFailoverGroup targetByRegion = new DnsFailoverGroup();
        DnsTarget t2 = new DnsTarget(DnsTarget.BasicType.LOCAL_REGION);
        t2.setPercentage(DnsRecordNumerics.INCONSEQUENTIAL_PERCENTAGE);
        targetByRegion.setTargets(Collections.singleton(t2));

        DnsFailoverGroup targetByOtherRegions = new DnsFailoverGroup();
        DnsTarget t3 = new DnsTarget(DnsTarget.BasicType.ALL_OTHER_REGIONS);
        t3.setPercentage(DnsRecordNumerics.INCONSEQUENTIAL_PERCENTAGE);
        targetByOtherRegions.setTargets(Collections.singleton(t3));

        DnsFailoverPlan plan = new DnsFailoverPlan();
        plan.setGroups(Arrays.asList(targetByRegion, targetByOtherRegions));
        return plan;
    }

    @Override
    public Collection<DefaultFirewallRule> getFirewallRules(FirewallManager manager) {
        return Collections.singleton(new DefaultFirewallRule(DNS_ADDRESS, FirewallRule.SystemId.PUBLIC));
    }

    @Override
    public Collection<Address> getAvailableAddresses(AddressManager manager, AddressType type, Location requester) {
        if (!type.equals(DNS_ADDRESS)) {
            return null;
        }
        Set<Address> addresses = new LinkedHashSet<Address>();
        DnsSettings settings = getSettings();
        if (settings.isServiceUnmanaged()) {
            List<String> dnsServers = settings.getUnmanagedDnsServers();
            for (String server : dnsServers) {
                Address address = new Address(DNS_ADDRESS, server);
                addresses.add(address);
            }
        }
        List<Location> locations = manager.getFeatureManager().getLocationsForEnabledFeature(FEATURE);
        addresses.addAll(Location.toAddresses(DNS_ADDRESS, locations));
        return addresses;
    }

    @Override
    public Collection<GlobalFeature> getAvailableGlobalFeatures(FeatureManager featureManager) {
        return null;
    }

    @Override
    public Collection<LocationFeature> getAvailableLocationFeatures(FeatureManager featureManager, Location l) {
        return Collections.singleton(FEATURE);
    }

    @Override
    public DnsSettings getSettings() {
        return m_settingsDao.findOrCreateOne();
    }

    @Override
    public void saveSettings(DnsSettings settings) {
        m_settingsDao.upsert(settings);
    }

    public void setSettingsDao(BeanWithSettingsDao<DnsSettings> settingsDao) {
        m_settingsDao = settingsDao;
    }

    List<DnsProvider> getProviders() {
        if (m_providers == null) {
            Map<String, DnsProvider> beanMap = m_beanFactory.getBeansOfType(DnsProvider.class, false, false);
            m_providers = new ArrayList<DnsProvider>(beanMap.values());
        }
        return m_providers;
    }

    @Override
    public void setBeanFactory(BeanFactory beanFactory) {
        m_beanFactory = (ListableBeanFactory) beanFactory;
    }

    public AddressManager getAddressManager() {
        return m_addressManager;
    }

    public void setAddressManager(AddressManager addressManager) {
        m_addressManager = addressManager;
    }

    void setProviders(List<DnsProvider> providers) {
        m_providers = providers;
    }

    @Override
    public Collection<ProcessDefinition> getProcessDefinitions(SnmpManager manager, Location location) {
        boolean enabled = manager.getFeatureManager().isFeatureEnabled(FEATURE, location);
        return (enabled ? Collections.singleton(ProcessDefinition.sysv("named", true)) : null);
    }

    @Override
    public void getBundleFeatures(FeatureManager featureManager, Bundle b) {
        if (b == Bundle.CORE) {
            b.addFeature(FEATURE);
        }
    }

    @Override
    public void featureChangePrecommit(FeatureManager manager, FeatureChangeValidator validator) {
        // consider requirement, if there's more than one location
        // at least one DNS is required or if SIP domain is not FQDN -- Douglas
    }

    @Override
    public void featureChangePostcommit(FeatureManager manager, FeatureChangeRequest request) {
    }

    void setInitialExternalDnsServer(File stashFile) {
        String externalDns = null;
        if (stashFile.exists()) {
            try {
                externalDns = FileUtils.readFileToString(stashFile);
            } catch (IOException e) {
                LOG.warn("Could not read from DNS forwarder stash file.", e);
            }
        } else {
            // only works on Sun/Open JDK >= 1.5
            List< ? > nameservers = sun.net.dns.ResolverConfiguration.open().nameservers();
            if (!nameservers.isEmpty()) {
                externalDns = nameservers.get(0).toString();
            }
        }
        if (StringUtils.isNotBlank(externalDns)) {
            try {
                FileUtils.writeStringToFile(stashFile, externalDns);
            } catch (IOException e) {
                LOG.warn("Could not write to DNS forwarder stash file.", e);
            }
            DnsSettings settings = getSettings();
            settings.setDnsForwarder(externalDns, 0);
            saveSettings(settings);
        }
    }

    @Override
    public boolean setup(SetupManager manager) {
        if (manager.isFalse(FEATURE.getId())) {
            Location primary = manager.getConfigManager().getLocationManager().getPrimaryLocation();
            if (primary == null) {
                return false;
            }

            setInitialExternalDnsServer(m_externalDnsStash);
            manager.getFeatureManager().enableLocationFeature(FEATURE, primary, true);
            manager.setTrue(FEATURE.getId());
        }

        return true;
    }

    public void setExternalDnsStash(String externalDnsStash) {
        m_externalDnsStash = new File(externalDnsStash);
    }

    public void setConfigJdbcTemplate(JdbcTemplate configJdbc) {
        m_db = configJdbc;
    }

    public void setRegionManager(RegionManager regionManager) {
        m_regionManager = regionManager;
    }

    public void setLocationsManager(LocationsManager locationsManager) {
        m_locationsManager = locationsManager;
    }

    @Override
    public void deletePlan(DnsFailoverPlan plan) {
        // assumes scheme has delete on cascade enabled for these tables
        m_db.update("delete from dns_plan where dns_plan_id = ?", plan.getId());
    }

    @Override
    public Collection<DnsView> getViews() {
        return loadViews(StringUtils.EMPTY);
    }

    @Override
    public DnsView getViewById(Integer viewId) {
        String filter = "where dns_view_id = " + viewId;
        Collection<DnsView> views = loadViews(filter);
        return DaoUtils.requireOneOrZero(views, filter);
    }

    List<DnsView> loadViews(String filter) {
        String sql = format("select * from dns_view as v %s"
                + " order by v.position", filter);
        final List<DnsView> views = new ArrayList<DnsView>();
        m_db.query(sql, new RowCallbackHandler() {
            @Override
            public void processRow(ResultSet rs) throws SQLException {
                DnsView view = new DnsView();
                view.setEnabled(rs.getBoolean(ENABLED));
                view.setUniqueId(rs.getInt("dns_view_id"));
                view.setName(rs.getString(NAME));
                view.setPlanId(rs.getInt(DNS_PLAN_ID));
                view.setRegionId(rs.getInt(REGION_ID));
                views.add(view);
            }
        });

        loadCustomIdsForViews(views);
        return views;
    }

    void loadCustomIdsForViews(Collection<DnsView> views) {
        for (DnsView view : views) {
            String sql = "select dns_custom_id from dns_custom_view_link where dns_view_id = ?";
            view.setCustomRecordsIds(m_db.queryForList(sql, Integer.class, view.getId()));
        }
    }

    @Override
    public void saveView(DnsView view) {
        String sql;
        String[] fields = new String[] {
            REGION_ID, ENABLED, NAME, DNS_PLAN_ID
        };
        if (view.isNew()) {
            int id = m_db.queryForInt("select nextval('dns_view_seq')");
            view.setUniqueId(id);
            sql = format("insert into dns_view (%s, position, dns_view_id) values (%s %d, ?)",
                    StringUtils.join(fields, ", "), StringUtils.repeat("?,", fields.length), id);
        } else {
            sql = format("update dns_view set %s = ? where dns_view_id = ?", StringUtils.join(fields, " = ?, "));
        }

        m_db.update(sql, view.getRegionId(), view.isEnabled(), view.getName(), view.getPlanId(), view.getId());

        m_db.update("delete  from dns_custom_view_link where dns_view_id = ?", view.getId());
        if (view.getCustomRecordsIds() != null && view.getCustomRecordsIds().size() > 0) {
            StringBuilder customIdsSql = new StringBuilder("insert into dns_custom_view_link "
                    + "(dns_custom_id, dns_view_id) values ");
            boolean first = true;
            for (Integer id : view.getCustomRecordsIds()) {
                if (!first) {
                    customIdsSql.append(",");
                }

                customIdsSql.append(format("(%d,%d)", id, view.getId()));
                first = false;
            }
            m_db.update(customIdsSql.toString());
        }
    }

    @Override
    public void deleteView(DnsView view) {
        m_db.update("delete from dns_custom_view_link where dns_view_id = ?", view.getId());
        m_db.update("delete from dns_view where dns_view_id = " + view.getId());
    }

    @Override
    public void moveViewById(Integer[] viewIds, int step) {
        List<DnsView> views = (List<DnsView>) getViews();
        DataCollectionUtil.moveByPrimaryKey(views, viewIds, step);
        int position = 0;
        String sql = "update dns_view set position = ? where dns_view_id = ?";
        for (DnsView view : views) {
            m_db.update(sql, position++, view.getId());
        }
        m_daoEventPublisher.publishSaveCollection(views);
    }

    public void setDaoEventPublisher(DaoEventPublisher eventPublisher) {
        m_daoEventPublisher = eventPublisher;
    }

    String[] inUse(String sql) {
        List<String> found = m_db.queryForList(sql, String.class);
        if (found == null || found.size() == 0) {
            return null;
        }
        return found.toArray(new String[0]);
    }

    @Override
    public String[] getViewNamesUsingRegion(Region region) {
        return inUse("select name from dns_view where region_id = " + region.getId());
    }

    @Override
    public String[] getViewNamesUsingPlan(DnsFailoverPlan plan) {
        return inUse("select name from dns_view where dns_plan_id = " + plan.getId());
    }

    String planInUseSql(String targetColumn, Integer targetId) {
        return format("select p.name from dns_plan as p, dns_group g, dns_target as t where "
                + "t.dns_group_id = g.dns_group_id and g.dns_plan_id = p.dns_plan_id and " + "t.%s = %s",
                targetColumn, targetId);
    }

    @Override
    public String[] getPlanNamesUsingRegion(Region region) {
        return inUse(planInUseSql(REGION_ID, region.getId()));
    }

    @Override
    public String[] getPlanNamesUsingLocation(Location location) {
        return inUse(planInUseSql(LOCATION_ID, location.getId()));
    }

    @Override
    public Collection<DnsCustomRecords> getCustomRecords() {
        String sql = "select * from dns_custom order by name";
        return loadCustomRecords(sql);
    }

    @Override
    public DnsCustomRecords getCustomRecordsById(Integer customId) {
        String sql = "select * from dns_custom where dns_custom_id = " + customId;
        Collection<DnsCustomRecords> customs = loadCustomRecords(sql);
        return DaoUtils.requireOneOrZero(customs, sql);
    }

    @Override
    public Collection<DnsCustomRecords> getCustomRecordsByIds(Collection<Integer> customIds) {
        String customIdStr = StringUtils.join(customIds, ',');
        String sql = format("select * from dns_custom where dns_custom_id in (%s) order by name", customIdStr);
        return loadCustomRecords(sql);
    }

    List<DnsCustomRecords> loadCustomRecords(String sql) {
        final List<DnsCustomRecords> records = new ArrayList<DnsCustomRecords>();
        m_db.query(sql, new RowCallbackHandler() {
            @Override
            public void processRow(ResultSet rs) throws SQLException {
                DnsCustomRecords record = new DnsCustomRecords();
                record.setUniqueId(rs.getInt("dns_custom_id"));
                record.setName(rs.getString(NAME));
                record.setRecords(rs.getString(RECORDS));
                records.add(record);
            }
        });
        return records;
    }

    @Override
    public void saveCustomRecords(DnsCustomRecords custom) {
        String sql;
        if (custom.isNew()) {
            int id = m_db.queryForInt("select nextval('dns_custom_seq')");
            custom.setUniqueId(id);
            sql = "insert into dns_custom (name, records, dns_custom_id) values (?, ?, ?)";
        } else {
            sql = "update dns_custom set name = ?, records = ? where dns_custom_id = ?";
        }

        m_db.update(sql, custom.getName(), custom.getRecords(), custom.getId());
    }

    @Override
    public void deleteCustomRecords(DnsCustomRecords custom) {
        m_db.update("delete from dns_custom where dns_custom_id = " + custom.getId());
    }

//    @Override
//    public Collection<DnsCustomRecords> getCustomRecordsForView(Integer viewId) {
//        String sql = "select c.* from dns_custom as c, dns_custom_view_link lnk, dns_view as v "
//                + "where c.dns_custom_id = lnk.dns_custom_id and "
//                + "lnk.dns_view_id = v.dns_view_id order by c.name";
//        return loadCustomRecords(sql);
//    }
//
//    @Override
//    public void setCustomRecordsForView(Integer viewId, Collection<DnsCustomRecords> custom) {
//        m_db.update("delete from dns_custom_view_link where dns_view_id = " + viewId);
//        StringBuilder update = new StringBuilder("insert into dns_custom_view_link("
//                + "dns_view_id, dns_custom_id) values ");
//        boolean first = true;
//        for (DnsCustomRecords c : custom) {
//            if (!first) {
//                update.append(',');
//            }
//            update.append(format("(%d, %d)", viewId, c.getId()));
//            first = false;
//        }
//        m_db.update(update.toString());
//    }
}
