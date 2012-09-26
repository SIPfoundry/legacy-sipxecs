/**
 *
 *
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
package org.sipfoundry.sipxconfig.common;

import static org.junit.Assert.assertEquals;

import java.io.File;
import java.io.IOException;

import org.junit.Test;
import org.sipfoundry.sipxconfig.test.TestHelper;

public class ReleaseInfoTest {

    @Test
    public void testReleaseUpdate() throws IOException {
        ReleaseInfo info = new ReleaseInfo();
        File f = TestHelper.getResourceAsFile(getClass(), "release");
        info.setReleaseInfoFile(f.getAbsolutePath());
        assertEquals("99", info.getReleaseUpdate());
    }
    
    @Test
    public void testPackageInfo() throws IOException {
        ReleaseInfo info = new ReleaseInfo();
        info.setPackageInfoExec("/bin/echo");
        assertEquals("\n", info.getPackageInfo());
        info.setPackageInfoExec("/bin/bogus-command");
        assertEquals("n/a", info.getPackageInfo());
    }
}
