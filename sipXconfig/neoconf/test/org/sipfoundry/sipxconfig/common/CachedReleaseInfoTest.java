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
import static org.junit.Assert.assertSame;

import java.io.File;
import java.io.IOException;

import org.apache.commons.io.FileUtils;
import org.junit.Test;
import org.sipfoundry.sipxconfig.test.TestHelper;

public class CachedReleaseInfoTest {

    @Test
    public void testCache() throws IOException, InterruptedException {
        ReleaseInfo delegate = new ReleaseInfo();
        String s = TestHelper.getTestOutputDirectory() + "/release-cache";
        File f = new File(s);
        FileUtils.writeStringToFile(f, "99");
        delegate.setPackageInfoExec("/bin/echo");
        delegate.setReleaseInfoFile(s);        
        CachedReleaseInfo cache = new CachedReleaseInfo();
        cache.setDelegate(delegate);
        String actual = cache.getReleaseUpdate();
        assertEquals("99", actual);
        assertSame(actual, cache.getReleaseUpdate());
        Thread.sleep(1500); // to ensure file as different timestamp
        FileUtils.writeStringToFile(f, "100");
        assertEquals("100", cache.getReleaseUpdate());
    }
}
