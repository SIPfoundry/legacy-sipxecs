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
import org.sipfoundry.sipxconfig.address.AddressType.Protocol;
import org.sipfoundry.sipxconfig.feature.LocationFeature;

/**
 * Implementation is in sipXopenfire project
 */
public interface ImManager {
    final String URL_FORMAT = "http://%s:%d/xmlrpc";
    final String FEATURE_ID = "instantMessage";
    final LocationFeature FEATURE = new LocationFeature(FEATURE_ID);
    final AddressType XMPP_ADDRESS = new AddressType("instantMessageXmpp", 5222, Protocol.tcp);
    final AddressType XMPP_SECURE_ADDRESS = new AddressType("instantMessageSecureXmpp", 5223, Protocol.tcp);
    final AddressType XMPP_FEDERATION_ADDRESS = new AddressType("instantMessageFederation", Protocol.tcp);
    final AddressType XMPP_FILE_TRANSFER_PROXY_ADDRESS = new AddressType("instantMessageFileTransferProxy",
            Protocol.tcp);
    final AddressType XMPP_ADMIN_CONSOLE_ADDRESS = new AddressType("instantMessageAdminConsole", Protocol.tcp);
    final AddressType XMPP_ADMIN_CONSOLE_SECURE_ADDRESS = new AddressType("instantMessageSecureAdminConsole",
        Protocol.tcp);
    final AddressType XMPP_BOSH_ADDRESS = new AddressType("instantMessageHttpBinding", Protocol.tcp);
    final AddressType XMPP_BOSH_SECURE_ADDRESS = new AddressType("instantMessageHttpBindingSecure", Protocol.tcp);
    final AddressType XMLRPC_ADDRESS = new AddressType("instantMessageXmlrpc", URL_FORMAT,
        XMPP_ADMIN_CONSOLE_ADDRESS.getCanonicalPort());
    final AddressType XMLRPC_VCARD_ADDRESS = new AddressType("instantMessageVcardXmlrpc", URL_FORMAT);
    final AddressType WATCHER_ADDRESS = new AddressType("instantMessageWatcher");

    boolean isPresenceEnabled();
}
