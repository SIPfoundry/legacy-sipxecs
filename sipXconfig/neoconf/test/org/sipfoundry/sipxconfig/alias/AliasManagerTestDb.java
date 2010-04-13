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

import org.sipfoundry.sipxconfig.SipxDatabaseTestCase;
import org.sipfoundry.sipxconfig.TestHelper;
import org.sipfoundry.sipxconfig.admin.dialplan.AttendantRule;
import org.sipfoundry.sipxconfig.common.BeanId;
import org.sipfoundry.sipxconfig.common.CoreContext;
import org.sipfoundry.sipxconfig.common.User;
import org.springframework.context.ApplicationContext;

public class AliasManagerTestDb extends SipxDatabaseTestCase {
    private AliasManagerImpl m_aliasManager;
    private CoreContext m_coreContext;

    protected void setUp() throws Exception {
        ApplicationContext app = TestHelper.getApplicationContext();
        m_aliasManager = (AliasManagerImpl) app.getBean(AliasManagerImpl.CONTEXT_BEAN_NAME);
        m_coreContext = (CoreContext) app.getBean(CoreContext.CONTEXT_BEAN_NAME);
        TestHelper.cleanInsert("ClearDb.xml");
    }

    // There are four AliasOwners (excluding AliasManagerImpl itself): CallGroupContextImpl,
    // ConferenceBridgeContextImpl, CoreContextImpl, and DialPlanContextImpl.
    public void testGetAliasOwners() {
        Collection aliasOwners = m_aliasManager.getAliasOwners();
        assertTrue(aliasOwners.size() >= 4);    // allow for more AliasOwners to be added
    }

    // See CoreContextImplTestDb.testIsAliasInUse
    public void testIsUserAliasInUse() throws Exception {
        TestHelper.cleanInsertFlat("common/SampleUsersSeed.xml");
        assertTrue(m_aliasManager.isAliasInUse("janus"));       // a user ID
        assertTrue(m_aliasManager.isAliasInUse("dweezil"));     // a user alias
        assertFalse(m_aliasManager.isAliasInUse("jessica"));    // a first name
    }

    // See DialPlanContextTestDb,DialPlanContextTestDb
    public void testIsAutoAttendantAliasInUse() throws Exception {
        TestHelper.cleanInsert("admin/dialplan/seedDialPlanWithAttendant.xml");
        assertTrue(m_aliasManager.isAliasInUse("100"));      // voicemail extension
        assertFalse(m_aliasManager.isAliasInUse("200"));     // a random extension that should not be in use
    }

    public void testGetBeanIdsOfObjectsWithAlias() throws Exception  {
        TestHelper.cleanInsertFlat("alias/AliasSeed.xml");
        Collection objs = m_aliasManager.getBeanIdsOfObjectsWithAlias("morcheeba");
        assertEquals(1, objs.size());
        BeanId bid = (BeanId) objs.iterator().next();
        assertEquals(AttendantRule.class, bid.getBeanClass());
    }

    public void testCanObjectUseAlias() throws Exception  {
        TestHelper.cleanInsertFlat("alias/AliasSeed.xml");

        // Test a new bean with no alias conflicts
        User user = new User();
        String alias = "harriet";
        user.setUserName(alias);
        assertTrue(m_aliasManager.canObjectUseAlias(user, alias));

        // Test a saved bean that owns the alias
        m_coreContext.saveUser(user);
        assertTrue(m_aliasManager.canObjectUseAlias(user, alias));

        // Test a new bean trying to use an alias that has already been claimed
        user = new User();
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

}
