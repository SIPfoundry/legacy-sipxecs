/*
 *
 *
 * Copyright (C) 2009 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 */
package org.sipfoundry.sipxconfig.userportal.widget;

import com.smartgwt.client.data.DataSource;
import com.smartgwt.client.data.DataSourceField;
import com.smartgwt.client.data.fields.DataSourceTextField;

public class PhonebookDataSource extends DataSource {
    public PhonebookDataSource(String id) {
        super();
        setID(id);

        DataSourceTextField firstName = new DataSourceTextField("first-name");
        DataSourceTextField lastName = new DataSourceTextField("last-name");
        DataSourceTextField number = new DataSourceTextField("number");

        DataSourceField emailAddress = new DataSourceTextField("emailAddress");
        emailAddress.setValueXPath("contact-information/emailAddress");
        DataSourceField jobTitle = new DataSourceTextField("jobTitle");
        jobTitle.setValueXPath("contact-information/jobTitle");
        DataSourceField jobDept = new DataSourceTextField("jobDept");
        jobDept.setValueXPath("contact-information/jobDept");
        DataSourceField companyName = new DataSourceTextField("companyName");
        companyName.setValueXPath("contact-information/companyName");
        DataSourceField assistantName = new DataSourceTextField("assistantName");
        assistantName.setValueXPath("contact-information/assistantName");
        DataSourceField location = new DataSourceTextField("location");
        location.setValueXPath("contact-information/location");
        DataSourceField cellPhoneNumber = new DataSourceTextField("cellPhoneNumber");
        cellPhoneNumber.setValueXPath("contact-information/cellPhoneNumber");
        DataSourceField homePhoneNumber = new DataSourceTextField("homePhoneNumber");
        homePhoneNumber.setValueXPath("contact-information/homePhoneNumber");
        DataSourceField assistantPhoneNumber = new DataSourceTextField("assistantPhoneNumber");
        assistantPhoneNumber.setValueXPath("contact-information/assistantPhoneNumber");
        DataSourceField faxNumber = new DataSourceTextField("faxNumber");
        faxNumber.setValueXPath("contact-information/faxNumber");
        DataSourceField imId = new DataSourceTextField("imId");
        imId.setValueXPath("contact-information/imId");
        DataSourceField alternateImId = new DataSourceTextField("alternateImId");
        alternateImId.setValueXPath("contact-information/alternateImId");
        DataSourceField alternateEmailAddress = new DataSourceTextField("alternateEmailAddress");
        alternateEmailAddress.setValueXPath("contact-information/alternateEmailAddress");

        DataSourceField homeStreet = new DataSourceTextField("homeStreet");
        homeStreet.setValueXPath("contact-information/homeAddress/street");
        DataSourceField homeCity = new DataSourceTextField("homeCity");
        homeCity.setValueXPath("contact-information/homeAddress/city");
        DataSourceField homeCountry = new DataSourceTextField("homeCountry");
        homeCountry.setValueXPath("contact-information/homeAddress/country");
        DataSourceField homeState = new DataSourceTextField("homeState");
        homeState.setValueXPath("contact-information/homeAddress/state");
        DataSourceField homeZip = new DataSourceTextField("homeZip");
        homeZip.setValueXPath("contact-information/homeAddress/zip");

        DataSourceField officeStreet = new DataSourceTextField("officeStreet");
        officeStreet.setValueXPath("contact-information/officeAddress/street");
        DataSourceField officeCity = new DataSourceTextField("officeCity");
        officeCity.setValueXPath("contact-information/officeAddress/city");
        DataSourceField officeCountry = new DataSourceTextField("officeCountry");
        officeCountry.setValueXPath("contact-information/officeAddress/country");
        DataSourceField officeState = new DataSourceTextField("officeState");
        officeState.setValueXPath("contact-information/officeAddress/state");
        DataSourceField officeZip = new DataSourceTextField("officeZip");
        officeZip.setValueXPath("contact-information/officeAddress/zip");
        DataSourceField officeDesignation = new DataSourceTextField("officeDesignation");
        officeDesignation.setValueXPath("contact-information/officeAddress/officeDesignation");

        setFields(firstName, lastName, number, emailAddress, jobTitle, jobDept, companyName, assistantName, location,
                cellPhoneNumber, homePhoneNumber, assistantPhoneNumber, faxNumber, imId, alternateImId,
                alternateEmailAddress, homeStreet, homeCity, homeCountry, homeState, homeZip, officeStreet, officeCity,
                officeCountry, officeState, officeZip, officeDesignation);
        setRecordXPath("/phonebook/entry");
        setDataURL("/sipxconfig/rest/my/phonebook");
        setClientOnly(true);
    }
}
