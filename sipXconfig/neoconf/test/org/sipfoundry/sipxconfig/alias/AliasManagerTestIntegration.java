/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.alias;

import java.util.Collection;

import org.sipfoundry.sipxconfig.common.BeanId;
import org.sipfoundry.sipxconfig.common.CoreContext;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.dialplan.AttendantRule;
import org.sipfoundry.sipxconfig.test.IntegrationTestCase;

public class AliasManagerTestIntegration extends IntegrationTestCase {
    private AliasManagerImpl m_aliasManager;
    private CoreContext m_coreContext;

    protected void onSetUpBeforeTransaction() throws Exception {
        super.onSetUpBeforeTransaction();
        clear();
        sql("commserver/SeedLocations.sql");
    }

    // There are four AliasOwners (excluding AliasManagerImpl itself): CallGroupContextImpl,
    // ConferenceBridgeContextImpl, CoreContextImpl, and DialPlanContextImpl.
    public void testGetAliasOwners() {
        Collection aliasOwners = m_aliasManager.getAliasOwners();
        assertTrue(aliasOwners.size() >= 4); // allow for more AliasOwners to be added
    }

    // See CoreContextImplTestIntegration.testIsAliasInUse
    public void testIsUserAliasInUse() throws Exception {
        sql("common/SampleUsersSeed.sql");
        assertTrue(m_aliasManager.isAliasInUse("janus")); // a user ID
        assertTrue(m_aliasManager.isAliasInUse("dweezil")); // a user alias
        assertFalse(m_aliasManager.isAliasInUse("jessica")); // a first name
    }

    // See DialPlanContextTestIntegration,DialPlanContextTestIntegration
    public void testIsAutoAttendantAliasInUse() throws Exception {
        loadDataSetXml("dialplan/seedDialPlanWithAttendant.xml");
        assertTrue(m_aliasManager.isAliasInUse("100")); // voicemail extension
        assertFalse(m_aliasManager.isAliasInUse("200")); // a random extension that should not be
                                                         // in use
    }

    public void testGetBeanIdsOfObjectsWithAlias() throws Exception {
        loadDataSet("alias/AliasSeed.xml");
        Collection objs = m_aliasManager.getBeanIdsOfObjectsWithAlias("morcheeba");
        assertEquals(1, objs.size());
        BeanId bid = (BeanId) objs.iterator().next();
        assertEquals(AttendantRule.class, bid.getBeanClass());
    }

    public void testCanObjectUseAlias() throws Exception {
        loadDataSet("alias/AliasSeed.xml");

        // Test a new bean with no alias conflicts
        User user = m_coreContext.newUser();
        String alias = "harriet";
        user.setUserName(alias);
        assertTrue(m_aliasManager.canObjectUseAlias(user, alias));

        // Test a saved bean that owns the alias
        m_coreContext.saveUser(user);
        assertTrue(m_aliasManager.canObjectUseAlias(user, alias));

        // Test a new bean trying to use an alias that has already been claimed
        user = m_coreContext.newUser();
        ;
        alias = "alpha";
        user.setUserName(alias);
        assertFalse(m_aliasManager.canObjectUseAlias(user, alias));

        // Test a new bean trying to use an alias that has already been claimed twice
        alias = "martha";
        user.setUserName(alias);
        assertFalse(m_aliasManager.canObjectUseAlias(user, alias));

        // Test a saved bean trying to use an alias that has already been claimed
        user = m_coreContext.loadUser(new Integer(1001));
        alias = "morcheeba";
        user.addAlias(alias);
        assertFalse(m_aliasManager.canObjectUseAlias(user, alias));

        // Test a saved bean trying to use an alias that has already been claimed twice
        user = m_coreContext.loadUser(new Integer(1001));
        alias = "martha";
        user.setUserName(alias);
        assertFalse(m_aliasManager.canObjectUseAlias(user, alias));
    }

    /**
     * DISABLED : You'd have to enable directed call pick-up that uses *78 for these assertions to
     * be true --Douglas
     */
    public void DISABLED_testFeatureCodes() {
        String pickUp = "*78";
        String retrieve = "*4";
        User user = m_coreContext.newUser();
        user.setUserName(pickUp);
        assertFalse(m_aliasManager.canObjectUseAlias(user, pickUp));
        user.setUserName(retrieve);
        assertFalse(m_aliasManager.canObjectUseAlias(user, retrieve));

        // Test a saved bean that owns the alias
        user.setUserName("user");
        m_coreContext.saveUser(user);
        assertFalse(m_aliasManager.canObjectUseAlias(user, pickUp));
        assertFalse(m_aliasManager.canObjectUseAlias(user, retrieve));
    }

    public void setAliasManagerImpl(AliasManagerImpl aliasManager) {
        m_aliasManager = aliasManager;
    }

    public void setCoreContext(CoreContext coreContext) {
        m_coreContext = coreContext;
    }
}
