/*
 *
 *
 * Copyright (C) 2009 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 *
 */
package org.sipfoundry.sipxconfig.imbot;

import static org.junit.Assert.assertEquals;

import java.io.StringWriter;

import org.apache.commons.io.IOUtils;
import org.junit.Test;
import org.sipfoundry.sipxconfig.address.Address;
import org.sipfoundry.sipxconfig.admin.AdminContext;
import org.sipfoundry.sipxconfig.domain.Domain;
import org.sipfoundry.sipxconfig.ivr.Ivr;
import org.sipfoundry.sipxconfig.restserver.RestServer;
import org.sipfoundry.sipxconfig.test.TestHelper;

public class ImbotConfigurationTest {

    @Test
    public void testConfig() throws Exception {
        ImBotConfiguration config = new ImBotConfiguration();
        StringWriter actual = new StringWriter();
        ImBotSettings settings = new ImBotSettings();
        settings.setModelFilesContext(TestHelper.getModelFilesContext());
        settings.setPersonalAssistantImPassword("iwonttell");
        Domain domain = new Domain("example.org");
        domain.setSipRealm("grapefruit");
        Address ivr = new Address(Ivr.REST_API, "ivr.example.org", 100);
        Address admin = new Address(AdminContext.HTTP_ADDRESS, "admin.example.org", 101);
        Address rest = new Address(RestServer.HTTP_API, "rest.example.org", 102);
        Address imApi = new Address(RestServer.HTTP_API, "imapi.example.org", 103);
        config.write(actual, settings, domain, ivr, admin, rest, imApi);
        String expected = IOUtils.toString(getClass().getResourceAsStream("expected-sipximbot.properties"));
        assertEquals(expected, actual.toString());
    }
}
