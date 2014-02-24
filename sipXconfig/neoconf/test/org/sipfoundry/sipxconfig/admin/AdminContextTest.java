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

import org.easymock.EasyMock;
import org.junit.Test;
import org.sipfoundry.sipxconfig.backup.ArchiveDefinition;
import org.sipfoundry.sipxconfig.backup.BackupManager;
import org.sipfoundry.sipxconfig.backup.BackupPlan;
import org.sipfoundry.sipxconfig.backup.BackupSettings;
import org.sipfoundry.sipxconfig.commserver.Location;
import org.sipfoundry.sipxconfig.commserver.LocationsManager;
import org.sipfoundry.sipxconfig.domain.Domain;
import org.sipfoundry.sipxconfig.domain.DomainManager;
import org.sipfoundry.sipxconfig.test.TestHelper;


public class AdminContextTest {

    @Test
    public void archiveDefinition() {
        AdminContextImpl impl = new AdminContextImpl();
        BackupManager mgr = createMock(BackupManager.class);
        replay(mgr);
        DomainManager domainMgr = createMock(DomainManager.class);
        Domain domain = new Domain("example.com");
        domainMgr.getDomain();
        EasyMock.expectLastCall().andReturn(domain).anyTimes();
        replay(domainMgr);
        LocationsManager locationMgr = createMock(LocationsManager.class);
        Location location = new Location("test.example.com");
        locationMgr.getPrimaryLocation();
        EasyMock.expectLastCall().andReturn(location);
        replay(locationMgr);
        impl.setDomainManager(domainMgr);
        impl.setLocationsManager(locationMgr);
        BackupSettings settings = new BackupSettings();
        BackupPlan plan = new BackupPlan();
        plan.setIncludeDeviceFiles(true);
        settings.setModelFilesContext(TestHelper.getModelFilesContext());
        Location primary = new Location("one.example.org", "1.1.1.1");
        primary.setPrimary(true);
        ArchiveDefinition def = impl.getArchiveDefinitions(mgr, primary, plan, settings).iterator().next();
        String base = "sipxconfig-archive --restore %s";
        String baseBackup = "sipxconfig-archive --backup %s";
        assertEquals(base, def.getRestoreCommand());
        assertEquals(baseBackup, def.getBackupCommand());
        
        settings.setSettingTypedValue("restore/keepDomain", Boolean.TRUE);
        settings.setSettingTypedValue("restore/keepFqdn", Boolean.TRUE);
        settings.setSettingTypedValue("restore/decodePins", Boolean.TRUE);
        settings.setSettingTypedValue("restore/decodePinMaxLen", (Integer)5);
        settings.setSettingTypedValue("restore/resetPin", "zzz");
        settings.setSettingTypedValue("restore/resetPassword", "yyy");
        plan.setIncludeDeviceFiles(false);
        def = impl.getArchiveDefinitions(mgr, primary, plan, settings).iterator().next();
        String options = " --domain example.com --fqdn test.example.com --crack-pin zzz --crack-passwd yyy --crack-pin-len 5";
        String backupOptions = " --no-device-files";
        assertEquals(base + options, def.getRestoreCommand());
        assertEquals(baseBackup + backupOptions, def.getBackupCommand());
        verify(mgr);
    }
}
