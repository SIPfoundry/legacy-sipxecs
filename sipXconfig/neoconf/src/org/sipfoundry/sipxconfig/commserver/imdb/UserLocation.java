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
package org.sipfoundry.sipxconfig.commserver.imdb;

import com.mongodb.DBObject;

import org.sipfoundry.commons.userdb.profile.Address;
import org.sipfoundry.commons.userdb.profile.UserProfile;
import org.sipfoundry.sipxconfig.branch.Branch;
import org.sipfoundry.sipxconfig.common.Replicable;
import org.sipfoundry.sipxconfig.common.User;

import static org.sipfoundry.commons.mongo.MongoConstants.ALT_IM_ID;
import static org.sipfoundry.commons.mongo.MongoConstants.ASSISTANT_NAME;
import static org.sipfoundry.commons.mongo.MongoConstants.ASSISTANT_PHONE;
import static org.sipfoundry.commons.mongo.MongoConstants.AVATAR;
import static org.sipfoundry.commons.mongo.MongoConstants.CELL_PHONE_NUMBER;
import static org.sipfoundry.commons.mongo.MongoConstants.COMPANY_NAME;
import static org.sipfoundry.commons.mongo.MongoConstants.FAX_NUMBER;
import static org.sipfoundry.commons.mongo.MongoConstants.HOME_CITY;
import static org.sipfoundry.commons.mongo.MongoConstants.HOME_COUNTRY;
import static org.sipfoundry.commons.mongo.MongoConstants.HOME_PHONE_NUMBER;
import static org.sipfoundry.commons.mongo.MongoConstants.HOME_STATE;
import static org.sipfoundry.commons.mongo.MongoConstants.HOME_STREET;
import static org.sipfoundry.commons.mongo.MongoConstants.HOME_ZIP;
import static org.sipfoundry.commons.mongo.MongoConstants.JOB_DEPT;
import static org.sipfoundry.commons.mongo.MongoConstants.JOB_TITLE;
import static org.sipfoundry.commons.mongo.MongoConstants.LOCATION;
import static org.sipfoundry.commons.mongo.MongoConstants.OFFICE_CITY;
import static org.sipfoundry.commons.mongo.MongoConstants.OFFICE_COUNTRY;
import static org.sipfoundry.commons.mongo.MongoConstants.OFFICE_DESIGNATION;
import static org.sipfoundry.commons.mongo.MongoConstants.OFFICE_STATE;
import static org.sipfoundry.commons.mongo.MongoConstants.OFFICE_STREET;
import static org.sipfoundry.commons.mongo.MongoConstants.OFFICE_ZIP;
import static org.sipfoundry.commons.mongo.MongoConstants.USER_LOCATION;

public class UserLocation extends AbstractDataSetGenerator {

    @Override
    protected DataSet getType() {
        return DataSet.USER_LOCATION;
    }

    @Override
    public void generate(Replicable entity, DBObject top) {
        if (entity instanceof User) {
            User user = (User) entity;
            Branch site = user.getSite();
            if (site != null) {
                top.put(USER_LOCATION, site.getName());
            } else {
                top.removeField(USER_LOCATION);
            }

            UserProfile profile = user.getUserProfile();
            if (profile != null) {
                putOnlyIfNotNull(top, ALT_IM_ID, (profile.getAlternateImId() != null)
                        ? (profile.getAlternateImId().toLowerCase()) : (null));
                putOnlyIfNotNull(top, JOB_TITLE, profile.getJobTitle());
                putOnlyIfNotNull(top, JOB_DEPT, profile.getJobDept());
                putOnlyIfNotNull(top, COMPANY_NAME, profile.getCompanyName());
                putOnlyIfNotNull(top, ASSISTANT_NAME, profile.getAssistantName());
                putOnlyIfNotNull(top, ASSISTANT_PHONE, profile.getAssistantPhoneNumber());
                putOnlyIfNotNull(top, FAX_NUMBER, profile.getFaxNumber());
                putOnlyIfNotNull(top, HOME_PHONE_NUMBER, profile.getHomePhoneNumber());
                putOnlyIfNotNull(top, CELL_PHONE_NUMBER, profile.getCellPhoneNumber());
                putOnlyIfNotNull(top, AVATAR, profile.getAvatar());
                putOnlyIfNotNull(top, LOCATION, profile.getLocation());
                // FIXME abe.getOfficeAddress should be accurate enough to get real office address
                // complete fix should be when XX-8002 gets solved
                Address officeAddress = null;
                if (profile.getUseBranchAddress() && site != null) {
                    officeAddress = new Address();
                    officeAddress.setCity(site.getAddress().getCity());
                    officeAddress.setCountry(site.getAddress().getCountry());
                    officeAddress.setOfficeDesignation(site.getAddress().getOfficeDesignation());
                    officeAddress.setState(site.getAddress().getState());
                    officeAddress.setStreet(site.getAddress().getStreet());
                    officeAddress.setZip(site.getAddress().getZip());
                } else {
                    officeAddress = profile.getOfficeAddress();
                }
                Address home = profile.getHomeAddress();
                if (home != null) {
                    putOnlyIfNotNull(top, HOME_STREET, home.getStreet());
                    putOnlyIfNotNull(top, HOME_CITY, home.getCity());
                    putOnlyIfNotNull(top, HOME_COUNTRY, home.getCountry());
                    putOnlyIfNotNull(top, HOME_STATE, home.getState());
                    putOnlyIfNotNull(top, HOME_ZIP, home.getZip());
                }
                if (officeAddress != null) {
                    putOnlyIfNotNull(top, OFFICE_STREET, officeAddress.getStreet());
                    putOnlyIfNotNull(top, OFFICE_CITY, officeAddress.getCity());
                    putOnlyIfNotNull(top, OFFICE_COUNTRY, officeAddress.getCountry());
                    putOnlyIfNotNull(top, OFFICE_STATE, officeAddress.getState());
                    putOnlyIfNotNull(top, OFFICE_ZIP, officeAddress.getZip());
                    putOnlyIfNotNull(top, OFFICE_DESIGNATION, officeAddress.getOfficeDesignation());
                }
            }
        }
    }

}
