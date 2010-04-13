/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.conference;

import java.io.Serializable;

import org.sipfoundry.sipxconfig.common.DataObjectSource;

public interface BridgeConferenceIdentity extends DataObjectSource {
    Conference load(Class<Conference> c, Serializable id);
    public void setBridge(Bridge bridge);
}
