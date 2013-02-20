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

import static org.sipfoundry.sipxconfig.commserver.imdb.MongoTestCaseHelper.assertObjectWithIdPresent;

import java.util.ArrayList;
import java.util.Collection;

import org.sipfoundry.sipxconfig.common.User;

public class AliasesTestIntegration extends ImdbTestCase {
    private User m_user;

    @Override
    protected void onSetUpBeforeTransaction() {
        Collection<AliasMapping> aliases = new ArrayList<AliasMapping>();
        aliases.add(new AliasMapping("301@example.org", "\"John Doe\"<sip:john.doe@" + DOMAIN + ">", "alias"));
        m_user = getCoreContext().newUser();
        m_user.setUniqueId(1);
    }

    public void testGenerate() throws Exception {
        loadDataSetXml("commserver/seedLocations.xml");
        getReplicationManager().replicateEntity(m_user, DataSet.ALIAS);
        assertObjectWithIdPresent(getEntityCollection(), "User1");
    }

}
