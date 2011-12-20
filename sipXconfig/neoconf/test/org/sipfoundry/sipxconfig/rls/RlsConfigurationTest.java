/*
 * Copyright (C) 2011 eZuce Inc., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the AGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.rls;

import static org.junit.Assert.assertEquals;

import java.io.StringWriter;

import org.apache.commons.io.IOUtils;
import org.dom4j.Document;
import org.junit.Test;
import org.sipfoundry.sipxconfig.commserver.Location;
import org.sipfoundry.sipxconfig.domain.Domain;
import org.sipfoundry.sipxconfig.test.TestHelper;


public class RlsConfigurationTest {
    @Test
    public void testConfig() throws Exception {
        RlsConfig config = new RlsConfig();
        RlsSettings settings = new RlsSettings();
        settings.setModelFilesContext(TestHelper.getModelFilesContext());
        Domain domain = new Domain("example.org");
        domain.setSipRealm("realm.example.org");
        Location location = TestHelper.createDefaultLocation();
        StringWriter actual = new StringWriter();
        config.write(actual, settings, location, domain);
        String expected = IOUtils.toString(getClass().getResourceAsStream("expected-rls-config"));
        assertEquals(expected, actual.toString());
    }
    
    public void testXml() throws Exception {
        ResourceLists lists = new ResourceLists();
        Document doc = lists.getDocument();
        String actual = TestHelper.asString(doc);
        String expected = IOUtils.toString(getClass().getResourceAsStream(""));
        assertEquals(expected, actual);
        
    }
}
