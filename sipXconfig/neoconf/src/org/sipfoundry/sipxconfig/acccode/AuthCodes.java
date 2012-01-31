/*
 * Copyright (C) 2011 eZuce Inc., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the AGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.acccode;

import org.sipfoundry.sipxconfig.feature.LocationFeature;

public interface AuthCodes {
    public static final LocationFeature FEATURE = new LocationFeature("authCode");

    public AuthCodeSettings getSettings();

    public void saveSettings(AuthCodeSettings settings);

}
