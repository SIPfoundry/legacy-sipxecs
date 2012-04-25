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
package org.sipfoundry.sipxconfig.im;

import org.sipfoundry.sipxconfig.address.AddressType;
import org.sipfoundry.sipxconfig.feature.LocationFeature;

/**
 * Implementation is in sipXopenfire project
 */
public abstract class ImManager  {
    public static final String URL_FORMAT = "http://%s:%d/xmlrpc";
    public static final LocationFeature FEATURE = new LocationFeature("instantMessage");
    public static final AddressType XMPP_ADDRESS = new AddressType("instantMessageXmpp");
    public static final AddressType XMLRPC_ADDRESS = new AddressType("instantMessageXmlrpc",
        URL_FORMAT);
    public static final AddressType XMLRPC_VCARD_ADDRESS = new AddressType("instantMessageVcardXmlrpc",
            URL_FORMAT);
    public static final AddressType WATCHER_ADDRESS = new AddressType("instantMessageWatcher");
}
