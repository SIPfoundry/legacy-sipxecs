/*
 * Copyright (C) 2011 eZuce Inc., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the AGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.mongo;

import org.sipfoundry.sipxconfig.address.AddressType;
import org.sipfoundry.sipxconfig.feature.LocationFeature;

public interface MongoManager {
    public static final String BEAN_ID = "mongo";
    public static final AddressType ADDRESS_ID = new AddressType(BEAN_ID);
    public static final LocationFeature FEATURE_ID = new LocationFeature(BEAN_ID);

    public MongoSettings getSettings();

    public void saveSettings(MongoSettings settings);
}
