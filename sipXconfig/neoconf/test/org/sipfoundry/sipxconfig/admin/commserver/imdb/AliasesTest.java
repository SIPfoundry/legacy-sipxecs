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

import org.sipfoundry.sipxconfig.TestHelper;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.domain.DomainManager;
import org.sipfoundry.sipxconfig.permission.PermissionManagerImpl;

public class AliasesTest extends MongoTestCase {
    private Aliases m_aliases;
    private User m_user;

    protected void setUp() throws Exception {
        super.setUp();
        m_aliases = new Aliases();

        Collection<AliasMapping> aliases = new ArrayList<AliasMapping>();
        aliases.add(new AliasMapping("301@example.org", "\"John Doe\"<sip:john.doe@" + DOMAIN + ">", "alias"));

        DomainManager dm = createMock(DomainManager.class);
        dm.getDomainName();
        expectLastCall().andReturn(DOMAIN).anyTimes();
        replay(dm);

        PermissionManagerImpl pm = new PermissionManagerImpl();
        pm.setModelFilesContext(TestHelper.getModelFilesContext());

        m_user = new User();
        m_user.setUniqueId(1);
        m_user.setDomainManager(dm);
        m_user.setPermissionManager(pm);

        replay(getCoreContext());
        m_aliases.setDbCollection(getCollection());
        m_aliases.setCoreContext(getCoreContext());
    }

    public void testGenerate() {
        m_aliases.generate(m_user);

        MongoTestCaseHelper.assertObjectWithIdPresent("User1");
    }

}
