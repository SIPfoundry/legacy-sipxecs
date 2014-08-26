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
        if (dtz.isNew()) {
            hibernate.save(dtz);
        } else {
            hibernate.merge(dtz);
        }
        getDaoEventPublisher().publishSave(dtz);
    }

    public DeviceTimeZone getDeviceTimeZone() {
        Collection<DeviceTimeZone> timeZones = getHibernateTemplate().loadAll(DeviceTimeZone.class);
        DeviceTimeZone dtz  = DataAccessUtils.singleResult(timeZones);
        if (dtz != null) {
            return dtz;
        }

        TimeZone tz = TimeZone.getDefault();
        dtz = new DeviceTimeZone();
        dtz.setTimeZone(tz);
        setDeviceTimeZone(dtz);
        return dtz;
    }
}
