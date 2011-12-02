/*
 * Copyright (C) 2011 eZuce Inc., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the AGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.registrar;

import org.sipfoundry.sipxconfig.admin.commserver.Location;
import org.sipfoundry.sipxconfig.feature.LocationFeature;
import org.sipfoundry.sipxconfig.feature.SimpleFeatureListener;

public class RegistrationFeatureListener extends SimpleFeatureListener {
    private Registrar m_registrar;

    @Override
    public void enableLocationFeature(FeatureEvent event, LocationFeature feature, Location location) {
        if (feature.equals(Registrar.FEATURE)) {
            if (event == FeatureEvent.PRE_ENABLE) {
                m_registrar.initialize();
            }
        }
    }
}
