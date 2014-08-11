/**
 *
 * Copyright (c) 2013 Karel Electronics Corp. All rights reserved.
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
 *
 */

package org.sipfoundry.sipxconfig.systemaudit;

import java.util.HashMap;
import java.util.HashSet;
import java.util.Map;
import java.util.Set;

import org.sipfoundry.sipxconfig.commserver.Location;
import org.sipfoundry.sipxconfig.feature.FeatureChangeRequest;
import org.sipfoundry.sipxconfig.feature.FeatureChangeValidator;
import org.sipfoundry.sipxconfig.feature.FeatureManager;
import org.sipfoundry.sipxconfig.feature.GlobalFeature;
import org.sipfoundry.sipxconfig.feature.LocationFeature;
import org.springframework.beans.factory.annotation.Required;

/**
 * This class contains the business logic for ENABLE or DISABLE actions done to
 * FEATURES
 */
public class FeatureAuditHandler extends AbstractSystemAuditHandler {

    private FeatureManager m_featureManager;

    public void handleFeaturesConfigChange(FeatureChangeRequest request) throws SystemAuditException {
        Map<Location, Set<LocationFeature>> enabledByLocation = request.getNewlyEnabledByLocation();
        handleFeaturesList(enabledByLocation, ConfigChangeAction.ENABLED, true);
        Map<Location, Set<LocationFeature>> disabledByLocation = request.getNewlyDisabledByLocation();
        handleFeaturesList(disabledByLocation, ConfigChangeAction.DISABLED, false);

        Set<GlobalFeature> enabled = request.getNewlyEnable();
        handleFeaturesList(enabled, ConfigChangeAction.ENABLED, true);
        Set<GlobalFeature> disabled = request.getNewlyDisable();
        handleFeaturesList(disabled, ConfigChangeAction.DISABLED, false);
    }

    public void handlePrecommitFeaturesConfigChange(FeatureManager manager, FeatureChangeValidator validator) {
        FeatureChangeRequest request = validator.getRequest();
        handlePrecommitFeaturesByLocation(manager, request);
        handlePrecommitGlobalFeatures(manager, request);
    }

    private void handlePrecommitGlobalFeatures(FeatureManager manager, FeatureChangeRequest request) {
        Set<GlobalFeature> enabled = request.getEnable();
        Set<GlobalFeature> disabled = request.getDisable();

        Set<GlobalFeature> newlyDisabledGlobalFeature = handleDuplicateGlobalFeatures(manager, disabled, true);
        Set<GlobalFeature> newlyEnabledGlobalFeature = handleDuplicateGlobalFeatures(manager, enabled, false);

        request.setNewlyDisable(newlyDisabledGlobalFeature);
        request.setNewlyEnable(newlyEnabledGlobalFeature);
    }

    private void handlePrecommitFeaturesByLocation(FeatureManager manager, FeatureChangeRequest request) {
        Map<Location, Set<LocationFeature>> enabledByLocation = request.getEnableByLocation();
        Map<Location, Set<LocationFeature>> disabledByLocation = request.getDisableByLocation();

        Map<Location, Set<LocationFeature>> newlyDisabledByLocation = handleDuplicateFeaturesByLocation(
                manager, disabledByLocation, true);
        Map<Location, Set<LocationFeature>> newlyEnabledByLocation = handleDuplicateFeaturesByLocation(
                manager, enabledByLocation, false);

        request.setNewlyDisabledByLocation(newlyDisabledByLocation);
        request.setNewlyEnabledByLocation(newlyEnabledByLocation);
    }

    private Set<GlobalFeature> handleDuplicateGlobalFeatures(FeatureManager manager,
            Set<GlobalFeature> featureSet, boolean isFeatureEnabled) {
        Set<GlobalFeature> newlyGlobalFeatureSet = new HashSet<GlobalFeature>();
        for (GlobalFeature globalFeature : featureSet) {
            boolean actualCondition = manager.isFeatureEnabled(globalFeature);
            if (actualCondition == isFeatureEnabled) {
                newlyGlobalFeatureSet.add(globalFeature);
            }
        }
        return newlyGlobalFeatureSet;
    }

    private Map<Location, Set<LocationFeature>> handleDuplicateFeaturesByLocation(FeatureManager manager,
            Map<Location, Set<LocationFeature>> featureSet, boolean isFeatureEnabled) {
        Map<Location, Set<LocationFeature>> newlyFeatureByLocation = new HashMap<Location, Set<LocationFeature>>();
        for (Location location : featureSet.keySet()) {
            Set<LocationFeature> newlyFeatureSet = new HashSet<LocationFeature>();
            for (LocationFeature locationFeature : featureSet.get(location)) {
                boolean actualCondition = manager.isFeatureEnabled(locationFeature, location);
                if (actualCondition == isFeatureEnabled) {
                    newlyFeatureSet.add(locationFeature);
                }
            }
            newlyFeatureByLocation.put(location, newlyFeatureSet);
        }
        return newlyFeatureByLocation;
    }

    /**
     * Create and stores ConfigChanges for featureByLocation
     *
     * @param featureMap
     * @param configChangeAction
     * @throws SystemAuditException
     */
    private void handleFeaturesList(Map<Location, Set<LocationFeature>> featureMap,
            ConfigChangeAction configChangeAction, boolean isFeatureEnabled) throws SystemAuditException {
        for (Location location : featureMap.keySet()) {
            for (LocationFeature locationFeature : featureMap.get(location)) {
                boolean actualCondition = m_featureManager.isFeatureEnabled(locationFeature, location);
                if (actualCondition == isFeatureEnabled) {
                    ConfigChange configChange = buildConfigChange(configChangeAction,
                            ConfigChangeType.FEATURE.getName());
                    configChange.setDetails(locationFeature.getId());
                    getConfigChangeContext().storeConfigChange(configChange);
                }
            }
        }
    }

    /**
     * Create and stores ConfigChanges for globalFeatures
     *
     * @param featureSet
     * @param configChangeAction
     * @throws SystemAuditException
     */
    private void handleFeaturesList(Set<GlobalFeature> featureSet, ConfigChangeAction configChangeAction,
            boolean isFeatureEnabled) throws SystemAuditException {
        for (GlobalFeature globalFeature : featureSet) {
            boolean actualCondition = m_featureManager.isFeatureEnabled(globalFeature);
            if (actualCondition == isFeatureEnabled) {
                ConfigChange configChange = buildConfigChange(configChangeAction,
                       ConfigChangeType.FEATURE.getName());
                if (configChange != null) {
                    configChange.setDetails(globalFeature.getId());
                    getConfigChangeContext().storeConfigChange(configChange);
                }
            }
        }
    }

    @Required
    public void setFeatureManager(FeatureManager featureManager) {
        m_featureManager = featureManager;
    }
}
