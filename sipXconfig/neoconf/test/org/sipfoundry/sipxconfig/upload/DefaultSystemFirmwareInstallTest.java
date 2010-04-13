/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 */
package org.sipfoundry.sipxconfig.upload;

import java.io.File;
import java.io.IOException;
import java.util.Arrays;
import java.util.List;

import org.apache.commons.io.FileUtils;
import org.sipfoundry.sipxconfig.test.TestUtil;

import junit.framework.TestCase;

import static org.easymock.EasyMock.anyObject;
import static org.easymock.EasyMock.expectLastCall;
import static org.easymock.EasyMock.same;
import static org.easymock.classextension.EasyMock.createMock;
import static org.easymock.classextension.EasyMock.replay;
import static org.easymock.classextension.EasyMock.verify;

public class DefaultSystemFirmwareInstallTest extends TestCase {

    private DefaultSystemFirmwareInstall m_defaultSystemFirmwareInstall;
    private File m_firmwareDir;

    @Override
    protected void setUp() throws IOException {
        File thisDir = new File(TestUtil.getTestSourceDirectory(getClass()));

        m_firmwareDir = TestUtil.createTempDir("firmware-test");

        m_defaultSystemFirmwareInstall = new DefaultSystemFirmwareInstall();
        m_defaultSystemFirmwareInstall.setFirmwareDirectory(m_firmwareDir.getPath());

        FileUtils.copyDirectory(new File(thisDir, "firmware-dir"), m_firmwareDir);
    }

    @Override
    protected void tearDown() throws Exception {
        FileUtils.deleteDirectory(m_firmwareDir);
    }

    public void testFindAvailableFirmwares() {
        boolean found_firmware_one = false;
        boolean found_firmware_two = false;
        boolean found_firmware_three = false;

        String[] firmware_one = {
            "", "testFirmware", "firmware", m_firmwareDir.getPath() + "/firmware.cfg"
        };
        String[] firmware_two = {
            "", "test2Firmware", "firmware2", m_firmwareDir.getPath() + "/firmware2.cfg"
        };
        String[] firmware_three = {
            "", "test3Firmware", "firmware3", m_firmwareDir.getPath() + "/firmware3.cfg"
        };

        List<DefaultSystemFirmware> firmwares = m_defaultSystemFirmwareInstall.findAvailableFirmwares();
        assertEquals(firmwares.size(), 3);

        for (DefaultSystemFirmware defaultSystemFirmware : firmwares) {
            String[] args = defaultSystemFirmware.getUploadArgs();

            if (Arrays.equals(firmware_one, args)) {
                found_firmware_one = true;
            } else if (Arrays.equals(firmware_two, args)) {
                found_firmware_two = true;
            } else if (Arrays.equals(firmware_three, args)) {
                found_firmware_three = true;
            }
        }

        assertTrue(found_firmware_one);
        assertTrue(found_firmware_two);
        assertTrue(found_firmware_three);

        // nothing should be found if we try again...
        firmwares = m_defaultSystemFirmwareInstall.findAvailableFirmwares();
        assertTrue(firmwares.isEmpty());
    }

    // FIXME: temporarily disabled see XX-5840
    public void _testInstall() {
        Upload upload1 = new Upload();
        upload1.setUniqueId();
        Upload upload2 = new Upload();
        upload2.setUniqueId();
        Upload upload3 = new Upload();
        upload3.setUniqueId();

        UploadUtil uploadUtil = createMock(UploadUtil.class);
        uploadUtil.addUpload((String[]) anyObject(), (String) anyObject());
        expectLastCall().andReturn(upload1).once();
        uploadUtil.addUpload((String[]) anyObject(), (String) anyObject());
        expectLastCall().andReturn(upload2).once();
        uploadUtil.addUpload((String[]) anyObject(), (String) anyObject());
        expectLastCall().andReturn(upload3).once();

        uploadUtil.setUploads(same(upload1), (String[]) anyObject());
        uploadUtil.setUploads(same(upload2), (String[]) anyObject());
        uploadUtil.setUploads(same(upload3), (String[]) anyObject());

        uploadUtil.deploy((Upload) anyObject());
        uploadUtil.forceDeploy((Upload) anyObject());

        replay(uploadUtil);

        m_defaultSystemFirmwareInstall.setUploadUtil(uploadUtil);
        m_defaultSystemFirmwareInstall.installAvailableFirmwares();

        verify(uploadUtil);
    }
}
