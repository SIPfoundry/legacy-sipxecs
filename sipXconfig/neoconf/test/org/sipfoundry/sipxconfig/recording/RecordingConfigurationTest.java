/*
 * Copyright (C) 2011 eZuce Inc., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the AGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.recording;

import static org.easymock.EasyMock.createMock;
import static org.easymock.EasyMock.expectLastCall;
import static org.easymock.EasyMock.replay;
import static org.junit.Assert.assertEquals;

import java.io.StringWriter;

import org.apache.commons.io.IOUtils;
import org.junit.Test;
import org.sipfoundry.sipxconfig.domain.DomainManager;
import org.sipfoundry.sipxconfig.test.TestHelper;


public class RecordingConfigurationTest {
    
    @Test
    public void testConfig() throws Exception {
        RecordingConfiguration config = new RecordingConfiguration();
        RecordingSettings settings = new RecordingSettings();
        DomainManager domainManager = createMock(DomainManager.class);
        domainManager.getDomainName();
        expectLastCall().andReturn("example.org").anyTimes();
        domainManager.getAuthorizationRealm();
        expectLastCall().andReturn("grapefruit").anyTimes();
        replay(domainManager);
        settings.setModelFilesContext(TestHelper.getModelFilesContext());
        settings.setDomainManager(domainManager);
        StringWriter actual = new StringWriter();
        config.write(actual, settings);
        String expected = IOUtils.toString(getClass().getResourceAsStream("expected-sipxrecording.properties"));
        assertEquals(expected, actual.toString());
    }
}
