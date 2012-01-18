/*
 * Copyright (C) 2011 eZuce Inc., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the AGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.saa;

import org.sipfoundry.sipxconfig.feature.LocationFeature;

public interface SaaManager {
    public static final LocationFeature FEATURE = new LocationFeature("saa");

    public SaaSettings getSettings();

    public void saveSettings(SaaSettings settings);
}
