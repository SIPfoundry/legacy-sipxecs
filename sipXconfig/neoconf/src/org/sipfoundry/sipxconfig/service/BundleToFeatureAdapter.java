/*
 * Copyright (C) 2011 eZuce Inc., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the AGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.service;

import java.util.Arrays;
import java.util.Collection;
import java.util.Collections;

import org.sipfoundry.sipxconfig.commserver.Location;
import org.sipfoundry.sipxconfig.feature.FeatureProvider;
import org.sipfoundry.sipxconfig.feature.GlobalFeature;
import org.sipfoundry.sipxconfig.feature.LocationFeature;

public class BundleToFeatureAdapter implements FeatureProvider {
    public static final GlobalFeature ADMIN = new GlobalFeature("management");
    public static final LocationFeature SIP_PROXY = new LocationFeature("SipRouter");
    public static final LocationFeature VM = new LocationFeature("voicemail");
    public static final LocationFeature CONFERENCE = new LocationFeature("conference");
    public static final LocationFeature IM = new LocationFeature("im");
    public static final LocationFeature OPENACD = new LocationFeature("openAcd");
    private static final Collection<GlobalFeature> GLOBAL_FEATURES = Collections.singleton(ADMIN);
    private static final Collection<LocationFeature> ANYWHERE_LOCATION_FEATURES = Arrays.asList(new LocationFeature[] {
        SIP_PROXY, CONFERENCE, IM
    });
    private static final Collection<LocationFeature> LOCATION_FEATURES = Arrays.asList(new LocationFeature[] {
        SIP_PROXY, VM, CONFERENCE, IM, OPENACD
    });

    @Override
    public Collection<GlobalFeature> getAvailableGlobalFeatures() {
        return GLOBAL_FEATURES;
    }

    @Override
    public Collection<LocationFeature> getAvailableLocationFeatures(Location l) {
        if (l.isPrimary()) {
            return LOCATION_FEATURES;
        }

        return ANYWHERE_LOCATION_FEATURES;
    }
}
