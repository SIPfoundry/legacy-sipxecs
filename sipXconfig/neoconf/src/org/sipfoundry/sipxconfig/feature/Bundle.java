/**
 *
 *
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

import java.util.Collection;
import java.util.HashMap;
import java.util.Map;

public class Bundle {

    /** Minimal system */
    public static final Bundle ROUTER = new Bundle("router");

    /** Things that don't fit anywhere */
    public static final Bundle OTHER = new Bundle("other");

    /** Minimal system plus media services */
    public static final Bundle BASIC = new Bundle("basic", ROUTER);

    /** Fully integrated media and IM services */
    public static final Bundle UNIFIED_COMMUNICATIONS = new Bundle("uc", BASIC);

    private String m_id;
    private Bundle[] m_basedOn;
    private Map<Feature, BundleConstraint> m_features = new HashMap<Feature, BundleConstraint>();

    public Bundle(String id) {
        m_id = id;
    }

    public Bundle(String id, Bundle...basedOn) {
        m_id = id;
        m_basedOn = basedOn;
    }

    public boolean isBasic() {
        return basedOn(BASIC);
    }

    public boolean isRouter() {
        return basedOn(ROUTER);
    }

    public boolean isUnifiedCommunications() {
        return basedOn(UNIFIED_COMMUNICATIONS);
    }

    public boolean basedOn(Bundle b) {
        if (m_id.equals(b.m_id)) {
            return true;
        }
        if (m_basedOn != null) {
            for (Bundle parent : m_basedOn) {
                // recurse
                if (parent.basedOn(b)) {
                    return true;
                }
            }
        }
        return false;
    }

    public void addFeature(Feature f, BundleConstraint c) {
        m_features.put(f, c);
    }

    public void addFeature(Feature f) {
        m_features.put(f, BundleConstraint.DEFAULT);
    }

    public String getId() {
        return m_id;
    }

    public BundleConstraint getConstraint(Feature f) {
        return m_features.get(f);
    }

    public Feature getFeature(String featureId) {
        for (Feature f : m_features.keySet()) {
            if (f.getId().equals(featureId)) {
                return f;
            }
        }
        return null;
    }

    public Collection<Feature> getFeatures() {
        return m_features.keySet();
    }
}
