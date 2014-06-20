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

import java.sql.SQLException;
import java.util.List;

import org.hibernate.HibernateException;
import org.hibernate.Session;
import org.hibernate.criterion.DetachedCriteria;
import org.hibernate.criterion.Order;
import org.hibernate.criterion.Projections;
import org.hibernate.criterion.Restrictions;
import org.sipfoundry.sipxconfig.common.SipxHibernateDaoSupport;
import org.sipfoundry.sipxconfig.common.UserIpAddress;
import org.sipfoundry.sipxconfig.setting.Group;
import org.sipfoundry.sipxconfig.setting.SettingDao;
import org.springframework.beans.factory.annotation.Required;
import org.springframework.orm.hibernate3.HibernateCallback;

public class ConfigChangeContextImpl extends SipxHibernateDaoSupport<ConfigChange> implements
        ConfigChangeContext {

    private SettingDao m_settingDao;

    @Override
    public List<ConfigChange> getConfigChanges() {
        return getHibernateTemplate().loadAll(ConfigChange.class);
    }

    @Override
    public List<ConfigChange> loadConfigChangesByPage(Integer groupId, int firstRow, int pageSize,
            String[] orderBy, boolean orderAscending, SystemAuditFilter filter) {
        DetachedCriteria c = DetachedCriteria.forClass(ConfigChange.class);
        addByGroupCriteria(c, groupId);
        addFilterCriteria(c, filter);
        if (orderBy != null) {
            for (String o : orderBy) {
                Order order = orderAscending ? Order.asc(o) : Order.desc(o);
                c.addOrder(order);
            }
        }
        return getHibernateTemplate().findByCriteria(c, firstRow, pageSize);
    }

    /**
     * Add SystemAuditFilter to hibernate criteria
     *
     * @param crit
     * @param filter
     */
    private void addFilterCriteria(DetachedCriteria crit, SystemAuditFilter filter) {
        crit.add(Restrictions.between("dateTime", filter.getStartDate(), filter.getEndDate()));
        ConfigChangeType type = filter.getType();
        if (type != null && type != ConfigChangeType.ALL) {
            crit.add(Restrictions.eq("configChangeType", type));
        }
        ConfigChangeAction action = filter.getAction();
        if (action != null && action != ConfigChangeAction.ALL) {
            crit.add(Restrictions.eq("configChangeAction", action));
        }
        String userName = filter.getUserName();
        if (userName != null) {
            String userIpAddress = "userIpAddress";
            crit.createAlias(userIpAddress, userIpAddress);
            crit.add(Restrictions.eq("userIpAddress.userName", userName));
        }
        String details = filter.getDetails();
        if (details != null) {
            crit.add(Restrictions.eq("details", details));
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
        getHibernateTemplate().executeWithNewSession(new HibernateCallback<ConfigChange>() {
            @Override
            public ConfigChange doInHibernate(Session session) throws HibernateException, SQLException {
                UserIpAddress userIpAddress = configChange.getUserIpAddress();
                if (userIpAddress != null && userIpAddress.isNew()) {
                    session.save(userIpAddress);
                }
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
        return (Integer) getHibernateTemplate().findByCriteria(c).get(0);
    }

}
