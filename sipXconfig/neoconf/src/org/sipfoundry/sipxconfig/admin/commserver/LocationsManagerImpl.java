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

import org.sipfoundry.sipxconfig.common.SipxHibernateDaoSupport;

public class LocationsManagerImpl extends SipxHibernateDaoSupport<Location> implements LocationsManager {

    /** Return the replication URLs, retrieving them on demand */
    public synchronized Location[] getLocations() {
        List<Location> locationList =  getHibernateTemplate().loadAll(Location.class);
        Location[] locationArray = new Location[locationList.size()];
        locationList.toArray(locationArray);
        return locationArray;
    }

    public void storeLocation(Location location) {
        getHibernateTemplate().saveOrUpdate(location);
    }
    
    public void deleteLocation(Location location) {
        getHibernateTemplate().delete(location);
    }
}
