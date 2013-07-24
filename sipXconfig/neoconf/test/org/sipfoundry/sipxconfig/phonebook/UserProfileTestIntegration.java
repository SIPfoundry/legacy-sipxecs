/**
 *
 *
 * Copyright (c) 2012 eZuce, Inc. All rights reserved.
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
package org.sipfoundry.sipxconfig.phonebook;

import java.util.Collections;

import org.sipfoundry.commons.userdb.profile.UserProfile;
import org.sipfoundry.sipxconfig.common.CoreContext;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.test.IntegrationTestCase;

public class UserProfileTestIntegration extends IntegrationTestCase {
    private CoreContext m_coreContext;

    @Override
    protected void onSetUpBeforeTransaction() throws Exception {
        super.onSetUpBeforeTransaction();
        clear();
        sql("commserver/SeedLocations.sql");
        sql("domain/DomainSeed.sql");
        User user = m_coreContext.newUser();
        user.setFirstName("First");
        user.setLastName("Last");
        user.setUserName("2001");
        user.setImId("imid");
        user.setImDisplayName("First ImId");
        user.setEmailAddress("test@test.org");
        user.setAlternateEmailAddress("alttest@test.org");
        m_coreContext.saveUser(user);
    }

    public void testSaveUser() throws Exception {
        assertEquals("2001", getUserProfileService().getUsernameByImId("imid"));
        int userId = getUserProfileService().getUserIdByImId("imid");
        UserProfile profile = getUserProfileService().getUserProfile(String.valueOf(userId));
        assertNotNull(profile);
        assertEquals("First", profile.getFirstName());
        assertEquals("Last", profile.getLastName());
        assertEquals("test@test.org", profile.getEmailAddress());
        assertEquals("alttest@test.org", profile.getAlternateEmailAddress());
        assertEquals("First Last", profile.getImDisplayName());
        assertTrue(getUserProfileService().isImIdInUse("imid"));
        assertFalse(getUserProfileService().isImIdInUse("imid", userId));
    }

    public void testDeleteUser() throws Exception {
        getDaoEventPublisher().resetListeners();
        m_coreContext.deleteUsersByUserName(Collections.singleton("2001"));
        assertNull(getUserProfileService().getUsernameByImId("imid"));
        assertNull(getUserProfileService().getUserProfileByImId("imid"));
    }

    public void testLoadUser() throws Exception {
        User user = m_coreContext.loadUserByUserName("2001");
        UserProfile profile = user.getUserProfile();
        assertEquals("First", profile.getFirstName());
        assertEquals("Last", profile.getLastName());
        assertEquals("test@test.org", profile.getEmailAddress());
        assertEquals("alttest@test.org", profile.getAlternateEmailAddress());
        assertEquals("First Last", profile.getImDisplayName());
    }

    public void setCoreContext(CoreContext coreContext) {
        m_coreContext = coreContext;
    }
}
