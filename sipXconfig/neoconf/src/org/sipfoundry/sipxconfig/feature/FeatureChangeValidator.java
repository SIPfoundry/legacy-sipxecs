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

    public void singleLocationOnly(LocationFeature subject) {
        if (m_request.hasChanged(subject)) {
            Collection<Location> on = getLocationsForEnabledFeature(subject);
            if (on.size() > 1) {
                InvalidChangeException err = new InvalidChangeException("&error.singleLocationOnly.{0}", subject);
                InvalidChange singleLocation = new InvalidChange(subject, err);
                m_invalid.add(singleLocation);
            }
        }
    }

    public void primaryLocationOnly(LocationFeature subject) {
        if (m_request.hasChanged(subject)) {
            Collection<Location> on = getLocationsForEnabledFeature(subject);
            if (on.size() > 1 || !isInstalledOnPrimary(on)) {
                InvalidChangeException err = new InvalidChangeException("&error.primaryLocationOnly.{0}", subject);
                InvalidChange primaryLocation = new InvalidChange(subject, err);
                m_invalid.add(primaryLocation);
            }
        }
    }

    private boolean isInstalledOnPrimary(Collection<Location> locations) {
        boolean isPrimary = (locations.size() == 0 ? true : false);
        for (Location location : locations) {
            if (location.isPrimary()) {
                isPrimary = true;
            }
        }
        return isPrimary;
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
        Collection<Location> requiredLocations = getLocationsForEnabledFeature(required);
        Collection<Location> subjectLocations = getLocationsForEnabledFeature(subject);
        Collection<Location> missingLocations = DataCollectionUtil.remove(subjectLocations, requiredLocations);
        for (Location invalid : missingLocations) {
            m_invalid.add(InvalidChange.requires(subject, required, invalid));
        }
    }

    public Collection<Location> getLocationsForEnabledFeature(LocationFeature feature) {
        Collection<Location> newlyOn = m_request.getLocationsForEnabledFeature(feature);
        Collection<Location> wasOn = m_manager.getLocationsForEnabledFeature(feature);
        Collection<Location> nowOn = DataCollectionUtil.merge(newlyOn, wasOn);
        Collection<Location> newlyOff = m_request.getLocationsForDisabledFeature(feature);
        Collection<Location> on = DataCollectionUtil.remove(nowOn, newlyOff);
        return on;
    }

    public void requiresAtLeastOne(Feature subject, LocationFeature required) {
        if (isEnabledSomewhere(subject) && !isEnabledSomewhere(required)) {
            m_invalid.add(InvalidChange.requires(subject, required));
        }
    }

    public boolean isEnabledSomewhere(Feature f) {
        if (m_request.getAllNewlyEnabledFeatures().contains(f)) {
            return true;
        }
        if (m_request.getAllNewlyDisabledFeatures().contains(f)) {
            return false;
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
