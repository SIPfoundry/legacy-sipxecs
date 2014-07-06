/**
 * Copyright (c) 2014 eZuce, Inc. All rights reserved.
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
package org.sipfoundry.sipxconfig.api;

import java.util.HashSet;
import java.util.Set;

import org.junit.Test;
import org.sipfoundry.sipxconfig.commserver.Location;
import org.sipfoundry.sipxconfig.commserver.LocationsManager;
import org.sipfoundry.sipxconfig.feature.FeatureManager;
import org.sipfoundry.sipxconfig.feature.LocationFeature;
import org.sipfoundry.sipxconfig.freeswitch.FreeswitchFeature;
import org.sipfoundry.sipxconfig.parkorbit.ParkOrbit;
import org.sipfoundry.sipxconfig.parkorbit.ParkOrbitContext;
import org.sipfoundry.sipxconfig.proxy.ProxyManager;
import org.sipfoundry.sipxconfig.registrar.Registrar;
import org.sipfoundry.sipxconfig.test.RestApiIntegrationTestCase;

public class CallParkApiTestIntegration extends RestApiIntegrationTestCase {
    private ParkOrbitContext m_parkContext;
    private LocationsManager m_locationsManager;
    private FeatureManager m_featureManger;

    @Override
    protected void onSetUpBeforeTransaction() throws Exception {
        super.onSetUpBeforeTransaction();
        clear();
        sql("commserver/SeedLocations.sql");
        Location primary = m_locationsManager.getPrimaryLocation();
        Location secondary = m_locationsManager.getLocationByFqdn("remote.example.org");
        Set<LocationFeature> features = new HashSet<LocationFeature>();
        features.add(ProxyManager.FEATURE);
        features.add(Registrar.FEATURE);
        features.add(FreeswitchFeature.FEATURE);
        features.add(ParkOrbitContext.FEATURE);
        m_featureManger.enableLocationFeatures(features, primary, true);
        m_featureManger.enableLocationFeatures(features, secondary, true);
        m_parkContext.clear();
    }

    @Test
    public void testCallParkJsonApi() throws Exception {
        // query empty phones
        String emptyOrbits = getAsJson("orbits");
        assertEquals("{\"orbits\":[]}", emptyOrbits);

        // create orbit
        String createOrbit = "{\"name\":\"primaryOrbit\",\"enabled\":true,\"extension\":\"1234\","
                + "\"description\":\"custom orbit\",\"music\":\"default.wav\",\"server\":\"primary.example.org\"}";
        int code = postJsonString(createOrbit, "orbits");
        assertEquals(200, code);
        assertEquals(1, m_parkContext.getParkOrbits().size());

        ParkOrbit orbit = m_parkContext.loadParkOrbitByName("primaryOrbit");

        // retrieve orbits
        String orbits = getAsJson("orbits");
        assertEquals(
                String.format(
                        "{\"orbits\":[{\"id\":%s,\"name\":\"primaryOrbit\",\"enabled\":true,\"extension\":\"1234\","
                                + "\"description\":\"custom orbit\",\"music\":\"default.wav\",\"server\":\"primary.example.org\"}]}",
                        orbit.getId()), orbits);

        // retrieve orbits by server
        orbits = getAsJson("orbits/servers/primary.example.org");
        assertEquals(
                String.format(
                        "{\"orbits\":[{\"id\":%s,\"name\":\"primaryOrbit\",\"enabled\":true,\"extension\":\"1234\","
                                + "\"description\":\"custom orbit\",\"music\":\"default.wav\",\"server\":\"primary.example.org\"}]}",
                        orbit.getId()), orbits);

        // retrieve orbits by server
        orbits = getAsJson("orbits/servers/remote.example.org");
        assertEquals("{\"orbits\":[]}", orbits);

        String createOrbitByServer = "{\"name\":\"remoteOrbit\",\"enabled\":true,\"extension\":\"5678\","
                + "\"description\":\"remote orbit\",\"music\":\"default.wav\"}";
        code = postJsonString(createOrbitByServer, "orbits/servers/remote.example.org");
        assertEquals(200, code);

        orbits = getAsJson("orbits/servers/remote.example.org");
        ParkOrbit remoteOrbit = m_parkContext.loadParkOrbitByName("remoteOrbit");
        assertEquals(
                String.format(
                        "{\"orbits\":[{\"id\":%s,\"name\":\"remoteOrbit\",\"enabled\":true,\"extension\":\"5678\",\"description\":\"remote orbit\","
                                + "\"music\":\"default.wav\",\"server\":\"remote.example.org\"}]}",
                        remoteOrbit.getId()), orbits);

        // retrieve orbit
        String orbitJson = getAsJson("orbits/" + remoteOrbit.getId());
        assertEquals(String.format("{\"id\":%s,\"name\":\"remoteOrbit\",\"enabled\":true,\"extension\":\"5678\","
                + "\"description\":\"remote orbit\",\"music\":\"default.wav\",\"server\":\"remote.example.org\"}",
                remoteOrbit.getId()), orbitJson);

        // modify orbit description and server
        String modifyOrbit = "{\"name\":\"remoteOrbit\",\"enabled\":false,\"extension\":\"5678\","
                + "\"description\":\"remote orbit changed\",\"music\":\"default.wav\",\"server\":\"primary.example.org\"}";
        int putCode = putJsonString(modifyOrbit, "orbits/" + remoteOrbit.getId());
        assertEquals(200, putCode);

        orbits = getAsJson("orbits/servers/primary.example.org");
        assertEquals(
                String.format(
                        "{\"orbits\":[{\"id\":%s,\"name\":\"primaryOrbit\",\"enabled\":true,\"extension\":\"1234\","
                                + "\"description\":\"custom orbit\",\"music\":\"default.wav\",\"server\":\"primary.example.org\"},"
                                + "{\"id\":%s,\"name\":\"remoteOrbit\",\"enabled\":false,\"extension\":\"5678\","
                                + "\"description\":\"remote orbit changed\",\"music\":\"default.wav\",\"server\":\"primary.example.org\"}]}",
                        orbit.getId(), remoteOrbit.getId()), orbits);

        // get setting
        String setting = getAsJson("orbits/" + remoteOrbit.getId() + "/settings/general/parkTimeout");
        assertEquals(
                "{\"path\":\"general/parkTimeout\",\"type\":\"integer\",\"options\":null,\"value\":\"120\",\"defaultValue\":\"120\","
                        + "\"label\":\"Park time-out\",\"description\":\"Time period in seconds, after which the call is automatically transferred back to the extension that parked the call if time-out is enabled.\"}",
                setting);

        // modify setting
        int settingCode = putPlainText("55", "orbits/" + remoteOrbit.getId() + "/settings/general/parkTimeout");
        assertEquals(200, settingCode);
        String modifiedSetting = getAsJson("orbits/" + remoteOrbit.getId() + "/settings/general/parkTimeout");
        assertEquals(
                "{\"path\":\"general/parkTimeout\",\"type\":\"integer\",\"options\":null,\"value\":\"55\",\"defaultValue\":\"120\","
                        + "\"label\":\"Park time-out\",\"description\":\"Time period in seconds, after which the call is automatically transferred back to the extension that parked the call if time-out is enabled.\"}",
                modifiedSetting);

        // reset setting
        int resetCode = delete("orbits/" + remoteOrbit.getId() + "/settings/general/parkTimeout");
        assertEquals(200, resetCode);
        setting = getAsJson("orbits/" + remoteOrbit.getId() + "/settings/general/parkTimeout");
        assertEquals(
                "{\"path\":\"general/parkTimeout\",\"type\":\"integer\",\"options\":null,\"value\":\"120\",\"defaultValue\":\"120\","
                        + "\"label\":\"Park time-out\",\"description\":\"Time period in seconds, after which the call is automatically transferred back to the extension that parked the call if time-out is enabled.\"}",
                setting);

        // delete orbit
        int deleteOrbit = delete("orbits/" + remoteOrbit.getId());
        assertEquals(200, deleteOrbit);
        assertEquals(1, m_parkContext.getParkOrbits().size());

        // delete orbits by server
        int deleteOrbits = delete("orbits/servers/primary.example.org");
        assertEquals(200, deleteOrbits);
        assertEquals(0, m_parkContext.getParkOrbits().size());
    }

    @Test
    public void testCallParkXmlApi() throws Exception {
        // query empty phones
        String emptyOrbits = getAsXml("orbits");
        assertEquals("<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?><Orbits/>", emptyOrbits);

        // create orbit
        String createOrbit = "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?><Orbit><name>primaryOrbit</name><enabled>true</enabled>"
                + "<extension>1234</extension><description>custom orbit</description><music>default.wav</music><server>primary.example.org</server></Orbit>";
        int code = postXmlString(createOrbit, "orbits");
        assertEquals(200, code);
        assertEquals(1, m_parkContext.getParkOrbits().size());

        ParkOrbit orbit = m_parkContext.loadParkOrbitByName("primaryOrbit");

        // retrieve orbits
        String orbits = getAsXml("orbits");
        assertEquals(
                String.format(
                        "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?><Orbits><Orbit><id>%s</id><name>primaryOrbit</name>"
                                + "<enabled>true</enabled><extension>1234</extension><description>custom orbit</description><music>default.wav</music>"
                                + "<server>primary.example.org</server></Orbit></Orbits>", orbit.getId()), orbits);

        // retrieve orbits by server
        orbits = getAsXml("orbits/servers/primary.example.org");
        assertEquals(
                String.format(
                        "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?><Orbits><Orbit><id>%s</id><name>primaryOrbit</name>"
                                + "<enabled>true</enabled><extension>1234</extension><description>custom orbit</description><music>default.wav</music>"
                                + "<server>primary.example.org</server></Orbit></Orbits>", orbit.getId()), orbits);

        // retrieve orbits by server
        orbits = getAsXml("orbits/servers/remote.example.org");
        assertEquals("<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?><Orbits/>", orbits);

        String createOrbitByServer = "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?><Orbit><name>remoteOrbit</name><enabled>true</enabled>"
                + "<extension>5678</extension><description>remote orbit</description><music>default.wav</music></Orbit>";
        code = postXmlString(createOrbitByServer, "orbits/servers/remote.example.org");
        assertEquals(200, code);

        orbits = getAsXml("orbits/servers/remote.example.org");
        ParkOrbit remoteOrbit = m_parkContext.loadParkOrbitByName("remoteOrbit");
        assertEquals(
                String.format(
                        "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?><Orbits><Orbit><id>%s</id><name>remoteOrbit</name>"
                                + "<enabled>true</enabled><extension>5678</extension><description>remote orbit</description><music>default.wav</music>"
                                + "<server>remote.example.org</server></Orbit></Orbits>", remoteOrbit.getId()),
                orbits);

        // retrieve orbit
        String orbitJson = getAsXml("orbits/" + remoteOrbit.getId());
        assertEquals(
                String.format(
                        "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?><Orbit><id>%s</id><name>remoteOrbit</name>"
                                + "<enabled>true</enabled><extension>5678</extension><description>remote orbit</description><music>default.wav</music>"
                                + "<server>remote.example.org</server></Orbit>", remoteOrbit.getId()), orbitJson);

        // modify orbit description and server
        String modifyOrbit = "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?><Orbit><name>remoteOrbit</name><enabled>false</enabled>"
                + "<extension>5678</extension><description>remote orbit changed</description><music>default.wav</music><server>primary.example.org</server></Orbit>";
        int putCode = putXmlString(modifyOrbit, "orbits/" + remoteOrbit.getId());
        assertEquals(200, putCode);

        orbits = getAsXml("orbits/servers/primary.example.org");
        assertEquals(
                String.format(
                        "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?><Orbits><Orbit><id>%s</id><name>primaryOrbit</name>"
                                + "<enabled>true</enabled><extension>1234</extension><description>custom orbit</description><music>default.wav</music>"
                                + "<server>primary.example.org</server></Orbit>"
                                + "<Orbit><id>%s</id><name>remoteOrbit</name><enabled>false</enabled><extension>5678</extension>"
                                + "<description>remote orbit changed</description><music>default.wav</music><server>primary.example.org</server></Orbit></Orbits>",
                        orbit.getId(), remoteOrbit.getId()), orbits);

        // get setting
        String setting = getAsXml("orbits/" + remoteOrbit.getId() + "/settings/general/parkTimeout");
        assertEquals(
                "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?><Setting><path>general/parkTimeout</path>"
                        + "<type>integer</type><value>120</value><defaultValue>120</defaultValue><label>Park time-out</label>"
                        + "<description>Time period in seconds, after which the call is automatically transferred back to the extension that parked the call if time-out is enabled.</description></Setting>",
                setting);

        // modify setting
        int settingCode = putPlainText("55", "orbits/" + remoteOrbit.getId() + "/settings/general/parkTimeout");
        assertEquals(200, settingCode);
        String modifiedSetting = getAsXml("orbits/" + remoteOrbit.getId() + "/settings/general/parkTimeout");
        assertEquals(
                "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?><Setting><path>general/parkTimeout</path>"
                        + "<type>integer</type><value>55</value><defaultValue>120</defaultValue><label>Park time-out</label>"
                        + "<description>Time period in seconds, after which the call is automatically transferred back to the extension that parked the call if time-out is enabled.</description></Setting>",
                modifiedSetting);

        // reset setting
        int resetCode = delete("orbits/" + remoteOrbit.getId() + "/settings/general/parkTimeout");
        assertEquals(200, resetCode);
        setting = getAsXml("orbits/" + remoteOrbit.getId() + "/settings/general/parkTimeout");
        assertEquals(
                "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?><Setting><path>general/parkTimeout</path>"
                        + "<type>integer</type><value>120</value><defaultValue>120</defaultValue><label>Park time-out</label>"
                        + "<description>Time period in seconds, after which the call is automatically transferred back to the extension that parked the call if time-out is enabled.</description></Setting>",
                setting);

        // delete orbit
        int deleteOrbit = delete("orbits/" + remoteOrbit.getId());
        assertEquals(200, deleteOrbit);
        assertEquals(1, m_parkContext.getParkOrbits().size());

        // delete orbits by server
        int deleteOrbits = delete("orbits/servers/primary.example.org");
        assertEquals(200, deleteOrbits);
        assertEquals(0, m_parkContext.getParkOrbits().size());
    }

    public void setFeatureManager(FeatureManager manager) {
        m_featureManger = manager;
    }

    public void setParkOrbitContext(ParkOrbitContext context) {
        m_parkContext = context;
    }

    public void setLocationsManager(LocationsManager manager) {
        m_locationsManager = manager;
    }

}
