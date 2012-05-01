/**
 * Copyright (c) 2012 eZuce, Inc. All rights reserved.
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
package org.sipfoundry.sipxconfig.feature;


import java.util.ArrayList;
import java.util.Collection;
import java.util.List;
import java.util.Set;

import org.apache.commons.collections.CollectionUtils;
import org.apache.commons.collections.Predicate;
import org.sipfoundry.sipxconfig.common.DataCollectionUtil;
import org.sipfoundry.sipxconfig.commserver.Location;

public class FeatureChangeValidator {
    private FeatureManager m_manager;
    private FeatureChangeRequest m_request;
    private Set<LocationFeature> m_currentEnabledByLocation;
    private Set<GlobalFeature> m_currentEnabled;
    private List<InvalidChange> m_invalid;

    public FeatureChangeValidator(FeatureManager manager, FeatureChangeRequest request) {
        m_manager = manager;
        m_request = request;
        m_currentEnabledByLocation = m_manager.getEnabledLocationFeatures();
        m_currentEnabled = m_manager.getEnabledGlobalFeatures();
        m_invalid = new ArrayList<InvalidChange>();
    }

    public void requiresGlobalFeature(Feature subject, GlobalFeature required) {
        if (!isEnabledSomewhere(subject)) {
            return;
        }
        if (m_request.getAllNewlyEnabledFeatures().contains(required)) {
            return;
        }
        if (!m_request.getAllNewlyDisabledFeatures().contains(required)) {
            if (m_manager.isFeatureEnabled(required)) {
                return;
            }
        }
        m_invalid.add(InvalidChange.requires(subject, required));
    }

    public boolean isValid() {
        return m_invalid.isEmpty();
    }

    public void requiredOnSameHost(LocationFeature subject, LocationFeature required) {
        if (!isEnabledSomewhere(subject)) {
            return;
        }
        Collection<Location> requiredLocations = m_request.getLocationsForEnabledFeature(required);
        final Collection<Integer> requiredLocationIds = DataCollectionUtil.extractPrimaryKeys(requiredLocations);
        Collection<Location> subjectLocations = m_request.getLocationsForEnabledFeature(subject);
        Collection<Location> select = CollectionUtils.select(subjectLocations, new Predicate() {
            public boolean evaluate(Object arg0) {
                return !requiredLocationIds.contains(((Location) arg0).getId());
            }
        });
        for (Location invalid : select) {
            m_invalid.add(InvalidChange.requires(subject, required, invalid));
        }
    }

    Collection<Location> getLocationsForEnabledFeature(LocationFeature feature) {
        Collection<Location> newly = m_request.getLocationsForEnabledFeature(feature);
        Collection<Location> current = m_manager.getLocationsForEnabledFeature(feature);
        return DataCollectionUtil.merge(newly, current);
    }

    public void requiresAtLeastOne(Feature subject, LocationFeature required) {
        if (isEnabledSomewhere(subject) && !isEnabledSomewhere(required)) {
            m_invalid.add(InvalidChange.requires(subject, required));
        }
    }

    public void enableAtSameLocationAs(LocationFeature what, LocationFeature where) {
        Collection<Location> newly = m_request.getLocationsForEnabledFeature(where);
        Location location;
        if (!newly.isEmpty()) {
            location = newly.iterator().next();
        } else {
            location = m_manager.getLocationsForEnabledFeature(what).get(0);
        }
        m_request.getEnableByLocation().get(location).add(what);
    }

    public boolean isEnabledSomewhere(Feature f) {
        if (m_request.getAllNewlyEnabledFeatures().contains(f)) {
            return true;
        }
        if (f instanceof LocationFeature) {
            return m_currentEnabledByLocation.contains((LocationFeature) f);
        }

        return m_currentEnabled.contains((GlobalFeature) f);
    }

    public FeatureChangeRequest getRequest() {
        return m_request;
    }

    public List<InvalidChange> getInvalidChanges() {
        return m_invalid;
    }
}
