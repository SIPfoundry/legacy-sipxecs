/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 *
 */
package org.sipfoundry.sipxconfig.security;

import org.acegisecurity.userdetails.UserDetails;
import org.acegisecurity.userdetails.UserDetailsService;
import org.acegisecurity.userdetails.UsernameNotFoundException;
import org.sipfoundry.sipxconfig.admin.commserver.Location;
import org.sipfoundry.sipxconfig.admin.commserver.LocationsManager;
import org.springframework.beans.factory.annotation.Required;

public class LocationUserService implements UserDetailsService {
    private LocationsManager m_locationsManager;

    public UserDetails loadUserByUsername(String userNameOrAlias) {
        Location location = m_locationsManager.getLocationByFqdn(userNameOrAlias);
        if (location == null) {
            throw new UsernameNotFoundException(userNameOrAlias);
        }
        return new LocationDetailsImpl(location);
    }

    @Required
    public void setLocationsManager(LocationsManager locationsManager) {
        m_locationsManager = locationsManager;
    }
}
