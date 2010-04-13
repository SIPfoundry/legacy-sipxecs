/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.admin.dialplan;

public interface AttendantMigrationContext {
    public static final String CONTEXT_BEAN_NAME = "attendantMigrationContext";

    void migrateAttendantRules();

    void setAttendantDefaults();
}
