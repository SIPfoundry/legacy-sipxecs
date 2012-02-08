/*
 * Copyright (C) 2011 eZuce Inc., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the AGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.restserver;

import org.sipfoundry.sipxconfig.address.AddressType;
import org.sipfoundry.sipxconfig.feature.LocationFeature;

public interface RestServer {
    public static final LocationFeature FEATURE = new LocationFeature("restServer");
    public static final AddressType HTTPS_API = new AddressType("restServerApi", "https://%s:%d");
    public static final AddressType EXTERNAL_API = new AddressType("restServerExternalApi");
    public static final AddressType SIP_TCP = AddressType.sip("restServerSip");

    public RestServerSettings getSettings();

    public void saveSettings(RestServerSettings settings);
}
