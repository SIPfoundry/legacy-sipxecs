/*
 *
 *
 * Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.sbc;

import org.sipfoundry.sipxconfig.setup.MigrationListener;
import org.sipfoundry.sipxconfig.setup.SetupManager;

public class SbcSetup implements MigrationListener {
    private SbcMigrationContext m_sbcMigrationContext;

    public void setSbcMigrationContext(SbcMigrationContext sbcMigrationContext) {
        m_sbcMigrationContext = sbcMigrationContext;
    }

    @Override
    public void migrate(SetupManager manager) {
        String id = "sbc_address_migrate_sbc_device";
        if (manager.isTrue(id)) {
            m_sbcMigrationContext.migrateSbc();
            manager.setFalse(id);
        }
    }
}
