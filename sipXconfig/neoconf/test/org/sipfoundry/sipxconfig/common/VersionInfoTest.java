/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.common;

import org.apache.commons.lang.StringUtils;

import junit.framework.TestCase;

public class VersionInfoTest extends TestCase {

    public void testGetLongVersionStringEmpty() {
        // no package information during tests
        VersionInfo info = new VersionInfo();
        assertEquals("", info.getVersion());
    }

    public void testGetVersionIds() {
        Integer[] ids = VersionInfo.versionStringToVersionIds("12.34.56");
        assertEquals(3, ids.length);
        assertEquals(12, ids[0].intValue());
        assertEquals(34, ids[1].intValue());
        assertEquals(56, ids[2].intValue());
        assertEquals(0, VersionInfo.versionStringToVersionIds(null).length);
    }

    public void testVersionDetails() {
        VersionInfo info = new VersionInfo() {
            String getBuildStamp() {
                return "date host";
            }

            public String getVersion() {
                return "version";
            }
        };
        assertEquals("version|date|host", StringUtils.join(info.getVersionDetails(), '|'));
    }

    public void testVersionDetailsNoJar() {
        VersionInfo info = new VersionInfo() {
            String getBuildStamp() {
                return null;
            }

            public String getVersion() {
                return null;
            }
        };
        assertEquals("|", StringUtils.join(info.getVersionDetails(), '|'));
    }
}
