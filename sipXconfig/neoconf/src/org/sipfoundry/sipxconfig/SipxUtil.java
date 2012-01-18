/**
 *
 *
 * Copyright (c) 2010 / 2011 eZuce, Inc. All rights reserved.
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
package org.sipfoundry.sipxconfig;

import java.io.File;
import java.io.IOException;

/**
 * This is a utility class with static utilities methods that can be used in sipxecs source and
 * test layers as well This class should not contain any dependencies on test libraries, otherwise
 * the source sipxecs layer won't be ale to load this class
 */
public final class SipxUtil {
    private SipxUtil() {

    }

    public static File createTempDir(String name) throws IOException {
        File createTempFile = File.createTempFile(name, "dir");
        String tempDirPath = createTempFile.getPath();
        createTempFile.delete();
        File tempDir = new File(tempDirPath);
        tempDir.mkdirs();
        return tempDir;
    }
}
