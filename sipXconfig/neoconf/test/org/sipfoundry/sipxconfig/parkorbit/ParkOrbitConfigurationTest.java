/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.parkorbit;


import static org.easymock.EasyMock.createMock;
import static org.easymock.EasyMock.expectLastCall;
import static org.easymock.EasyMock.replay;

import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.io.StringReader;
import java.io.StringWriter;
import java.util.ArrayList;
import java.util.List;

import org.apache.commons.io.IOUtils;
import org.custommonkey.xmlunit.XMLTestCase;
import org.custommonkey.xmlunit.XMLUnit;
import org.sipfoundry.sipxconfig.commserver.Location;
import org.sipfoundry.sipxconfig.dialplan.config.XmlFile;
import org.sipfoundry.sipxconfig.test.TestHelper;

public class ParkOrbitConfigurationTest extends XMLTestCase {
    private List<ParkOrbit> m_parkOrbits;

    private static String[][] DATA = {
        {
            "disable", "101", "x.wav", "this one should not be generated"
        }, {
            "sales", "400", "name with spaces.wav", "test description"
        }, {
            "CEO orbit", "179", "marketing.wav", null
        }
    };

    @Override
    protected void setUp() throws Exception {
        XMLUnit.setIgnoreWhitespace(true);

        m_parkOrbits = new ArrayList<ParkOrbit>(DATA.length);
        for (int i = 0; i < DATA.length; i++) {
            ParkOrbit orbit = new ParkOrbit();
            orbit.setModelFilesContext(TestHelper.getModelFilesContext());
            orbit.setName(DATA[i][0]);
            orbit.setExtension(DATA[i][1]);
            orbit.setMusic(DATA[i][2]);
            orbit.setDescription(DATA[i][3]);
            // the first one is disabled
            // the remaining orbits will be enabled
            orbit.setEnabled(i > 0);
            m_parkOrbits.add(orbit);
        }

        // make 3rd orbit more interesting

        ParkOrbit orbit = m_parkOrbits.get(2);
        orbit.setSettingValue("general/enableTimeout", "1");
        orbit.setSettingValue("general/parkTimeout", "150");
        orbit.setSettingValue("general/allowTransfer", "1");
        orbit.setSettingValue("general/transferKey", "9");
        orbit.setSettingValue("general/multipleCalls", "1");

    }
    
    public void testFlatConfig() throws IOException {
        Location location = new Location();
        location.setAddress("1.2.3.4");
        ParkSettings settings = new ParkSettings();
        settings.setModelFilesContext(TestHelper.getModelFilesContext());
        StringWriter actual = new StringWriter();
        ParkOrbitConfiguration orbits = new ParkOrbitConfiguration();
        orbits.write(actual, location, settings);
        InputStream expected = getClass().getResourceAsStream("expected-sipxpark-config");
        assertEquals(IOUtils.toString(expected), actual.toString());
    }

    public void testXmlConfig() throws Exception {
        ParkOrbitContext parkOrbitContext = createMock(ParkOrbitContext.class);
        parkOrbitContext.getDefaultMusicOnHold();
        expectLastCall().andReturn("default.wav");
        parkOrbitContext.getParkOrbits();
        expectLastCall().andReturn(m_parkOrbits);
        replay(parkOrbitContext);

        ParkOrbitConfiguration orbits = new ParkOrbitConfiguration();
        orbits.setAudioDirectory("/var/sipxdata/parkserver/music");
        orbits.setParkOrbitContext(parkOrbitContext);
        
        StringWriter xml = new StringWriter();
        XmlFile f = new XmlFile(xml);
        f.write(orbits.getDocument());

        // orbits file on windows includes "c:/" etc
        String actual = xml.toString().replaceAll(TestHelper.currentDrive(), "");
        actual = actual.replaceAll(TestHelper.currentDrive(), "");
        actual = actual.replace('\\', '/');
        InputStream referenceXml = getClass().getResourceAsStream("orbits.test.xml");
        assertXMLEqual(new InputStreamReader(referenceXml), new StringReader(actual));
    }    
}
