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

import org.sipfoundry.sipxconfig.commserver.Location;
import org.sipfoundry.sipxconfig.commserver.LocationsManager;
import org.springframework.beans.factory.annotation.Required;
import org.springframework.security.core.userdetails.UserDetails;
import org.springframework.security.core.userdetails.UserDetailsService;
import org.springframework.security.core.userdetails.UsernameNotFoundException;

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
