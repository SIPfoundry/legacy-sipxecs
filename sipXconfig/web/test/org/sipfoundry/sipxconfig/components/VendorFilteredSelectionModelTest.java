/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.components;

import java.util.ArrayList;
import java.util.List;

import junit.framework.TestCase;

import org.sipfoundry.sipxconfig.phone.PhoneModel;
import org.sipfoundry.sipxconfig.phone.cisco.CiscoModel;
import org.sipfoundry.sipxconfig.phone.grandstream.GrandstreamModel;

/**
 * VendorFilteredSelectionModelTest
 */
public class VendorFilteredSelectionModelTest extends TestCase {

    private VendorFilteredDeviceSelectionModel m_model;

    private PhoneModel ciscoModel1;
    private PhoneModel grandstreamModel1;
    private PhoneModel ciscoModel2;

    protected void setUp() throws Exception {
        super.setUp();
        m_model = new VendorFilteredDeviceSelectionModel();
        m_model.setVendorFilter("cisco");
        m_model.setLabelExpression("label");
        m_model.setValueExpression("");

        List<PhoneModel> devicesModels = new ArrayList<PhoneModel>();

        ciscoModel1 = new CiscoModel();
        ciscoModel1.setBeanId("cisco1");
        ciscoModel1.setLabel("Cisco 1");
        ciscoModel1.setVendor("cisco");

        grandstreamModel1 = new GrandstreamModel();
        grandstreamModel1.setBeanId("grandstream1");
        grandstreamModel1.setLabel("GrandStream 1");
        grandstreamModel1.setVendor("grandstream");

        ciscoModel2 = new CiscoModel();
        ciscoModel2.setBeanId("cisco2");
        ciscoModel2.setLabel("Cisco 2");
        ciscoModel2.setVendor("cisco");

        devicesModels.add(ciscoModel1);
        devicesModels.add(grandstreamModel1);
        devicesModels.add(ciscoModel2);
        m_model.setCollection(devicesModels);
    }

    public void testGetOptionCount() {
        assertEquals(2, m_model.getOptionCount());
    }

    public void testGetOption() {
        assertEquals(ciscoModel1, m_model.getOption(0));
        assertEquals(ciscoModel2, m_model.getOption(1));
    }

    public void testGetLabel() {
        assertEquals("Cisco 1", m_model.getLabel(0));
        assertEquals("Cisco 2", m_model.getLabel(1));
    }

    public void testGetValue() {
        assertEquals("0", m_model.getValue(0));
        assertEquals("1", m_model.getValue(1));
    }

    public void testTranslateValue() {
        assertEquals(ciscoModel1, m_model.translateValue("0"));
        assertEquals(ciscoModel2, m_model.translateValue("1"));
    }
}
