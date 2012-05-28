/*
 * Copyright (C) 2012 eZuce Inc., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the AGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.apache;

import org.sipfoundry.sipxconfig.address.AddressType;
import org.sipfoundry.sipxconfig.feature.LocationFeature;

public interface ApacheManager {
    public static final LocationFeature FEATURE = new LocationFeature("apache");
    public static final AddressType HTTP_ADDRESS = new AddressType("apacheHttp", 80);
    public static final AddressType HTTPS_ADDRESS = new AddressType("apacheHttps", "https://%s", 443);

    public void avoidCheckstyleError();

}
