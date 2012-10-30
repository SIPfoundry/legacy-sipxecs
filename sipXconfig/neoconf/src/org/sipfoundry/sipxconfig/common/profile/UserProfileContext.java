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

import java.sql.ResultSet;
import java.sql.SQLException;

import static org.apache.commons.lang.StringUtils.EMPTY;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import org.sipfoundry.commons.userdb.profile.Address;
import org.sipfoundry.commons.userdb.profile.UserProfile;
import org.sipfoundry.commons.userdb.profile.UserProfileService;
import org.sipfoundry.sipxconfig.branch.Branch;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.common.UserException;
import org.sipfoundry.sipxconfig.common.event.DaoEventListener;
import org.sipfoundry.sipxconfig.commserver.LocationsManager;
import org.sipfoundry.sipxconfig.setup.SetupListener;
import org.sipfoundry.sipxconfig.setup.SetupManager;
import org.springframework.beans.factory.annotation.Required;
import org.springframework.core.io.ClassPathResource;
import org.springframework.core.io.Resource;
import org.springframework.jdbc.core.JdbcTemplate;
import org.springframework.jdbc.core.RowCallbackHandler;

public class UserProfileContext implements DaoEventListener, SetupListener {
    private static final Log LOG = LogFactory.getLog(UserProfileContext.class);
    private static final String AVATAR_FORMAT = "https://%s/sipxconfig/rest/avatar/%s";
    private static final String PROFILE_SETUP = "profile";
    private static final String PROFILE_MIGRATION = "migrate_profiles";
    private UserProfileService m_userProfileService;
    private LocationsManager m_locationManager;
    private JdbcTemplate m_jdbc;

    @Override
    public boolean setup(SetupManager manager) {
        try {
            if (manager.isFalse(PROFILE_SETUP)) {
                Resource defaultAvatar = new ClassPathResource(
                        "org/sipfoundry/sipxconfig/common/profile/default_avatar.jpg");
                m_userProfileService.saveAvatar("default", defaultAvatar.getInputStream(), false);
                manager.setTrue(PROFILE_SETUP);
            }

            if (manager.isTrue(PROFILE_MIGRATION)) {
                migrateProfiles();
                manager.setFalse(PROFILE_MIGRATION);
            }
        } catch (Exception ex) {
            LOG.error("failed to upload default avatar / migrate profiles" + ex.getMessage());
        }
        return true;
    }

    @Override
    public void onDelete(Object entity) {
        if (entity instanceof User) {
            try {
                User user = (User) entity;
                m_userProfileService.deleteUserProfile(user.getUserProfile());
            } catch (Exception ex) {
                LOG.error("failed to delete profile from mongo " + ex.getMessage());
                throw new UserException("&err.msg.deleteProfile", ex.getMessage());
            }
        }
    }

    @Override
    public void onSave(Object entity) {
        if (entity instanceof User) {
            try {
                User user = (User) entity;
                UserProfile profile = user.getUserProfile();
                profile.setAvatar(String.format(AVATAR_FORMAT,
                        m_locationManager.getPrimaryLocation().getFqdn(), user.getUserName()));
                profile.setBranchName(user.getSite() != null ? user.getSite().getName() : EMPTY);
                profile.setBranchAddress(user.getSite() != null ? createBranchAddress(user.getSite()) : new Address());
                m_userProfileService.saveUserProfile(profile);
            } catch (Exception ex) {
                LOG.error("failed to save profile in mongo" + ex.getMessage());
                throw new UserException("&err.msg.saveProfile", ex.getMessage());
            }
        } else if (entity instanceof Branch) {
            Branch branch = (Branch) entity;
            Address branchAddress = createBranchAddress(branch);
            m_userProfileService.updateBranchAddress(branch.getName(), branchAddress);
        }
    }

    private Address createBranchAddress(Branch branch) {
        Address branchAddress = new Address();
        branchAddress.setCity(branch.getAddress().getCity());
        branchAddress.setCountry(branch.getAddress().getCountry());
        branchAddress.setOfficeDesignation(branch.getAddress().getOfficeDesignation());
        branchAddress.setState(branch.getAddress().getState());
        branchAddress.setStreet(branch.getAddress().getStreet());
        branchAddress.setZip(branch.getAddress().getZip());
        return branchAddress;
    }

    protected void migrateProfiles() {
        RowCallbackHandler handler = new RowCallbackHandler() {

            @Override
            public void processRow(ResultSet rs) throws SQLException {
                UserProfile profile = new UserProfile();
                profile.setUserId(rs.getString("user_id"));
                profile.setUserName(rs.getString("user_name"));
                profile.setFirstName(rs.getString("first_name"));
                profile.setLastName(rs.getString("last_name"));
                profile.setJobTitle(rs.getString("job_title"));
                profile.setJobDept(rs.getString("job_dept"));
                profile.setCompanyName(rs.getString("company_name"));
                profile.setAssistantName(rs.getString("assistant_name"));
                profile.setAssistantPhoneNumber(rs.getString("assistant_phone_number"));
                profile.setHomePhoneNumber(rs.getString("home_phone_number"));
                profile.setCellPhoneNumber(rs.getString("cell_phone_number"));
                profile.setFaxNumber(rs.getString("fax_number"));
                profile.setImId(rs.getString("im_id"));
                profile.setImDisplayName(rs.getString("im_display_name"));
                profile.setAlternateImId(rs.getString("alternate_im_id"));
                profile.setLocation(rs.getString("location"));
                profile.setUseBranchAddress(Boolean.valueOf(rs.getString("use_branch_address")));
                profile.setEmailAddress(rs.getString("email_address"));
                profile.setAlternateEmailAddress(rs.getString("alternate_email_address"));
                profile.setDidNumber(rs.getString("did_number"));

                profile.getHomeAddress().setStreet(rs.getString("home_street"));
                profile.getHomeAddress().setZip(rs.getString("home_zip"));
                profile.getHomeAddress().setCountry(rs.getString("home_country"));
                profile.getHomeAddress().setState(rs.getString("home_state"));
                profile.getHomeAddress().setCity(rs.getString("home_city"));
                profile.getHomeAddress().setOfficeDesignation(rs.getString("home_post"));

                profile.getOfficeAddress().setStreet(rs.getString("office_street"));
                profile.getOfficeAddress().setZip(rs.getString("office_zip"));
                profile.getOfficeAddress().setCountry(rs.getString("office_country"));
                profile.getOfficeAddress().setState(rs.getString("office_state"));
                profile.getOfficeAddress().setCity(rs.getString("office_city"));
                profile.getOfficeAddress().setOfficeDesignation(rs.getString("office_post"));

                profile.getBranchAddress().setStreet(rs.getString("branch_street"));
                profile.getBranchAddress().setZip(rs.getString("branch_zip"));
                profile.getBranchAddress().setCountry(rs.getString("branch_country"));
                profile.getBranchAddress().setState(rs.getString("branch_state"));
                profile.getBranchAddress().setCity(rs.getString("branch_city"));
                profile.getBranchAddress().setOfficeDesignation(rs.getString("branch_post"));

                profile.setAvatar(String.format(AVATAR_FORMAT, m_locationManager.
                        getPrimaryLocation().getFqdn(), profile.getUserName()));

                m_userProfileService.saveUserProfile(profile);
            }
        };
        m_jdbc.query(
                "SELECT u.user_id, u.user_name, u.first_name, u.last_name,"
                        + " abe.job_title, abe.job_dept, abe.company_name, abe.assistant_name,"
                        + " abe.assistant_phone_number, abe.home_phone_number, abe.cell_phone_number, abe.fax_number,"
                        + " abe.im_id, abe.im_display_name, abe.alternate_im_id, abe.location,"
                        + " abe.use_branch_address, abe.email_address, abe.alternate_email_address, abe.did_number,"
                        + " h.street as home_street, h.zip as home_zip, h.country as home_country,"
                        + " h.state as home_state, h.city as home_city, h.office_designation as home_post,"
                        + " o.street as office_street, o.zip as office_zip, o.country as office_country,"
                        + " o.state as office_state, o.city as office_city, o.office_designation as office_post,"
                        + " b.street as branch_street, b.zip as branch_zip, b.country as branch_country,"
                        + " b.state as branch_state, b.city as branch_city, b.office_designation as branch_post"
                        + " from users u inner join address_book_entry abe"
                        + " on u.address_book_entry_id = abe.address_book_entry_id"
                        + " left join address h on abe.office_address_id = h.address_id"
                        + " left join address o on abe.office_address_id = o.address_id"
                        + " left join address b on abe.office_address_id = b.address_id", handler);
    }

    public void setUserProfileService(UserProfileService profileService) {
        m_userProfileService = profileService;
    }

    @Required
    public void setLocationManager(LocationsManager locationManager) {
        m_locationManager = locationManager;
    }

    public void setJdbc(JdbcTemplate jdbc) {
        m_jdbc = jdbc;
    }
}
