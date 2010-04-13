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

import org.sipfoundry.sipxconfig.common.InitTaskListener;

public class SipTrunkMigrationTrigger extends InitTaskListener {
    private SipTrunkMigrationContext m_sipTrunkMigrationContext;

    public void setSipTrunkMigrationContext(SipTrunkMigrationContext sipTrunkMigrationContext) {
        m_sipTrunkMigrationContext = sipTrunkMigrationContext;
    }

    @Override
    public void onInitTask(String task) {
        if ("sip_trunk_address_migrate_sbc_device".equals(task)) {
            m_sipTrunkMigrationContext.migrateSipTrunk();
        }
    }
}
