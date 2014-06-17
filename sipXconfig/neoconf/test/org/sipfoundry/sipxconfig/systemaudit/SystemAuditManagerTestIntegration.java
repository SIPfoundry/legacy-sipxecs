/**
 *
 *
 * Copyright (c) 2014 Karel, Inc. All rights reserved.
 * Contributed to SIPfoundry under a Contributor Agreement
 *
 * This software is free software; you can redistribute it and/or modify it under
 * the terms of the Affero General Public License (AGPL) as published by the
 * Free Software Foundation; either version 3 of the License, or (at your option)
 * any later version.
 *
 * This software is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more
 * details.
 */
package org.sipfoundry.sipxconfig.systemaudit;

import static org.easymock.EasyMock.createMock;
import static org.easymock.EasyMock.expectLastCall;
import static org.easymock.EasyMock.replay;
import static org.easymock.EasyMock.verify;

import org.sipfoundry.commons.userdb.profile.UserProfile;
import org.sipfoundry.sipxconfig.common.CoreContext;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.test.IntegrationTestCase;
import org.sipfoundry.sipxconfig.test.TestHelper;

public class SystemAuditManagerTest extends IntegrationTestCase {

    private CoreContext m_coreContext;

    @Override
    protected void onSetUpBeforeTransaction() throws Exception {
        super.onSetUpBeforeTransaction();
        TestHelper.cleanInsertFlat("systemaudit/config_change.db.xml");
    }

    @Override
    protected void onTearDownAfterTransaction() throws Exception {
        super.onSetUpBeforeTransaction();
        clear();
    }

    public void testAuditableEntity() throws Exception {
        User user = m_coreContext.newUser();
        user.setUserName("201");
        UserProfile disabledProfile = new UserProfile();
        disabledProfile.setEnabled(false);
        user.setUserProfile(disabledProfile);

        SystemAuditManager systemAuditManager = createMock(SystemAuditManager.class);
        systemAuditManager.onConfigChangeAction((Object) user,
                ConfigChangeAction.ADDED, null, null, null);
        expectLastCall().once();
        replay(systemAuditManager);

        getEntityInterceptor().setSystemAuditManager(systemAuditManager);
        m_coreContext.saveUser(user);
        verify(systemAuditManager);
    }

    public void setCoreContext(CoreContext coreContext) {
        m_coreContext = coreContext;
    }

}
