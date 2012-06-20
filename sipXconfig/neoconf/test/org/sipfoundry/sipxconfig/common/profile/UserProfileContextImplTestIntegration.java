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
package org.sipfoundry.sipxconfig.common.profile;

import org.sipfoundry.commons.userdb.profile.UserProfile;
import org.sipfoundry.commons.userdb.profile.UserProfileService;
import org.sipfoundry.sipxconfig.test.IntegrationTestCase;

public class UserProfileContextImplTestIntegration extends IntegrationTestCase {
    private UserProfileContext m_profileContext;
    private UserProfileService m_profileService;

    @Override
    protected void onSetUpBeforeTransaction() throws Exception {
        super.onSetUpBeforeTransaction();
        clear();
    }
    
    @Override
    protected void onSetUpInTransaction() throws Exception {
        super.onSetUpInTransaction();
        db().execute("INSERT INTO address_book_entry (address_book_entry_id, cell_phone_number, home_phone_number, assistant_name, " +
        		"assistant_phone_number, fax_number, location, company_name, job_title, job_dept, im_id, alternate_im_id, im_display_name, " +
        		"use_branch_address, email_address, alternate_email_address, home_address_id, office_address_id, branch_address_id, " +
        		"did_number) VALUES (1, '12345', '6789', 'Duffy', '8888', '9999', 'Mars', 'Company', 'Title', 'Dept', 'test', 'alt_test', " +
        		"'im_display', false, 'duffy@duck.com', 'bugs@bunny.com', NULL, NULL, NULL, NULL);");
        db().execute("INSERT INTO users (user_id, first_name, pintoken, sip_password, last_name, user_name, value_storage_id, address_book_entry_id, " +
        		"user_type) VALUES (10, 'FirstName', '1234', 'crPBRpX1', 'LastName', '201', NULL, 1, 'C');");
    }

    public void testMigrateProfiles() throws Exception {
        m_profileContext.migrateProfiles();
        UserProfile profile = m_profileService.getUserProfile("10");
        assertEquals("FirstName", profile.getFirstName());
        assertEquals("LastName", profile.getLastName());
        assertEquals("201", profile.getUserName());
        assertEquals("12345", profile.getCellPhoneNumber());
        assertEquals("6789", profile.getHomePhoneNumber());
        assertEquals("Duffy", profile.getAssistantName());
        assertEquals("8888", profile.getAssistantPhoneNumber());
        assertEquals("9999", profile.getFaxNumber());
        assertEquals("Mars", profile.getLocation());
        assertEquals("Company", profile.getCompanyName());
        assertEquals("Title", profile.getJobTitle());
        assertEquals("Dept", profile.getJobDept());
        assertEquals("test", profile.getImId());
        assertEquals("alt_test", profile.getAlternateImId());
        assertEquals("im_display", profile.getImDisplayName());
        assertEquals("duffy@duck.com", profile.getEmailAddress());
        assertEquals("bugs@bunny.com", profile.getAlternateEmailAddress());
    }

    public void setUserProfileContext(UserProfileContext profileContext) {
        m_profileContext = profileContext;
    }

    public void setUserProfileService(UserProfileService profileService) {
        m_profileService = profileService;
    }

}
