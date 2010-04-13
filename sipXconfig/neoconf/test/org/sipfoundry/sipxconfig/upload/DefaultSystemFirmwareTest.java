/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.upload;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.List;

import org.sipfoundry.sipxconfig.test.TestUtil;

import junit.framework.TestCase;

public class DefaultSystemFirmwareTest extends TestCase {

    public void testGetUploadArgs() {
        String uploadSpec = "testUploadSpec";
        String rootDir = TestUtil.getTestSourceDirectory(getClass()) + "/firmware-dir/";

        List<String> fileSettings = new ArrayList<String>();
        List<String> uploadFiles = new ArrayList<String>();

        fileSettings.add("fs_one");
        fileSettings.add("fs_two");
        fileSettings.add("fs_three");
        fileSettings.add("fs_four");

        uploadFiles.add(rootDir + "firmware.cfg");
        uploadFiles.add(rootDir + "firmware2.cfg");
        uploadFiles.add(rootDir + "firmware3.cfg");
        uploadFiles.add(rootDir + "firmware_does_not_exisit.cfg");

        String[] args = {
            "", uploadSpec, fileSettings.get(0), uploadFiles.get(0), fileSettings.get(1), uploadFiles.get(1),
            fileSettings.get(2), uploadFiles.get(2)
        };

        DefaultSystemFirmware firmware = new DefaultSystemFirmware(uploadSpec, fileSettings,
                uploadFiles, "true", "true", "1.2");

        assertTrue(Arrays.equals(args, firmware.getUploadArgs()));

        fileSettings.add("fs_five");
        fileSettings.add("fs_six");

        firmware = new DefaultSystemFirmware(uploadSpec, fileSettings, uploadFiles, "true", "true", "1.2");

        assertTrue(Arrays.equals(args, firmware.getUploadArgs()));
    }
}
