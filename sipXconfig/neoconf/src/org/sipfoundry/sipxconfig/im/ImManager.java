/*
 * Copyright (C) 2011 eZuce Inc., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the AGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.im;

import org.sipfoundry.sipxconfig.address.AddressType;
import org.sipfoundry.sipxconfig.feature.LocationFeature;

/**
 * Implementation is in sipXopenfire project
 */
public abstract class ImManager  {
    public static final LocationFeature FEATURE = new LocationFeature("instantMessage");
    public static final AddressType XMPP_ADDRESS = new AddressType("instantMessageXmpp");
    public static final AddressType XMLRPC_ADDRESS = new AddressType("instantMessageXmlrpc", 
        "http://%s:%d/xmlrpc");
    public static final AddressType WATCHER_ADDRESS = new AddressType("instantMessageWatcher");
}
