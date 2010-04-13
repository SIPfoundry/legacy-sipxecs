/*
 *
 *
 * Copyright (C) 2009 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 *
 */
package org.sipfoundry.sipxconfig.device;

import org.apache.commons.collections.Predicate;

public class FeatureFilter implements Predicate {
    private final String m_feature;

    public FeatureFilter(String feature) {
        m_feature = feature;
    }

    public boolean evaluate(Object object) {
        if (object instanceof FeatureProvider) {
            return ((FeatureProvider) object).isSupported(m_feature);
        }
        return false;
    }
}
