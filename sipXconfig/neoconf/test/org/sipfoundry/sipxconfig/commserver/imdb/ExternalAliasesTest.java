/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.commserver.imdb;

import java.net.URL;
import java.util.List;

import junit.framework.TestCase;

public class ExternalAliasesTest extends TestCase {

    public void testGetAliasMappings() {
        ExternalAliases externalAliases = new ExternalAliases();
        assertTrue(externalAliases.getAliasMappings().isEmpty());
    }

    public void testGetAliasMappingsBadProperty() {
        // warning log messages are expected
        ExternalAliases externalAliases = new ExternalAliases();
        externalAliases.setAliasAddins("ds dkkk, sd");
        assertTrue(externalAliases.getAliasMappings().isEmpty());
        externalAliases.setAddinsDirectory("/ff/ff");
        assertTrue(externalAliases.getAliasMappings().isEmpty());

        // bad file
        URL resource = getClass().getResource("UserGroupSeed.db.xml");
        externalAliases.setAliasAddins(resource.getFile());
        assertTrue(externalAliases.getAliasMappings().isEmpty());
    }

    // BROKEN. alias object do not have id attribute set. -- Douglas
    public void testRealAliases() {
        ExternalAliases externalAliases = new ExternalAliases();
        URL other = getClass().getResource("alias.test.xml");
        externalAliases.setAliasAddins(other.getFile());
        List<AliasMapping> aliases = (List<AliasMapping>) externalAliases.getAliasMappings();
        assertEquals(5, aliases.size());
        for (AliasMapping alias : aliases) {
            assertTrue(alias.getIdentity().startsWith("30"));
            assertTrue(alias.getContact().indexOf("example.org") > 0);
        }
    }

    public void testRealAliasesTwice() {
        ExternalAliases externalAliases = new ExternalAliases();
        URL aliases1 = getClass().getResource("alias.test.xml");
        externalAliases.setAliasAddins(aliases1.getFile());
        List<AliasMapping> aliasMappings = (List<AliasMapping>) externalAliases.getAliasMappings();
        assertEquals(5, aliasMappings.size());
        URL aliases2 = getClass().getResource("alias2.test.xml");
        externalAliases.setAliasAddins(aliases2.getFile());
        aliasMappings = (List<AliasMapping>) externalAliases.getAliasMappings();
        assertEquals(2, aliasMappings.size());
        AliasMapping alias = (AliasMapping) aliasMappings.get(1);
        assertTrue(alias.getIdentity().startsWith("extra"));
    }
}
