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

import static org.sipfoundry.sipxconfig.admin.commserver.imdb.MongoTestCaseHelper.assertObjectWithIdPresent;

import java.util.ArrayList;
import java.util.Collection;

import org.sipfoundry.sipxconfig.common.User;

public class AliasesTestIntegration extends ImdbTestCase {
    private Aliases m_aliasDataSet;
    private User m_user;

    @Override
    protected void onSetUpBeforeTransaction() {
        Collection<AliasMapping> aliases = new ArrayList<AliasMapping>();
        aliases.add(new AliasMapping("301@example.org", "\"John Doe\"<sip:john.doe@" + DOMAIN + ">", "alias"));
        m_user = new User();
        m_user.setUniqueId(1);
        m_user.setDomainManager(getDomainManager());
        m_user.setPermissionManager(getPermissionManager());
    }

    public void testGenerate() {
        m_aliasDataSet.generate(m_user, m_aliasDataSet.findOrCreate(m_user));
        assertObjectWithIdPresent(getEntityCollection(), "User1");
    }

    public void setAliasDataSet(Aliases aliasDataSet) {
        m_aliasDataSet = aliasDataSet;
    }
}
