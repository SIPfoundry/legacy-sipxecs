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

import static org.easymock.EasyMock.expectLastCall;
import static org.easymock.EasyMock.replay;
import static org.easymock.classextension.EasyMock.createMock;

import java.util.ArrayList;
import java.util.Collection;
import java.util.HashMap;
import java.util.Map;

import org.easymock.EasyMock;
import org.sipfoundry.sipxconfig.admin.commserver.AliasProvider;
import org.sipfoundry.sipxconfig.common.Replicable;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.domain.DomainManager;

public class AliasesTest extends MongoTestCase {
    private Map<Replicable, Collection<AliasMapping>> m_aliasMappings = new HashMap<Replicable, Collection<AliasMapping>>();

    private Aliases m_aliases;
    private User m_user;

    protected void setUp() throws Exception {
        super.setUp();
        m_aliases = new Aliases();

        Collection<AliasMapping> aliases = new ArrayList<AliasMapping>();
        aliases.add(new AliasMapping("301@example.org", "\"John Doe\"<sip:john.doe@" + DOMAIN + ">"));
        aliases.add(new AliasMapping("alias1@" + DOMAIN));

        DomainManager dm = createMock(DomainManager.class);
        dm.getDomainName();
        expectLastCall().andReturn(DOMAIN).anyTimes();
        replay(dm);

        m_user = new User();
        m_user.setUniqueId(1);
        m_user.setDomainManager(dm);

        m_aliasMappings.put(m_user, aliases);

        AliasProvider aliasProvider = EasyMock.createMock(AliasProvider.class);
        aliasProvider.getAliasMappings();
        expectLastCall().andReturn(m_aliasMappings).anyTimes();
        replay(aliasProvider, getCoreContext());
        m_aliases.setAliasProvider(aliasProvider);
        m_aliases.setDbCollection(getCollection());
        m_aliases.setCoreContext(getCoreContext());
    }

    public void testGenerate() {
        m_aliases.generate();

        assertObjectWithIdPresent("User1");

    }

}
