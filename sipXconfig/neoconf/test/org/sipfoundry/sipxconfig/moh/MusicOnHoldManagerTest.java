/*
 *
 *
 * Copyright (C) 2009 Nortel., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 *
 */
package org.sipfoundry.sipxconfig.moh;

import junit.framework.TestCase;

import org.apache.commons.collections.CollectionUtils;
import org.sipfoundry.sipxconfig.commserver.imdb.AliasMapping;

public class MusicOnHoldManagerTest extends TestCase {

    private MusicOnHoldManagerImpl m_musicOnHoldManager;

    @Override
    protected void setUp() throws Exception {
        m_musicOnHoldManager = new MusicOnHoldManagerImpl();
        m_musicOnHoldManager.setMohUser("~~testMohUser~");
    }

    public void testGetBeanIdsOfObjectsWithAlias() {
        assertTrue(!CollectionUtils.isEmpty(m_musicOnHoldManager.getBeanIdsOfObjectsWithAlias("~~testMohUser~")));
        assertTrue(!CollectionUtils.isEmpty(m_musicOnHoldManager.getBeanIdsOfObjectsWithAlias("~~testMohUser~asdf")));
        assertTrue(CollectionUtils.isEmpty(m_musicOnHoldManager.getBeanIdsOfObjectsWithAlias("~~testUser~")));
    }

    public void testIsAliasInUse() {
        assertTrue(m_musicOnHoldManager.isAliasInUse("~~testMohUser~"));
        assertTrue(m_musicOnHoldManager.isAliasInUse("~~testMohUser~asdf"));
        assertTrue(!m_musicOnHoldManager.isAliasInUse("~~testUser~"));
    }

    public boolean checkAliasMappings(AliasMapping alias, String contactToTest) {
        if ("sip:~~testMohUser~@randomAddress.test".equals(alias.getIdentity())) {
            assertEquals(contactToTest, alias.getContact());
        } else if ("sip:~~testMohUser~l@randomAddress.test".equals(alias.getIdentity())) {
            assertEquals("<sip:IVR@FSaddr:42;action=moh;moh=l>", alias.getContact());
        } else if ("sip:~~testMohUser~p@randomAddress.test".equals(alias.getIdentity())) {
            assertEquals("<sip:IVR@FSaddr:42;action=moh;moh=p>", alias.getContact());
        } else if ("sip:~~testMohUser~n@randomAddress.test".equals(alias.getIdentity())) {
            assertEquals("<sip:IVR@FSaddr:42;action=moh;moh=n>", alias.getContact());
        } else {
            return false;
        }

        return true;
    }

    public boolean checkAliasMappingsHA(AliasMapping alias, String contactToTest) {
        if ("sip:~~testMohUser~@randomAddress.test".equals(alias.getIdentity())) {
            assertEquals(contactToTest, alias.getContact());
        } else if ("sip:~~testMohUser~l@randomAddress.test".equals(alias.getIdentity())) {
            assertEquals("<sip:IVR@192.168.1.1:42;action=moh;moh=l>", alias.getContact());
        } else if ("sip:~~testMohUser~p@randomAddress.test".equals(alias.getIdentity())) {
            assertEquals("<sip:IVR@192.168.1.1:42;action=moh;moh=p>", alias.getContact());
        } else if ("sip:~~testMohUser~n@randomAddress.test".equals(alias.getIdentity())) {
            assertEquals("<sip:IVR@192.168.1.1:42;action=moh;moh=n>", alias.getContact());
        } else {
            return false;
        }

        return true;
    }
}
