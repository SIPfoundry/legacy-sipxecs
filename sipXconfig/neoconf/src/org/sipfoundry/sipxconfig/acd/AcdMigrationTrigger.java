/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.acd;

import org.sipfoundry.sipxconfig.common.InitTaskListener;

public class AcdMigrationTrigger extends InitTaskListener {
    private AcdContext m_acdContext;

    public void setAcdContext(AcdContext acdContext) {
        m_acdContext = acdContext;
    }

    @Override
    public void onInitTask(String task) {
        if ("acd_migrate_line_extensions".equals(task)) {
            m_acdContext.migrateLineExtensions();
        } else if ("acd_migrate_overflow_queues".equals(task)) {
            m_acdContext.migrateOverflowQueues();
        } else if ("acd_server_migrate_acd_service".equals(task)) {
            m_acdContext.migrateAcdServers();
        }
    }
}
