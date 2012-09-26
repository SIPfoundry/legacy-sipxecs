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

import org.junit.Test;
import org.sipfoundry.sipxconfig.test.TestHelper;

public class CachedReleaseInfoTest {

    @Test
    public void testCache() {
        ReleaseInfo delegate = new ReleaseInfo();
        File f1 = TestHelper.getResourceAsFile(getClass(), "release");
        File f2 = TestHelper.getResourceAsFile(getClass(), "release-new");
        delegate.setPackageInfoExec("/bin/echo");
        delegate.setReleaseInfoFile(f1.getAbsolutePath());        
        CachedReleaseInfo cache = new CachedReleaseInfo();
        cache.setDelegate(delegate);
        String actual = cache.getReleaseUpdate();
        assertEquals("99", actual);
        assertSame(actual, cache.getReleaseUpdate());
        delegate.setReleaseInfoFile(f2.getAbsolutePath());
        assertEquals("100", cache.getReleaseUpdate());
    }
}
