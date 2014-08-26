/**
 *
 * Copyright (c) 2013 Karel Electronics Corp. All rights reserved.
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
 *
 */

package org.sipfoundry.sipxconfig.systemaudit;

import java.io.PrintWriter;
import java.sql.SQLException;
import java.util.ArrayList;
import java.util.Collection;
import java.util.HashSet;
import java.util.List;
import java.util.Set;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import org.hibernate.HibernateException;
import org.hibernate.Session;
import org.hibernate.criterion.DetachedCriteria;
import org.hibernate.criterion.Order;
import org.hibernate.criterion.Projections;
import org.hibernate.criterion.Restrictions;
import org.sipfoundry.sipxconfig.common.CoreContext;
import org.sipfoundry.sipxconfig.common.InExpressionIgnoringCase;
import org.sipfoundry.sipxconfig.common.SipxHibernateDaoSupport;
import org.sipfoundry.sipxconfig.setting.Group;
import org.sipfoundry.sipxconfig.setting.SettingDao;
import org.springframework.beans.factory.annotation.Required;
import org.springframework.orm.hibernate3.HibernateCallback;

public class ConfigChangeContextImpl extends SipxHibernateDaoSupport<ConfigChange> implements
        ConfigChangeContext {

    private static final Log LOG = LogFactory.getLog(ConfigChangeContextImpl.class);
    private static final String COMMA_SEPARATOR = ",";
    private static final String DATE_TIME = "dateTime";
    private static final String DETAILS = "details";

    private SettingDao m_settingDao;
    private CoreContext m_coreContext;

    @Override
    public List<ConfigChange> getConfigChanges() {
        return getHibernateTemplate().loadAll(ConfigChange.class);
    }

    @Override
    public List<ConfigChange> loadConfigChangesByPage(Integer groupId,
            int firstRow, int pageSize, String[] orderBy,
            boolean orderAscending, SystemAuditFilter filter) {
        DetachedCriteria c = createCriteria(ConfigChange.class, groupId, orderBy, orderAscending,
                filter);
        return getHibernateTemplate().findByCriteria(c, firstRow, pageSize);
    }

    @Override
    public List<ConfigChangeValue> loadConfigChangeValuesByPage(Integer configChangeId, Integer groupId,
            int firstRow, int pageSize, String[] orderBy,
            boolean orderAscending) {
        DetachedCriteria c = createCriteria(ConfigChangeValue.class, groupId, orderBy, orderAscending, null);
        c.add(Restrictions.eq("configChange.id", configChangeId));
        return getHibernateTemplate().findByCriteria(c, firstRow, pageSize);
    }

    /**
     * Add SystemAuditFilter to hibernate criteria
     *
     * @param crit
     * @param filter
     */
    private void addFilterCriteria(DetachedCriteria crit, SystemAuditFilter filter) {
        crit.add(Restrictions.between(DATE_TIME, filter.getStartDate(), filter.getEndDate()));
        String type = filter.getType();
        if (type != null && !type.equals(ConfigChangeType.ALL.getName())) {
            crit.add(Restrictions.eq("configChangeType", type));
        }
        ConfigChangeAction action = filter.getAction();
        if (action != null && action != ConfigChangeAction.ALL) {
            crit.add(Restrictions.eq("configChangeAction", action));
        }
        String userNameKey = "userName";
        String userName = filter.getUserName();
        if (userName != null) {
            crit.add(Restrictions.eq(userNameKey, userName));
        }
        Set<Group> userGroups = filter.getUserGroup();
        Set<String> userNames = new HashSet<String>();
        if (!userGroups.isEmpty()) {
            for (Group userGroup : userGroups) {
                Collection<String> userNamesInGroup = m_coreContext.getGroupMembersNames(userGroup);
                userNames.addAll(userNamesInGroup);
            }
            if (userNames.isEmpty()) {
                userNames.add("");
            }
            crit.add(Restrictions.in(userNameKey, userNames));
        }
        String details = filter.getDetails();
        String localizedDetails = filter.getLocalizedDetails();
        List<String> detailsList = new ArrayList<String>();
        if (details != null) {
            detailsList.add(details);
        }
        if (localizedDetails != null) {
            detailsList.add(localizedDetails);
        }
        if (!detailsList.isEmpty()) {
            crit.add(new InExpressionIgnoringCase(DETAILS, detailsList.toArray()));
        }
    }

    @Override
    public List<Group> getGroups() {
        return m_settingDao.getGroups(GROUP_RESOURCE_ID);
    }

    @Required
    public void setSettingDao(SettingDao settingDao) {
        m_settingDao = settingDao;
    }

    public void storeConfigChange(final ConfigChange configChange) throws SystemAuditException {
        getHibernateTemplate().executeWithNewSession(
                new HibernateCallback<ConfigChange>() {
                    @Override
                    public ConfigChange doInHibernate(Session session)
                        throws HibernateException, SQLException {
                        if (!configChange.isNew()) {
                            session.merge(configChange);
                        } else {
                            session.save(configChange);
                        }
                        return configChange;
                    }
                });
    }

    @Override
    public int getConfigChangesCount(SystemAuditFilter filter) {
        DetachedCriteria c = DetachedCriteria.forClass(ConfigChange.class);
        c.setProjection(Projections.rowCount());
        addFilterCriteria(c, filter);
        Long count = (Long) getHibernateTemplate().findByCriteria(c).get(0);
        return count.intValue();
    }

    @Required
    public void setCoreContext(CoreContext coreContext) {
        m_coreContext = coreContext;
    }

    private DetachedCriteria createCriteria(Class clazz, Integer groupId, String[] orderBy,
            boolean orderAscending, SystemAuditFilter filter) {
        DetachedCriteria c = DetachedCriteria.forClass(clazz);
        addByGroupCriteria(c, groupId);
        if (filter != null) {
            addFilterCriteria(c, filter);
        }
        if (orderBy != null) {
            for (String o : orderBy) {
                Order order = orderAscending ? Order.asc(o) : Order.desc(o);
                c.addOrder(order);
            }
        }
        return c;
    }

    private List<ConfigChange> loadConfigChangesByFilter(String[] orderBy,
            boolean orderAscending, SystemAuditFilter filter) {
        DetachedCriteria c = createCriteria(ConfigChange.class, null, orderBy, orderAscending,
                filter);
        return getHibernateTemplate().findByCriteria(c);
    }

    @Override
    public void dumpSystemAuditLogs(PrintWriter writer, SystemAuditFilter filter) {
        // create header
        writer.print("date_time" + COMMA_SEPARATOR);
        writer.print("user_name" + COMMA_SEPARATOR);
        writer.print("ip_address" + COMMA_SEPARATOR);
        writer.print("config_change_type" + COMMA_SEPARATOR);
        writer.print("config_change_action" + COMMA_SEPARATOR);
        writer.println(DETAILS + COMMA_SEPARATOR);

        // fill the table
        List<ConfigChange> configChangesList = loadConfigChangesByFilter(
                new String[] {DATE_TIME}, false, filter);
        for (ConfigChange configChange : configChangesList) {
            writer.print(configChange.getDateTime() + COMMA_SEPARATOR);
            writer.print(configChange.getUserName() + COMMA_SEPARATOR);
            writer.print(configChange.getIpAddress() + COMMA_SEPARATOR);
            writer.print(configChange.getConfigChangeType() + COMMA_SEPARATOR);
            writer.print(configChange.getConfigChangeAction() + COMMA_SEPARATOR);
            writer.println(configChange.getDetails() + COMMA_SEPARATOR);
        }
    }

}
