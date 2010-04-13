/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.device;

import java.util.Collection;
import java.util.TimeZone;

import org.sipfoundry.sipxconfig.common.SipxHibernateDaoSupport;
import org.springframework.dao.support.DataAccessUtils;
import org.springframework.orm.hibernate3.HibernateTemplate;

public class TimeZoneManagerImpl extends SipxHibernateDaoSupport implements TimeZoneManager {
    public void setDeviceTimeZone(DeviceTimeZone dtz) {
        HibernateTemplate hibernate = getHibernateTemplate();
        hibernate.saveOrUpdate(dtz);
    }

    public DeviceTimeZone getDeviceTimeZone() {
        Collection<DeviceTimeZone> timeZones = getHibernateTemplate().loadAll(DeviceTimeZone.class);
        return (DeviceTimeZone) DataAccessUtils.requiredSingleResult(timeZones);
    }

    public void saveDefault() {
        TimeZone tz = TimeZone.getDefault();
        DeviceTimeZone dtz = new DeviceTimeZone(tz);
        setDeviceTimeZone(dtz);
    }
}
