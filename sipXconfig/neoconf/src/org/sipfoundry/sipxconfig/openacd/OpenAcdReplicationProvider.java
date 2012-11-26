/*
 * Copyright (C) 2012 eZuce Inc., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the AGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.openacd;

import java.util.List;

import org.sipfoundry.sipxconfig.common.Replicable;


public interface OpenAcdReplicationProvider {
    List<Replicable> getReplicables();
}
