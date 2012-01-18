/*
 *
 *
 * Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 */
package org.sipfoundry.sipxconfig.freeswitch.config;

import static org.junit.Assert.assertEquals;

import java.io.IOException;
import java.io.StringWriter;

import org.apache.commons.io.IOUtils;
import org.junit.Before;
import org.junit.Test;
import org.sipfoundry.sipxconfig.freeswitch.FreeswitchSettings;
import org.sipfoundry.sipxconfig.test.TestHelper;

public class LogfileConfigurationTest {
   private LogfileConfiguration m_configuration;

   @Before
   public void setUp() {
       m_configuration = new LogfileConfiguration();
       m_configuration.setVelocityEngine(TestHelper.getVelocityEngine());
   }
   
   @Test
   public void config() throws IOException {
       FreeswitchSettings settings = new FreeswitchSettings();
       StringWriter actual = new StringWriter();
       settings.setModelFilesContext(TestHelper.getModelFilesContext());
       m_configuration.write(actual, null, settings);
       String expected = IOUtils.toString(getClass().getResourceAsStream("logfile.conf.test.xml"));
       assertEquals(expected, actual.toString());
   }
}
