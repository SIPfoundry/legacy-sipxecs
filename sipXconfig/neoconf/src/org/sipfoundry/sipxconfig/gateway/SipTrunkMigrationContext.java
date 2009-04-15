/*
 *
 *
 * Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.gateway;

public interface SipTrunkMigrationContext {
    public static final String CONTEXT_BEAN_NAME = "sipTrunkMigrationContext";

    void migrateSipTrunk();
}
