/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 *
 */
package org.sipfoundry.sipxconfig.sbc;

import java.io.IOException;
import java.util.Collection;

import org.sipfoundry.sipxconfig.bridge.BridgeSbc;
import org.sipfoundry.sipxconfig.common.UserException;
import org.sipfoundry.sipxconfig.commserver.Location;
import org.sipfoundry.sipxconfig.commserver.LocationsManager;
import org.sipfoundry.sipxconfig.device.ModelSource;
import org.sipfoundry.sipxconfig.test.IntegrationTestCase;

public class SbcDeviceManagerImplTestIntegration extends IntegrationTestCase {
    private SbcDeviceManager m_sbcDeviceManager;
    private ModelSource<SbcDescriptor> m_modelSource;
    private LocationsManager m_locationsManager;    
    private Location m_location;

    @Override
    protected void onSetUpBeforeTransaction() throws Exception {
        super.onSetUpBeforeTransaction();
        clear();
    }
    
    @Override
    protected void onSetUpInTransaction() throws Exception {
        super.onSetUpInTransaction();
        db().execute("insert into location (location_id, name, fqdn, ip_address, primary_location) values (1, 'a', 'b', '10.1.2.1', true)");
        db().execute("insert into location (location_id, name, fqdn, ip_address) values (2, 'x', 'y', '10.1.2.5')");
        m_location = m_locationsManager.getLocation(2);
    }
    
    @Override
    protected void onTearDownInTransaction() throws Exception {
        // TODO Auto-generated method stub
        super.onTearDownInTransaction();
    }

    public void testNewSbc() {
        Collection<SbcDescriptor> models = m_modelSource.getModels();
        // at least generic model is empty
        assertFalse(models.isEmpty());
        for (SbcDescriptor model : models) {
            SbcDevice newSbcDevice = m_sbcDeviceManager.newSbcDevice(model);
            assertEquals(newSbcDevice.getBeanId(), model.getBeanId());
            assertEquals(newSbcDevice.getModelId(), model.getModelId());
            assertSame(newSbcDevice.getModel(), model);
        }
    }

    public void testClear() throws IOException {
        sql("sbc/sbc-device.sql");
        assertEquals(4, countRowsInTable("sbc_device"));
        m_sbcDeviceManager.clear();
        flush();

        assertEquals(0, countRowsInTable("sbc_device"));
    }

    public void testGetAllSbcDeviceIds() throws IOException {
        sql("sbc/sbc-device.sql");
        Collection<Integer> allSbcDeviceIds = m_sbcDeviceManager.getAllSbcDeviceIds();
        assertEquals(4, allSbcDeviceIds.size());
        assertTrue(allSbcDeviceIds.contains(1000));
        assertTrue(allSbcDeviceIds.contains(1001));
        assertTrue(allSbcDeviceIds.contains(1002));
        assertTrue(allSbcDeviceIds.contains(1003));
    }


    public void testSave() {
        SbcDescriptor model = m_modelSource.getModel("sbcGenericModel");
        SbcDevice newSbcDevice = m_sbcDeviceManager.newSbcDevice(model);
        m_sbcDeviceManager.saveSbcDevice(newSbcDevice);
        assertFalse(newSbcDevice.isNew());
        flush();

        assertEquals(1, countRowsInTable("sbc_device"));

        newSbcDevice.setName("abc");
        m_sbcDeviceManager.saveSbcDevice(newSbcDevice);
    }

    public void testSaveErrorMaxAllowed() {
        SbcDescriptor model = m_modelSource.getModel("sipXbridgeSbcModel");
        model.setMaxAllowed(1);
        SbcDevice newSbcDevice = m_sbcDeviceManager.newSbcDevice(model);
        newSbcDevice.setName("aaa");
        m_sbcDeviceManager.saveSbcDevice(newSbcDevice);
        flush();
        try {
            newSbcDevice = m_sbcDeviceManager.newSbcDevice(model);
            newSbcDevice.setName("bbb");
            m_sbcDeviceManager.saveSbcDevice(newSbcDevice);
            fail("Creation of two bridges should not be accepted.");
        } catch (UserException e) {
            // ok - 2 bridges failed
        }
    }

    public void testSaveDuplicate() {
        SbcDescriptor model = m_modelSource.getModel("sbcGenericModel");
        SbcDevice newSbcDevice = m_sbcDeviceManager.newSbcDevice(model);
        newSbcDevice.setName("aaa");
        m_sbcDeviceManager.saveSbcDevice(newSbcDevice);
        flush();

        try {
            newSbcDevice = m_sbcDeviceManager.newSbcDevice(model);
            newSbcDevice.setName("aaa");
            m_sbcDeviceManager.saveSbcDevice(newSbcDevice);
            fail("Should not accept duplicated name.");
        } catch (UserException e) {
            // ok - duplicate name
        }
    }

    public void testMaxAllowedLimitReached() throws IOException {
        sql("sbc/sbc-device.sql");
        SbcDescriptor bridgeModel = m_modelSource.getModel("sipXbridgeSbcModel");
        assertTrue(m_sbcDeviceManager.maxAllowedLimitReached(bridgeModel));
        SbcDescriptor sbcModel = m_modelSource.getModel("sbcGenericModel");
        assertFalse(m_sbcDeviceManager.maxAllowedLimitReached(sbcModel));
    }

    public void testGetSbcBridge() throws Exception {
        BridgeSbc bridgeSbc = m_sbcDeviceManager.newBridgeSbc(m_location);
        assertEquals("sipXbridge-" + m_location.getId().toString(), bridgeSbc.getName());
        assertEquals("10.1.2.5", bridgeSbc.getAddress());
        assertEquals("Internal SBC on y", bridgeSbc.getDescription());
    }
    
    public void testAutoRemove() throws Exception {
        m_locationsManager.deleteLocation(m_location);
        BridgeSbc bridgeSbc = m_sbcDeviceManager.getBridgeSbc(m_location);
        assertTrue(bridgeSbc == null);
    }

    public void setSbcDeviceManager(SbcDeviceManager sbcDeviceManager) {
        m_sbcDeviceManager = sbcDeviceManager;
    }

    public void setLocationsManager(LocationsManager locationsManager) {
        m_locationsManager = locationsManager;
    }

    public void setSbcModelSource(ModelSource<SbcDescriptor> modelSource) {
        m_modelSource = modelSource;
    }
}
