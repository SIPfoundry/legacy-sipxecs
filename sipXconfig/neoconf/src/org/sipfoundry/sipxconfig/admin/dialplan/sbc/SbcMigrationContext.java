/*
 *
 *
 * Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.admin.dialplan.sbc;

public interface SbcMigrationContext {
    public static final String CONTEXT_BEAN_NAME = "sbcMigrationContext";

    void migrateSbc();
}
