/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.admin.commserver.imdb;

import java.util.List;
import java.util.Map;

import org.sipfoundry.sipxconfig.IntegrationTestCase;
import org.sipfoundry.sipxconfig.common.Md5Encoder;

public class CredentialsTestIntegration extends IntegrationTestCase {

    private Credentials m_credentials;

    public void testGenerateEmpty() throws Exception {
        loadDataSetXml("domain/DomainSeed.xml");
        List<Map<String, String>> callerAliases = ((DataSetGenerator) m_credentials).generate();
        assertEquals(0, callerAliases.size());
    }

    public void testAddCallgroup() throws Exception {
        loadDataSetXml("domain/DomainSeed.xml");
        loadDataSetXml("admin/commserver/imdb/CredentialsCallGroupSeed.xml");
        List<Map<String, String>> callerAliases = ((DataSetGenerator) m_credentials).generate();
        Md5Encoder.digestPassword("sales", "example.org", "pass4321");
        String digest = "282e44b75e1e04d379d3157c34e31814";
        Map<String, String> item = callerAliases.get(0);
        assertEquals("sip:sales@example.org", item.get("uri"));
        assertEquals(digest, item.get("pintoken"));
        assertEquals("pass43213", item.get("passtoken"));
        assertEquals("sipfoundry.org", item.get("realm"));
        assertEquals("DIGEST", item.get("authtype"));
    }

    public void testAddUser() throws Exception {
        loadDataSetXml("domain/DomainSeed.xml");
        loadDataSet("common/UserSearchSeed.xml");
        List<Map<String, String>> callerAliases = ((DataSetGenerator) m_credentials).generate();
        assertEquals(11, callerAliases.size());
        Map<String, String> item = callerAliases.get(0);

        assertEquals("sip:userseed1@example.org", item.get("uri"));
        assertEquals("x", item.get("pintoken"));
        assertEquals("", item.get("passtoken"));
        assertEquals("sipfoundry.org", item.get("realm"));
        assertEquals("DIGEST", item.get("authtype"));
    }

    public void setCredentialDataSet(Credentials credentials) {
        m_credentials = credentials;
    }

}
