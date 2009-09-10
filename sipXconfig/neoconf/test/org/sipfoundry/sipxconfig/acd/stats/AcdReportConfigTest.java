/*
 *
 *
 * Copyright (C) 2009 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 *
 */
package org.sipfoundry.sipxconfig.acd.stats;

import java.io.InputStream;
import java.io.InputStreamReader;
import java.io.Reader;
import java.io.StringReader;
import java.io.StringWriter;
import java.util.ArrayList;
import java.util.List;
import java.util.Properties;

import org.apache.commons.io.IOUtils;
import org.apache.velocity.app.VelocityEngine;
import org.custommonkey.xmlunit.XMLTestCase;
import org.easymock.EasyMock;
import org.easymock.IMocksControl;
import org.sipfoundry.sipxconfig.TestHelper;
import org.sipfoundry.sipxconfig.acd.AcdContext;
import org.sipfoundry.sipxconfig.acd.AcdServer;
import org.sipfoundry.sipxconfig.admin.commserver.Location;

public class AcdReportConfigTest extends XMLTestCase {
    @Override
    protected void setUp() throws Exception {

    }

    public void testReplicateReportConfig() throws Exception {
        AcdHistoricalConfigurationFile acdHistoricalConf = new AcdHistoricalConfigurationFile();

        // creates velocity engine for Report component
        Properties sysdir = TestHelper.getSysDirProperties();
        String etcDir = sysdir.getProperty("sysdir.etc");
        VelocityEngine engine = new VelocityEngine();
        engine.setProperty("resource.loader", "file");
        engine.setProperty("file.resource.loader.path", etcDir + "/../../report/etc");
        engine.init();

        acdHistoricalConf.setVelocityEngine(engine);
        acdHistoricalConf.setTemplate("sipxconfig-report-config.vm");
        acdHistoricalConf.setDbUser("postgres");

        IMocksControl mc = EasyMock.createControl();
        AcdContext acdContext = mc.createMock(AcdContext.class);
        acdContext.getServers();
        Location location = new Location();
        location.setFqdn("example.org");
        AcdServer acdServer = new AcdServer();
        acdServer.setLocation(location);
        List<AcdServer> acdServers = new ArrayList<AcdServer>();
        acdServers.add(acdServer);
        mc.andReturn(acdServers);
        mc.replay();

        acdHistoricalConf.setAcdContext(acdContext);
        acdHistoricalConf.setAgentPort(8120);

        InputStream resourceAsStream = AcdReportConfigTest.class.getResourceAsStream("report-config-expected.xml");
        assertNotNull(resourceAsStream);

        Reader referenceConfigReader = new InputStreamReader(resourceAsStream);
        String referenceConfig = IOUtils.toString(referenceConfigReader);

        StringWriter actualConfigWriter = new StringWriter();
        acdHistoricalConf.write(actualConfigWriter, location);

        Reader actualConfigReader = new StringReader(actualConfigWriter.toString());
        String actualConfig = IOUtils.toString(actualConfigReader);

        assertEquals(referenceConfig, actualConfig);

    }
}
