/*
 *
 *
 * Copyright (C) 2009 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 *
 */

package org.sipfoundry.sipxconfig.admin.commserver;

import java.util.ArrayList;
import java.util.Collection;
import java.util.HashMap;
import java.util.HashSet;
import java.util.List;
import java.util.Map;
import java.util.Map.Entry;
import java.util.Set;

import org.sipfoundry.sipxconfig.service.SipxService;

public class RestartNeededState {
    private final Map<String, Set<String>> m_cache = new HashMap<String, Set<String>>();

    Set<String> getServices(Location location) {
        return getServices(location.getFqdn());
    }

    Set<String> getServices(String fqdn) {
        Set<String> services = m_cache.get(fqdn);
        if (services == null) {
            services = new HashSet<String>();
            m_cache.put(fqdn, services);
        }
        return services;
    }

    public void mark(Location location, SipxService service) {
        Set<String> services = getServices(location);
        services.add(service.getBeanId());
    }

    public void mark(Location location, Collection< ? extends SipxService> services) {
        for (SipxService sipxService : services) {
            mark(location, sipxService);
        }
    }

    public void unmark(Location location, SipxService service) {
        Set<String> services = getServices(location);
        services.remove(service.getBeanId());
    }

    public void unmark(Location location, Collection< ? extends SipxService> services) {
        for (SipxService sipxService : services) {
            unmark(location, sipxService);
        }
    }

    public void unmark(RestartNeededService serviceToRestart) {
        Set<String> services = getServices(serviceToRestart.getLocation());
        services.remove(serviceToRestart.getServiceBeanId());
    }

    public void unmark(Collection<RestartNeededService> servicesToUnmark) {
        for (RestartNeededService service : servicesToUnmark) {
            unmark(service);
        }
    }

    public boolean isMarked(Location location, SipxService service) {
        Set<String> services = getServices(location);
        return services.contains(service.getBeanId());
    }

    public boolean isEmpty() {
        if (m_cache.isEmpty()) {
            return true;
        }
        for (Set services : m_cache.values()) {
            if (!services.isEmpty()) {
                return false;
            }
        }
        return true;
    }

    public Collection<RestartNeededService> getAffected() {
        List<RestartNeededService> affected = new ArrayList<RestartNeededService>();
        for (Entry<String, Set<String>> locationAndService : m_cache.entrySet()) {
            String locationFqdn = locationAndService.getKey();
            for (String serviceBeanId : locationAndService.getValue()) {
                affected.add(new RestartNeededService(locationFqdn, serviceBeanId));
            }
        }
        return affected;
    }
}
