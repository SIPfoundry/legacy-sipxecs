/**
 *
 *
 * Copyright (c) 2010 / 2011 eZuce, Inc. All rights reserved.
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
 */
package org.sipfoundry.sipxconfig.commserver;

import java.util.HashMap;
import java.util.Map;
import java.util.Set;
import java.util.TreeSet;

public class FailedReplicationsState {
    private final Map<String, Set<String>> m_cacheFailed = new HashMap<String, Set<String>>();
    private final Map<String, Set<String>> m_cacheSucceded = new HashMap<String, Set<String>>();

    private void mark(String fqdn, String name, Map<String, Set<String>> cache) {
        Set<String> elementsList = cache.get(fqdn);
        if (elementsList == null) {
            elementsList = new TreeSet<String>();
            cache.put(fqdn, elementsList);
        }
        elementsList.add(name);
    }

    private void unmark(String fqdn, String name, Map<String, Set<String>> cache) {
        Set<String> elementsList = cache.get(fqdn);
        if (elementsList != null) {
            if (!elementsList.isEmpty()) {
                elementsList.remove(name);
            }
            if (elementsList.isEmpty()) {
                cache.remove(fqdn);
            }
        }
    }

    public void reset(String fqdn) {
        Set<String> failed = m_cacheFailed.get(fqdn);
        Set<String> success = m_cacheSucceded.get(fqdn);
        if (failed != null) {
            failed.clear();
        }
        if (success != null) {
            success.clear();
        }
    }

    public void saveState(Location location) {
        m_cacheFailed.put(location.getFqdn(), new TreeSet<String>(location.getFailedReplications()));
        //m_cacheSucceded.put(location.getFqdn(), new TreeSet<String>(location.getSuccessfulReplications()));
        m_cacheSucceded.clear();
    }

    public void mark(String fqdn, String name) {
        mark(fqdn, name, m_cacheFailed);
        unmark(fqdn, name, m_cacheSucceded);
    }

    public void unmark(String fqdn, String name) {
        mark(fqdn, name, m_cacheSucceded);
        unmark(fqdn, name, m_cacheFailed);
    }

    public Set<String> getFailedList(String fqdn) {
        //send profiles may be in progress when reading - we want to avoid concurent exception
        synchronized (m_cacheFailed) {
            return m_cacheFailed.get(fqdn);
        }
    }

    public Set<String> getSuccededList(String fqdn) {
        //send profiles may be in progress when reading - we want to avoid concurent exception
        synchronized (m_cacheSucceded) {
            return m_cacheSucceded.get(fqdn);
        }
    }
}
