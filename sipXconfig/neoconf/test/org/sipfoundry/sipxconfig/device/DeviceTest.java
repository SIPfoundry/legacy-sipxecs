/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.device;

import java.io.File;
import java.io.IOException;
import java.io.OutputStream;

import junit.framework.TestCase;

import org.apache.commons.io.IOUtils;
import org.sipfoundry.sipxconfig.TestHelper;
import org.sipfoundry.sipxconfig.common.NamedObject;
import org.sipfoundry.sipxconfig.gateway.Gateway;
import org.sipfoundry.sipxconfig.gateway.GatewayModel;
import org.sipfoundry.sipxconfig.phone.Phone;
import org.sipfoundry.sipxconfig.phone.TestPhone;
import org.sipfoundry.sipxconfig.phone.TestPhoneModel;
import org.sipfoundry.sipxconfig.setting.Setting;

public class DeviceTest extends TestCase {

    private final class ProfileGeneratorStub extends AbstractProfileGenerator {
        protected void generateProfile(ProfileContext context, OutputStream out) throws IOException {
            IOUtils.write("profile", out);
        }
    }

    public void testGenerateAndRemoveProfilesPhone() {
        AbstractProfileGenerator generator = new ProfileGeneratorStub();

        String root = TestHelper.getTestDirectory() + "/phone";
        FileSystemProfileLocation location = new FileSystemProfileLocation();
        location.setParentDir(root);

        final String profileFilename = "gateway.cfg";

        Phone device = new TestPhone() {
            public String getProfileFilename() {
                return profileFilename;
            }
        };
        device.setModel(new TestPhoneModel());

        device.setProfileGenerator(generator);

        File profile = new File(root, profileFilename);
        device.generateProfiles(location);
        assertTrue(profile.exists());
        device.removeProfiles(location);
        assertFalse(profile.exists());

        new File(root).delete();
    }

    public void testGenerateAndRemoveProfilesGateway() {
        AbstractProfileGenerator generator = new ProfileGeneratorStub();

        String root = TestHelper.getTestDirectory() + "/gateway";
        FileSystemProfileLocation location = new FileSystemProfileLocation();
        location.setParentDir(root);

        final String profileFilename = "gateway.cfg";

        Gateway device = new Gateway() {
            public String getProfileFilename() {
                return profileFilename;
            }
        };
        device.setModel(new GatewayModel("test"));

        device.setProfileGenerator(generator);

        File profile = new File(root, profileFilename);
        device.generateProfiles(location);
        assertTrue(profile.exists());
        device.removeProfiles(location);
        assertFalse(profile.exists());

        new File(root).delete();
    }

    public void testGetNiceName() {
        Device d = new Device() {
            private DeviceDescriptor m_descriptor = new DeviceDescriptor() {
            };

            public DeviceDescriptor getModel() {
                m_descriptor.setLabel("Acme Device");
                return m_descriptor;
            }

            protected Setting loadSettings() {
                return null;
            }

        };

        assertEquals("Acme Device", d.getNiceName());

        d.setSerialNumber("01234");
        assertEquals("01234", d.getNiceName());
    }

    static class NamedDevice extends Device implements NamedObject {
        private DeviceDescriptor m_descriptor = new DeviceDescriptor() {
        };
        private String m_name;

        public DeviceDescriptor getModel() {
            m_descriptor.setLabel("Acme Device");
            return m_descriptor;
        }

        protected Setting loadSettings() {
            return null;
        }

        public String getName() {
            return m_name;
        }

        public void setName(String name) {
            m_name = name;
        }

    }

    public void testGetNiceNameForNamedObject() {
        NamedDevice d = new NamedDevice();

        assertEquals("Acme Device", d.getNiceName());

        d.setSerialNumber("01234");
        assertEquals("01234", d.getNiceName());

        d.setName("bongo");
        assertEquals("bongo/01234", d.getNiceName());
    }

}
