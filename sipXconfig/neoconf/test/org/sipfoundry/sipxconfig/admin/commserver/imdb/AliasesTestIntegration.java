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

import java.util.HashSet;
import java.util.List;
import java.util.Set;

import org.sipfoundry.sipxconfig.common.User;

import com.mongodb.DBObject;

public class AliasesTestIntegration extends ImdbTestCase {
    private Aliases m_aliasDataSet;
    private User m_user;
    private ReplicationManagerImpl m_replManager;

    @Override
    protected void onSetUpBeforeTransaction() {
        m_user = new User();
        m_user.setUniqueId(1);
        m_user.setDomainManager(getDomainManager());
        m_user.setPermissionManager(getPermissionManager());
        Set<String> aliases = new HashSet<String>();
        aliases.add("alias1");
        m_user.setAliases(aliases);
        m_user.setUserName("201");
    }

    @Override
    protected void onSetUpInTransaction() throws Exception {
    }

    public void testGenerate() throws Exception {
        loadDataSetXml("admin/commserver/seedLocations.xml");
        DBObject mongoUser = m_replManager.findOrCreate(m_user);
        m_aliasDataSet.generate(m_user, mongoUser);
        List aliases = (List) mongoUser.get("als");
        assertEquals(2, aliases.size());
        DBObject aliasObj = (DBObject) aliases.get(0);
        assertEquals("alias1", aliasObj.get("id"));
        assertEquals("sip:201@example.org", aliasObj.get("cnt"));
        assertEquals("alias", aliasObj.get("rln"));
        aliasObj = (DBObject) aliases.get(1);
        assertEquals("~~vm~201", aliasObj.get("id"));
        assertEquals("<sip:IVR@192.168.0.26:15060;mailbox=201;action=deposit;locale=en>", aliasObj.get("cnt"));
        assertEquals("vmprm", aliasObj.get("rln"));
    }

    public void setAliasDataSet(Aliases aliasDataSet) {
        m_aliasDataSet = aliasDataSet;
    }

    public void setReplicationManagerImpl(ReplicationManagerImpl replManager) {
        m_replManager = replManager;
    }
}
