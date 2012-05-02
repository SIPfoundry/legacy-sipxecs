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

import org.apache.commons.collections.CollectionUtils;
import org.apache.commons.collections.Predicate;

public class Bundle {

    /** Minimal VoIP system */
    public static final Bundle CORE_TELEPHONY = new Bundle("coreTelephony");

    /** Minimal VoIP system */
    //public static final Bundle ADVANCED_TELEPHONY = new Bundle("advancedTelephony");

    /** Minimal system plus media services */
    public static final Bundle CORE = new Bundle("core");

    /** Minimal system plus media services */
    //public static final Bundle ADVANCED = new Bundle("advanced");

    /** ACD */
    public static final Bundle CALL_CENTER = new Bundle("callCenter");

    /** Used to hide features that are not ready  */
    public static final Bundle EXPERIMENTAL = new Bundle("experimental");

    /** Fully integrated media and IM services */
    public static final Bundle IM = new Bundle("im");

    /** End-point provisioning */
    public static final Bundle PROVISION = new Bundle("provision");

    //public static final Bundle SIP_TRUNKING = new Bundle("sipTrunking");

    private String m_id;
    private Map<Feature, BundleConstraint> m_features = new HashMap<Feature, BundleConstraint>();

    public Bundle(String id) {
        m_id = id;
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

    public Collection<GlobalFeature> getGlobalFeatures() {
        return CollectionUtils.select(m_features.keySet(), new Predicate() {
            public boolean evaluate(Object arg0) {
                return arg0 instanceof GlobalFeature;
            }
        });
    }

    public Collection<LocationFeature> getLocationFeatures() {
        return CollectionUtils.select(m_features.keySet(), new Predicate() {
            public boolean evaluate(Object arg0) {
                return arg0 instanceof LocationFeature;
            }
        });
    }
}
