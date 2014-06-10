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
package org.sipfoundry.sipxconfig.freeswitch.config;

import static org.junit.Assert.assertEquals;

import java.io.IOException;
import java.io.StringWriter;

import org.apache.commons.io.IOUtils;
import org.easymock.EasyMock;
import org.easymock.IMocksControl;
import org.junit.Before;
import org.junit.Test;
import org.sipfoundry.sipxconfig.commserver.Location;
import org.sipfoundry.sipxconfig.feature.FeatureManager;
import org.sipfoundry.sipxconfig.freeswitch.FreeswitchSettings;
import org.sipfoundry.sipxconfig.ivr.Ivr;
import org.sipfoundry.sipxconfig.test.TestHelper;

public class VarsConfigurationTest {
   private VarsConfiguration m_configuration;

   @Before
   public void setUp() {
       m_configuration = new VarsConfiguration();
       m_configuration.setVelocityEngine(TestHelper.getVelocityEngine());
   }
   
   @Test
   public void testConfigVMDisabled() throws IOException {
       FreeswitchSettings settings = new FreeswitchSettings();
       StringWriter actual = new StringWriter();
       settings.setModelFilesContext(TestHelper.getModelFilesContext());
       Location location = new Location();
       IMocksControl mc = EasyMock.createControl();
       FeatureManager mgr = mc.createMock(FeatureManager.class);
       mgr.isFeatureEnabled(Ivr.FEATURE, location);
       mc.andReturn(false);
       mc.replay();
       m_configuration.setFeatureManager(mgr);
       m_configuration.write(actual, location, settings);
       String expected = IOUtils.toString(getClass().getResourceAsStream("vars.test.xml"));
       assertEquals(expected, actual.toString());
   }

   @Test
   public void testConfigVMEnabled() throws IOException {
       FreeswitchSettings settings = new FreeswitchSettings();
       StringWriter actual = new StringWriter();
       settings.setModelFilesContext(TestHelper.getModelFilesContext());
       Location location = new Location();
       location.setFqdn("toor.mydomain.org");
       IMocksControl mc = EasyMock.createControl();
       FeatureManager mgr = mc.createMock(FeatureManager.class);
       mgr.isFeatureEnabled(Ivr.FEATURE, location);
       mc.andReturn(true);
       mc.replay();
       m_configuration.setFeatureManager(mgr);
       m_configuration.write(actual, location, settings);
       String expected = IOUtils.toString(getClass().getResourceAsStream("vars_vm_enabled.test.xml"));
       assertEquals(expected, actual.toString());
   }
}
