/*
 * Copyright (C) 2012 eZuce Inc., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the AGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.feature;

import java.util.Collection;
import java.util.HashMap;
import java.util.Map;

public class Bundle {

    /** Minimal system */
    public static final Bundle ROUTER = new Bundle("router");

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

    public Collection<Feature> getFeatures() {
        return m_features.keySet();
    }
}
