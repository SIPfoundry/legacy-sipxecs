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
