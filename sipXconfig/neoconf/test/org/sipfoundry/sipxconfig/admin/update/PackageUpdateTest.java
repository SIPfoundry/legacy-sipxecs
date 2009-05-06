/*
 *
 *
 * Copyright (C) 2009 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 *
 */

package org.sipfoundry.sipxconfig.admin.update;

import org.sipfoundry.sipxconfig.admin.update.PackageUpdate;

import junit.framework.TestCase;

public class PackageUpdateTest extends TestCase {


    public void testParsePackageInfo() {
        String commentLine = "# This is a comment";
        assertNull(PackageUpdate.parse(commentLine));

        String packageLine = "sipxecs|3.11.6|3.11.8";
        PackageUpdate packageUpdate = PackageUpdate.parse(packageLine);
        assertNotNull(packageUpdate);
        assertEquals("sipxecs", packageUpdate.getPackageName());
        assertEquals("3.11.6", packageUpdate.getCurrentVersion());
        assertEquals("3.11.8", packageUpdate.getUpdatedVersion());

        String invalidLine = "sipxecs|3.11.6*3.11.8";
        assertNull(PackageUpdate.parse(invalidLine));
    }


}
