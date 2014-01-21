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
package org.sipfoundry.sipxconfig.admin;

import static org.easymock.EasyMock.createMock;
import static org.easymock.EasyMock.replay;
import static org.easymock.EasyMock.verify;
import static org.junit.Assert.assertEquals;

import org.junit.Test;
import org.sipfoundry.sipxconfig.backup.ArchiveDefinition;
import org.sipfoundry.sipxconfig.backup.BackupManager;
import org.sipfoundry.sipxconfig.backup.BackupSettings;
import org.sipfoundry.sipxconfig.commserver.Location;
import org.sipfoundry.sipxconfig.test.TestHelper;


public class AdminContextTest {

    @Test
    public void archiveDefinition() {
        AdminContextImpl impl = new AdminContextImpl();
        BackupManager mgr = createMock(BackupManager.class);
        replay(mgr);
        BackupSettings settings = new BackupSettings();
        settings.setModelFilesContext(TestHelper.getModelFilesContext());
        Location primary = new Location("one.example.org", "1.1.1.1");
        primary.setPrimary(true);
        ArchiveDefinition def = impl.getArchiveDefinitions(mgr, primary, settings).iterator().next();
        String base = "sipxconfig-archive --restore %s";
        String baseBackup = "sipxconfig-archive --backup %s";
        assertEquals(base, def.getRestoreCommand());
        assertEquals(baseBackup, def.getBackupCommand());
        settings.setSettingTypedValue("backup/device", Boolean.FALSE);
        settings.setSettingTypedValue("restore/keepDomain", Boolean.TRUE);
        settings.setSettingTypedValue("restore/keepFqdn", Boolean.TRUE);
        settings.setSettingTypedValue("restore/decodePins", Boolean.TRUE);
        settings.setSettingTypedValue("restore/decodePinMaxLen", (Integer)5);
        settings.setSettingTypedValue("restore/resetPin", "zzz");
        settings.setSettingTypedValue("restore/resetPassword", "yyy");
        def = impl.getArchiveDefinitions(mgr, primary, settings).iterator().next();
        String options = " --domain $(sipx.domain) --fqdn $(sipx.host).$(sipx.net_domain) --crack-pin zzz --crack-passwd yyy --crack-pin-len 5";
        String backupOptions = " --no-device-files";
        assertEquals(base + options, def.getRestoreCommand());
        assertEquals(baseBackup + backupOptions, def.getBackupCommand());
        verify(mgr);
    }
}
