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

import com.mongodb.DBObject;

import org.sipfoundry.sipxconfig.branch.Branch;
import org.sipfoundry.sipxconfig.common.Replicable;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.phonebook.Address;
import org.sipfoundry.sipxconfig.phonebook.AddressBookEntry;

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
    public boolean generate(Replicable entity, DBObject top) {
        if (entity instanceof User) {
            User user = (User) entity;
            Branch site = user.getSite();
            if (site != null) {
                top.put(USER_LOCATION, site.getName());
            } else {
                top.put(USER_LOCATION, null);
            }
            AddressBookEntry abe = user.getAddressBookEntry();
            if (abe != null) {
                top.put(ALT_IM_ID, abe.getAlternateImId());
                top.put(JOB_TITLE, abe.getJobTitle());
                top.put(JOB_DEPT, abe.getJobDept());
                top.put(COMPANY_NAME, abe.getCompanyName());
                top.put(ASSISTANT_NAME, abe.getAssistantName());
                top.put(ASSISTANT_PHONE, abe.getAssistantPhoneNumber());
                top.put(FAX_NUMBER, abe.getFaxNumber());
                top.put(HOME_PHONE_NUMBER, abe.getHomePhoneNumber());
                top.put(CELL_PHONE_NUMBER, abe.getCellPhoneNumber());
                top.put(AVATAR, abe.getAvatar());
                top.put(LOCATION, abe.getLocation());
                // FIXME abe.getOfficeAddress should be accurate enough to get real office address
                // complete fix should be when XX-8002 gets solved
                Address officeAddress = null;
                if (abe.getUseBranchAddress() && site != null) {
                    officeAddress = site.getAddress();
                } else {
                    officeAddress = abe.getOfficeAddress();
                }
                Address home = abe.getHomeAddress();
                if (home != null) {
                    top.put(HOME_STREET, home.getStreet());
                    top.put(HOME_CITY, home.getCity());
                    top.put(HOME_COUNTRY, home.getCountry());
                    top.put(HOME_STATE, home.getState());
                    top.put(HOME_ZIP, home.getZip());
                }
                if (officeAddress != null) {
                    top.put(OFFICE_STREET, officeAddress.getStreet());
                    top.put(OFFICE_CITY, officeAddress.getCity());
                    top.put(OFFICE_COUNTRY, officeAddress.getCountry());
                    top.put(OFFICE_STATE, officeAddress.getState());
                    top.put(OFFICE_ZIP, officeAddress.getZip());
                    top.put(OFFICE_DESIGNATION, officeAddress.getOfficeDesignation());
                }
            }
            return true;
        }
        return false;
    }

}
