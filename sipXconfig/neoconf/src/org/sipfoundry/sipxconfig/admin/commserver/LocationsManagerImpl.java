/*
 *
 *
 * Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 *
 */
package org.sipfoundry.sipxconfig.admin.commserver;

import java.util.List;

import org.hibernate.Criteria;
import org.hibernate.Session;
import org.hibernate.criterion.Criterion;
import org.hibernate.criterion.Restrictions;
import org.sipfoundry.sipxconfig.common.SipxHibernateDaoSupport;
import org.springframework.orm.hibernate3.HibernateCallback;
import static org.springframework.dao.support.DataAccessUtils.singleResult;

public class LocationsManagerImpl extends SipxHibernateDaoSupport<Location> implements LocationsManager {

    private static final String LOCATION_PROP_NAME = "fqdn";

    /** Return the replication URLs, retrieving them on demand */
    public Location[] getLocations() {
        List<Location> locationList = getHibernateTemplate().loadAll(Location.class);
        Location[] locationArray = new Location[locationList.size()];
        locationList.toArray(locationArray);
        return locationArray;
    }

    public Location getLocation(int id) {
        return (Location) getHibernateTemplate().load(Location.class, id);
    }


    public Location getLocationByFqdn(String fqdn) {
        return loadLocationByUniqueProperty(LOCATION_PROP_NAME, fqdn);
    }

    private Location loadLocationByUniqueProperty(String propName, String propValue) {
        final Criterion expression = Restrictions.eq(propName, propValue);

        HibernateCallback callback = new HibernateCallback() {
            public Object doInHibernate(Session session) {
                Criteria criteria = session.createCriteria(Location.class).add(expression);
                return criteria.list();
            }
        };
        List<Location> locations = getHibernateTemplate().executeFind(callback);
        Location location = (Location) singleResult(locations);

        return location;
    }

    public void storeLocation(Location location) {
        getHibernateTemplate().saveOrUpdate(location);
    }

    public void deleteLocation(Location location) {
        getHibernateTemplate().delete(location);
    }

    public Location getPrimaryLocation() {
        Location[] allLocations = getLocations();
        if (allLocations.length > 0) {
            return getLocations()[0];
        }
        return null;
    }
}
