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
package org.sipfoundry.sipxconfig.restserver;

import org.sipfoundry.sipxconfig.address.AddressType;
import org.sipfoundry.sipxconfig.feature.LocationFeature;

public interface RestServer {
    public static final String HTTP_FORMAT = "http://%s:%d";
    public static final LocationFeature FEATURE = new LocationFeature("restServer");
    public static final AddressType HTTP_API = new AddressType("restServerApi", HTTP_FORMAT);
    public static final AddressType PUBLIC_HTTP_API = new AddressType("publicRestServerApi", HTTP_FORMAT);
    public static final AddressType SIP_TCP = AddressType.sipTcp("restServerSip");

    public RestServerSettings getSettings();

    public void saveSettings(RestServerSettings settings);
}
