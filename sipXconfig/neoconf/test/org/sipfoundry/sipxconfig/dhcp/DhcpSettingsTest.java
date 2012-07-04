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
package org.sipfoundry.sipxconfig.dhcp;

import static org.easymock.EasyMock.createMock;
import static org.easymock.EasyMock.expectLastCall;
import static org.easymock.EasyMock.replay;
import static org.easymock.EasyMock.verify;
import static org.junit.Assert.assertEquals;

import org.junit.Test;
import org.sipfoundry.sipxconfig.commserver.Location;
import org.sipfoundry.sipxconfig.commserver.LocationsManager;
import org.sipfoundry.sipxconfig.test.TestHelper;


public class DhcpSettingsTest {
    
    @Test
    public void settings() {
        LocationsManager lmgr = createMock(LocationsManager.class);
        lmgr.getPrimaryLocation();
        Location l = new Location("crane", "1.2.3.4");
        expectLastCall().andReturn(l).once(); // only once verifies cache
        replay(lmgr);
        
        DhcpSettings settings = new DhcpSettings();
        settings.setModelFilesContext(TestHelper.getModelFilesContext());
        settings.setLocationsManager(lmgr);
        assertEquals("1.2.3.50", settings.getSettingTypedValue("dhcpd-config/range_begin"));
        assertEquals("1.2.3.250", settings.getSettingTypedValue("dhcpd-config/range_end"));
        assertEquals("1.2.3.0", settings.getSettingTypedValue("dhcpd-config/subnet"));
        verify(lmgr);
    }
}
