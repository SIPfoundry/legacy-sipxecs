/*
 *
 *
 * Copyright (C) 2010 eZuce, Inc. All rights reserved.
 *
 * This library is free software; you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License as published by the Free
 * Software Foundation; either version 2.1 of the License, or (at your option)
 * any later version.
 *
 * This library is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more
 * details.
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

public class ReloadNeededState {
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

    public void unmark(Location location, SipxService service) {
        Set<String> services = getServices(location);
        services.remove(service.getBeanId());
    }

    public void unmark(Location location, Collection< ? extends SipxService> services) {
        for (SipxService sipxService : services) {
            unmark(location, sipxService);
        }
    }

    public void unmark(ReloadNeededService serviceToRestart) {
        Set<String> services = getServices(serviceToRestart.getLocation());
        services.remove(serviceToRestart.getServiceBeanId());
    }

    public void unmark(Collection<ReloadNeededService> servicesToUnmark) {
        for (ReloadNeededService service : servicesToUnmark) {
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

    public Collection<ReloadNeededService> getAffected() {
        List<ReloadNeededService> affected = new ArrayList<ReloadNeededService>();
        for (Entry<String, Set<String>> locationAndService : m_cache.entrySet()) {
            String locationFqdn = locationAndService.getKey();
            for (String serviceBeanId : locationAndService.getValue()) {
                affected.add(new ReloadNeededService(locationFqdn, serviceBeanId));
            }
        }
        return affected;
    }
}
