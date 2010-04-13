/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.admin.commserver;

import java.util.Collection;

import org.sipfoundry.sipxconfig.admin.forwarding.AliasMapping;

public interface AliasProvider {
    /**
     * @return collection of AliasMapping objects
     */
    Collection<AliasMapping> getAliasMappings();
}
