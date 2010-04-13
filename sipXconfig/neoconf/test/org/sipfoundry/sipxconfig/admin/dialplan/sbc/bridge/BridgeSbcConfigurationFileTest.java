/*
 *
 *
 * Copyright (C) 2009 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 *
 */

package org.sipfoundry.sipxconfig.admin.dialplan.sbc.bridge;

import static org.easymock.EasyMock.*;

import java.util.Collections;

import org.sipfoundry.sipxconfig.TestHelper;
import org.sipfoundry.sipxconfig.admin.commserver.Location;
import org.sipfoundry.sipxconfig.admin.commserver.LocationsManager;
import org.sipfoundry.sipxconfig.admin.dialplan.sbc.SbcDescriptor;
import org.sipfoundry.sipxconfig.admin.dialplan.sbc.SbcDeviceManager;
import org.sipfoundry.sipxconfig.device.DeviceDefaults;
import org.sipfoundry.sipxconfig.gateway.GatewayContext;
import org.sipfoundry.sipxconfig.gateway.SipTrunk;
import org.sipfoundry.sipxconfig.nattraversal.NatLocation;
import org.sipfoundry.sipxconfig.phone.PhoneTestDriver;
import org.sipfoundry.sipxconfig.service.SipxServiceTestBase;
import org.sipfoundry.sipxconfig.setting.ModelFilesContext;

public class BridgeSbcConfigurationFileTest extends SipxServiceTestBase {
    public void testWrite() throws Exception {
        ModelFilesContext modelFilesContext = TestHelper.getModelFilesContext();
        DeviceDefaults deviceDefaults = PhoneTestDriver.getDeviceDefaults();
        Location location = createDefaultLocation();

        NatLocation natLocation = new NatLocation();
        natLocation.setPublicAddress("192.168.5.240");

        location.setNat(natLocation);

        SbcDescriptor descriptor = new SbcDescriptor();
        descriptor.setModelId("sbcSipXbridge");

        BridgeSbc sbc = new BridgeSbc();
        sbc.setLocation(location);
        sbc.setAddress(location.getAddress());
        sbc.setModel(descriptor);
        sbc.setProfileName("sibxbridge.xml");
        sbc.setDefaults(deviceDefaults);
        sbc.setModelFilesContext(modelFilesContext);
        sbc.setPort(5090);
        TestHelper.setVelocityProfileGenerator(sbc);

        BridgeSbcConfigurationFile out = new BridgeSbcConfigurationFile();
        out.setName("sipxbridge.xml");

        SbcDeviceManager sbcDeviceManager = createMock(SbcDeviceManager.class);
        sbcDeviceManager.getBridgeSbc(location);
        expectLastCall().andReturn(sbc);

        GatewayContext gatewayContext = createMock(GatewayContext.class);
        gatewayContext.getGatewayByType(SipTrunk.class);
        expectLastCall().andReturn(Collections.emptyList()).anyTimes();

        LocationsManager locationsManager = createMock(LocationsManager.class);
        locationsManager.getLocationByAddress(location.getAddress());
        expectLastCall().andReturn(location);

        replay(sbcDeviceManager, gatewayContext, locationsManager);

        out.setSbcDeviceManager(sbcDeviceManager);
        sbc.setGatewayContext(gatewayContext);
        sbc.setLocationsManager(locationsManager);

        assertCorrectFileGeneration(out, "sipxbridge-no-itsp.test.xml");
    }
}
