/**
 * Copyright (c) 2012 eZuce, Inc. All rights reserved.
 * Contributed to SIPfoundry under a Contributor Agreement
 *
 * This software is free software; you can redistribute it and/or modify it under
 * the terms of the Affero General Public License (AGPL) as published by the
 * Free Software Foundation; either version 3 of the License, or (at your option)
 * any later version.
 *
 * This software is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more
 * details.
 */
package org.sipfoundry.sipxconfig.backup;

import org.apache.commons.collections.Transformer;

public class ArchiveDefinition implements Comparable {
    public static final Transformer GET_IDS = new Transformer() {
        public Object transform(Object arg0) {
            return ((ArchiveDefinition) arg0).getId();
        }
    };
    private String m_id;
    private String m_backupCommand;
    private String m_restoreCommand;
    //if the service is running on more than one node, execute backup only on one of them
    private boolean m_singleNodeBackup = true;
    private boolean m_singleNodeRestore = true;

    public ArchiveDefinition(String id, String backupCommand, String restoreCommand) {
        m_id = id;
        m_backupCommand = backupCommand;
        m_restoreCommand = restoreCommand;
    }

    public ArchiveDefinition(String id, String backupCommand, String restoreCommand,
        boolean singleNodeBackup, boolean singleNodeRestore) {
        this(id, backupCommand, restoreCommand);
        m_singleNodeBackup = singleNodeBackup;
        m_singleNodeRestore = singleNodeRestore;
    }

    public String getId() {
        return m_id;
    }

    public String getBackupCommand() {
        return m_backupCommand;
    }

    public String getRestoreCommand() {
        return m_restoreCommand;
    }

    public boolean isSingleNodeBackup() {
        return m_singleNodeBackup;
    }

    public boolean isSingleNodeRestore() {
        return m_singleNodeRestore;
    }

    @Override
    public int compareTo(Object arg0) {
        ArchiveDefinition arch = (ArchiveDefinition) arg0;
        return arch.getId().compareTo(getId());
    }
}
