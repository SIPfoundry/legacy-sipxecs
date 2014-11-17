/*
 *
 *
 * Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 */
package org.sipfoundry.sipxconfig.freeswitch.config;

import static org.easymock.EasyMock.expect;
import static org.easymock.classextension.EasyMock.createMock;
import static org.easymock.classextension.EasyMock.replay;
import static org.junit.Assert.assertEquals;

import java.io.StringWriter;
import java.util.Collection;
import java.util.Collections;
import java.util.Comparator;
import java.util.LinkedList;
import java.util.List;
import java.util.Set;
import java.util.TreeSet;

import org.apache.commons.io.IOUtils;
import org.easymock.EasyMock;
import org.easymock.IMocksControl;
import org.junit.Before;
import org.junit.Test;
import org.sipfoundry.sipxconfig.commserver.Location;
import org.sipfoundry.sipxconfig.conference.Bridge;
import org.sipfoundry.sipxconfig.conference.Conference;
import org.sipfoundry.sipxconfig.domain.Domain;
import org.sipfoundry.sipxconfig.domain.DomainManagerImpl;
import org.sipfoundry.sipxconfig.feature.FeatureManager;
import org.sipfoundry.sipxconfig.freeswitch.FreeswitchAction;
import org.sipfoundry.sipxconfig.freeswitch.FreeswitchCondition;
import org.sipfoundry.sipxconfig.freeswitch.FreeswitchExtension;
import org.sipfoundry.sipxconfig.ivr.Ivr;
import org.sipfoundry.sipxconfig.parkorbit.ParkOrbit;
import org.sipfoundry.sipxconfig.setting.Setting;
import org.sipfoundry.sipxconfig.test.TestHelper;

public class DefaultContextConfigurationTest {
    private DefaultContextConfiguration m_configuration;

    private static String[][] DATA = {
        {
            "disable", "101", "0000", "000", "000", "", ""
        },
        {
            "sales", "400", "400111", "400222", "400AAA", "sip:sales@bridge.sipfoundry.org", "sales@400+400222"
        },
        {
            "marketing", "500", "500111", "500222", "500AAA", "sip:marketing@bridge.sipfoundry.org",
            "marketing@500+500222"
        }
    };

    @Before
    public void setUp() {
        m_configuration = new DefaultContextConfiguration();
        m_configuration.setVelocityEngine(TestHelper.getVelocityEngine());
        Domain domain = new Domain();
        domain.setName("ezuce.ro");
        DomainManagerImpl manager = new DomainManagerImpl();
        manager.setTestDomain(domain);
    }

    private Bridge createBridge() {
        Comparator<Conference> comparator = new Comparator<Conference>() {
            @Override
            public int compare(Conference o1, Conference o2) {
                return o1.getId().compareTo(o2.getId());
            }
        };
        Set<Conference> conferences = new TreeSet<Conference>(comparator);
        for (int i = 0; i < DATA.length; i++) {
            Conference conference = createMock(Conference.class);

            expect(conference.getId()).andReturn(i).anyTimes();
            expect(conference.getName()).andReturn(DATA[i][0]).anyTimes();
            expect(conference.getExtension()).andReturn(DATA[i][1]).anyTimes();
            expect(conference.getOrganizerAccessCode()).andReturn(DATA[i][2]).anyTimes();
            expect(conference.getParticipantAccessCode()).andReturn(DATA[i][3]).anyTimes();
            expect(conference.getRemoteAdmitSecret()).andReturn(DATA[i][4]).anyTimes();
            expect(conference.getUri()).andReturn(DATA[i][5]).anyTimes();
            expect(conference.getDialString()).andReturn(DATA[i][6]).once();

            // the first one is disabled
            expect(conference.isEnabled()).andReturn(i > 0);
            replay(conference);

            conferences.add(conference);
        }
        Bridge bridge = new Bridge();
        bridge.setConferences(conferences);
        return bridge;
    }

    static List<FreeswitchExtension> getExtensions() {
        FreeswitchExtension extension = new FreeswitchExtension() {
            @Override
            protected Setting loadSettings() {
                return null;
            }
        };

        extension.setName("sales");
        FreeswitchCondition condition = new FreeswitchCondition();
        condition.setField("destination_number");
        condition.setExpression("^300$");
        extension.addCondition(condition);
        FreeswitchAction action = new FreeswitchAction();
        action.setApplication("fly");
        action.setData("away");
        condition.addAction(action);
        return Collections.singletonList(extension);
    }

    @Test
    public void testMinimalConfig() throws Exception {
        StringWriter actual = new StringWriter();
        Location location = TestHelper.createDefaultLocation();
        Bridge bridge = new Bridge();
        List<FreeswitchExtension> extensions = Collections.emptyList();
        IMocksControl mc = EasyMock.createControl();
        FeatureManager mgr = mc.createMock(FeatureManager.class);
        mgr.getLocationsForEnabledFeature(Ivr.FEATURE);
        mc.andReturn(null);
        mc.replay();
        m_configuration.setFeatureManager(mgr);
        m_configuration.write(actual, location, bridge, false, false, null, extensions, false);
        String expected = IOUtils
                .toString(getClass().getResourceAsStream("default_context-no-conferences.test.xml"));
        assertEquals(expected, actual.toString());
    }

    @Test
    public void testExtensionsConfig() throws Exception {
        StringWriter actual = new StringWriter();
        Location location = TestHelper.createDefaultLocation();
        Bridge bridge = new Bridge();
        IMocksControl mc = EasyMock.createControl();
        FeatureManager mgr = mc.createMock(FeatureManager.class);
        mgr.getLocationsForEnabledFeature(Ivr.FEATURE);
        mc.andReturn(null);
        mc.replay();
        m_configuration.setFeatureManager(mgr);
        List<FreeswitchExtension> extensions = getExtensions();
        m_configuration.write(actual, location, bridge, false, false, null, extensions, true);
        String expected = IOUtils.toString(getClass().getResourceAsStream(
                "default_context_freeswitch_extensions.test.xml"));
        assertEquals(expected, actual.toString());
    }

    @Test
    public void testConferenceConfig() throws Exception {
        StringWriter actual = new StringWriter();
        Location location = TestHelper.createDefaultLocation();
        IMocksControl mc = EasyMock.createControl();
        FeatureManager mgr = mc.createMock(FeatureManager.class);
        mgr.getLocationsForEnabledFeature(Ivr.FEATURE);
        mc.andReturn(null);
        mc.replay();
        m_configuration.setFeatureManager(mgr);
        Bridge bridge = createBridge();
        List<FreeswitchExtension> extensions = Collections.emptyList();
        m_configuration.write(actual, location, bridge, false, false, null, extensions, false);
        String expected = IOUtils.toString(getClass().getResourceAsStream("default_context.test.xml"));
        assertEquals(expected, actual.toString());
    }

    @Test
    public void testAuthCodesConfig() throws Exception {
        StringWriter actual = new StringWriter();
        Location location = TestHelper.createDefaultLocation();
        IMocksControl mc = EasyMock.createControl();
        FeatureManager mgr = mc.createMock(FeatureManager.class);
        mgr.getLocationsForEnabledFeature(Ivr.FEATURE);
        mc.andReturn(null);
        mc.replay();
        m_configuration.setFeatureManager(mgr);
        Bridge bridge = new Bridge();
        List<FreeswitchExtension> extensions = Collections.emptyList();
        m_configuration.write(actual, location, bridge, true, false, null, extensions, false);
        String expected = IOUtils.toString(getClass().getResourceAsStream("default_context-authcodes.test.xml"));
        assertEquals(expected, actual.toString());
    }

    @Test
    public void testVMsConfig() throws Exception {
        StringWriter actual = new StringWriter();
        List<Location> locations = new LinkedList<Location>();
        Location manila = TestHelper.createDefaultLocation();
        manila.setAddress("10.1.1.0");
        locations.add(manila);
        Location boston = new Location("boston.test.org");
        boston.setAddress("10.1.1.1");
        locations.add(boston);
        Location bucharest = new Location("buc.test.org");
        bucharest.setAddress("10.1.1.2");
        locations.add(bucharest);
        IMocksControl mc = EasyMock.createControl();
        FeatureManager mgr = mc.createMock(FeatureManager.class);
        mgr.getLocationsForEnabledFeature(Ivr.FEATURE);
        mc.andReturn(locations);
        mc.replay();
        m_configuration.setFeatureManager(mgr);
        Bridge bridge = new Bridge();
        List<FreeswitchExtension> extensions = Collections.emptyList();
        m_configuration.write(actual, manila, bridge, false, false, null, extensions, false);
        String expected = IOUtils.toString(getClass().getResourceAsStream("default_context-vms.test.xml"));
        assertEquals(expected, actual.toString());
    }

    @Test
    public void testParkConfig() throws Exception {
        StringWriter actual = new StringWriter();
        Location location = TestHelper.createDefaultLocation();
        IMocksControl mc = EasyMock.createControl();
        FeatureManager mgr = mc.createMock(FeatureManager.class);
        mgr.getLocationsForEnabledFeature(Ivr.FEATURE);
        mc.andReturn(null);
        mc.replay();
        m_configuration.setFeatureManager(mgr);
        Bridge bridge = new Bridge();
        List<FreeswitchExtension> extensions = getExtensions();
        Collection<ParkOrbit> orbits = new LinkedList<ParkOrbit>();
        MockParkOrbit orbit = new MockParkOrbit(true, 120, false, false, null);
        orbit.setEnabled(true);
        orbit.setExtension("12345");
        orbit.setName("Test");
        orbits.add(orbit);
        ParkOrbit orbit1 = new ParkOrbit();
        orbit1.setEnabled(false);
        orbit1.setExtension("6789");
        orbit1.setName("Disabled");
        orbits.add(orbit1);
        MockParkOrbit orbit2 = new MockParkOrbit(true, 120, true, true, "0");
        orbit2.setEnabled(true);
        orbit2.setExtension("1111");
        orbit2.setName("Transfer");
        orbits.add(orbit2);
        MockParkOrbit orbit3 = new MockParkOrbit(false, 120, true, false, "0");
        orbit3.setEnabled(true);
        orbit3.setExtension("2222");
        orbit3.setName("Bare");
        orbits.add(orbit3);
        MockParkOrbit orbit4 = new MockParkOrbit(true, 10, true, true, "5");
        orbit4.setEnabled(true);
        orbit4.setExtension("5555");
        orbit4.setName("Full");
        orbit4.setMusic("custom.wav");
        orbits.add(orbit4);
        m_configuration.write(actual, location, bridge, false, true, orbits, extensions, true);
        String expected = IOUtils.toString(getClass().getResourceAsStream("default_context_orbits.test.xml"));
        assertEquals(expected, actual.toString());
    }

    private static class MockParkOrbit extends ParkOrbit {
        private final boolean m_timeoutEnabled;
        private final int m_timeout;
        private final boolean m_multiple;
        private final boolean m_transferAllowed;
        private final String m_transferKey;
        public MockParkOrbit(boolean timeoutEnabled, int timeout, boolean multiple, boolean transferAllowed,
                String transferKey) {
            m_timeoutEnabled = timeoutEnabled;
            m_timeout = timeout;
            m_multiple = multiple;
            m_transferAllowed = transferAllowed;
            m_transferKey = transferKey;
        }

        @Override
        public boolean isParkTimeoutEnabled() {
            return m_timeoutEnabled;
        }

        @Override
        public int getParkTimeout() {
            if (m_timeoutEnabled) {
                return m_timeout;
            }
            return 86400;
        }

        @Override
        public boolean isMultipleCalls() {
            return m_multiple;
        }

        @Override
        public boolean isTransferAllowed() {
            return m_transferAllowed;
        }

        @Override
        public String getTransferKey() {
            return m_transferKey;
        }

        @Override
        public String getAudioDirectory() {
            return "/var/sipxdata/parkserver/music/";
        }

        @Override
        public String getUnparkExtension() {
            return "\\*4" + getExtension();
        }

        @Override
        public String getCallPickupExtension() {
            return "\\*78" + getExtension();
        }
    }
}
