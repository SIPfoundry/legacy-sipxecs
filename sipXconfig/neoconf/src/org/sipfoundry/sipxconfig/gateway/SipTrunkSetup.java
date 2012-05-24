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

import org.sipfoundry.sipxconfig.setup.MigrationListener;
import org.sipfoundry.sipxconfig.setup.SetupManager;

public class SipTrunkSetup implements MigrationListener {
    private SipTrunkMigrationContext m_sipTrunkMigrationContext;

    public void setSipTrunkMigrationContext(SipTrunkMigrationContext sipTrunkMigrationContext) {
        m_sipTrunkMigrationContext = sipTrunkMigrationContext;
    }

    @Override
    public void migrate(SetupManager manager) {
        String id = "sip_trunk_address_migrate_sbc_device";
        if (manager.isTrue(id)) {
            m_sipTrunkMigrationContext.migrateSipTrunk();
            manager.setFalse(id);
        }
    }
}
